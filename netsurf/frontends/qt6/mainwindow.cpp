#include "mainwindow.h"
#include "browsertab.h"
#include "dialogs.h"

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QLineEdit>
#include <QLabel>
#include <QTabBar>
#include <QStackedWidget>
#include <QStatusBar>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWindow>
#include <QIcon>
#include <QSizePolicy>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeySequence>
#include <QUrl>

extern "C" {
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/mouse.h"
#include "netsurf/window.h"
#include "desktop/browser_history.h"
}

namespace {

constexpr int kResizeMargin = 6;

/** The tab-strip toolbar doubles as the window's title bar (frameless
 * window -- see BesraWindow's constructor): clicking its own background
 * (not any child widget like a tab or button) drags the window via the
 * window manager's native move protocol, and double-clicking toggles
 * maximize -- both standard title-bar behaviours otherwise lost once
 * the native decoration is gone. */
class TitleBarToolBar : public QToolBar {
public:
    TitleBarToolBar(const QString &title, QWidget *window)
        : QToolBar(title, window), window_(window) {}

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            if (QWindow *wh = window_->windowHandle()) {
                wh->startSystemMove();
                return;
            }
        }
        QToolBar::mousePressEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            if (window_->isMaximized()) {
                window_->showNormal();
            } else {
                window_->showMaximized();
            }
            return;
        }
        QToolBar::mouseDoubleClickEvent(event);
    }

private:
    QWidget *window_;
};

} // namespace

QList<BesraWindow *> &BesraWindow::registry()
{
    static QList<BesraWindow *> windows;
    return windows;
}

BesraWindow *BesraWindow::activeWindow()
{
    auto &windows = registry();
    return windows.isEmpty() ? nullptr : windows.first();
}

BesraWindow::BesraWindow(QWidget *parent) : QMainWindow(parent)
{
    registry().prepend(this);

    /* Frameless: we draw our own title bar (buildTitleBar()) with tabs
     * sharing the row with minimize/maximize/close, like Chrome/Edge.
     * The OS gives us none of the usual title-bar behaviour for free
     * once this is set -- drag-to-move, double-click-to-maximize, and
     * edge/corner resize are all hand-implemented below via
     * QWindow::startSystemMove()/startSystemResize(), which delegate
     * the actual operation to the window manager rather than computing
     * geometry by hand. */
    setWindowFlag(Qt::FramelessWindowHint);

    tab_stack_ = new QStackedWidget(this);
    setCentralWidget(tab_stack_);

    buildTitleBar();
    /* Without this, QMainWindow just flows same-area toolbars
     * left-to-right on one row until they run out of width -- it does
     * NOT reliably wrap the nav toolbar onto its own row below the
     * title bar just because there are two of them. A narrower window
     * showed exactly that: tabs, window buttons, back/forward/reload,
     * and the address bar all crammed onto a single line. */
    addToolBarBreak(Qt::TopToolBarArea);

    status_label_ = new QLabel(this);
    statusBar()->addWidget(status_label_, 1);

    QList<QMenu *> menus = buildMenus();
    /* Traditional menu bar stays hidden until a bare Alt press reveals
     * it (see keyReleaseEvent) -- its actions are always reachable via
     * the hamburger menu on the address bar regardless. */
    menuBar()->setVisible(false);

    buildToolbar(menus);

    connect(tab_bar_, &QTabBar::currentChanged, this, &BesraWindow::onCurrentTabChanged);
    connect(tab_bar_, &QTabBar::tabCloseRequested, this, &BesraWindow::onTabCloseRequested);
    connect(tab_bar_, &QTabBar::tabMoved, this, &BesraWindow::onTabMoved);

    qApp->installEventFilter(this);

    resize(1200, 800);
    setWindowTitle(QStringLiteral("Besra"));
}

BesraWindow::~BesraWindow()
{
    qApp->removeEventFilter(this);
    registry().removeAll(this);
}

void BesraWindow::buildTitleBar()
{
    auto *bar = new TitleBarToolBar(tr("Tabs"), this);
    bar->setMovable(false);
    addToolBar(bar);

    tab_bar_ = new QTabBar(bar);
    tab_bar_->setTabsClosable(true);
    tab_bar_->setMovable(true);
    tab_bar_->setExpanding(false);
    bar->addWidget(tab_bar_);

    QToolButton *new_tab_button = new QToolButton(bar);
    new_tab_button->setText(QStringLiteral("+"));
    new_tab_button->setToolTip(tr("New Tab"));
    connect(new_tab_button, &QToolButton::clicked, this, &BesraWindow::onNewTab);
    bar->addWidget(new_tab_button);

    /* Pushes the window-control buttons to the far right of the row,
     * same as the tab strip's own empty space -- clicks here also fall
     * through to TitleBarToolBar's drag/maximize handling since a plain
     * QWidget doesn't consume mouse events on its own. */
    auto *spacer = new QWidget(bar);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    bar->addWidget(spacer);

    minimize_button_ = new QToolButton(bar);
    minimize_button_->setText(QStringLiteral("─"));
    minimize_button_->setToolTip(tr("Minimize"));
    connect(minimize_button_, &QToolButton::clicked, this, &BesraWindow::onMinimize);
    bar->addWidget(minimize_button_);

    maximize_button_ = new QToolButton(bar);
    connect(maximize_button_, &QToolButton::clicked, this, &BesraWindow::onMaximizeRestore);
    bar->addWidget(maximize_button_);
    updateMaximizeIcon();

    close_button_ = new QToolButton(bar);
    close_button_->setText(QStringLiteral("✕"));
    close_button_->setToolTip(tr("Close"));
    connect(close_button_, &QToolButton::clicked, this, &QWidget::close);
    bar->addWidget(close_button_);
}

void BesraWindow::buildToolbar(const QList<QMenu *> &menus)
{
    QToolBar *bar = addToolBar(tr("Navigation"));
    bar->setMovable(false);

    back_button_ = new QToolButton(bar);
    back_button_->setText(QStringLiteral("←"));
    back_button_->setToolTip(tr("Back"));
    connect(back_button_, &QToolButton::clicked, this, &BesraWindow::onBack);
    bar->addWidget(back_button_);

    forward_button_ = new QToolButton(bar);
    forward_button_->setText(QStringLiteral("→"));
    forward_button_->setToolTip(tr("Forward"));
    connect(forward_button_, &QToolButton::clicked, this, &BesraWindow::onForward);
    bar->addWidget(forward_button_);

    reload_stop_button_ = new QToolButton(bar);
    reload_stop_button_->setText(QStringLiteral("↻"));
    reload_stop_button_->setToolTip(tr("Reload"));
    connect(reload_stop_button_, &QToolButton::clicked, this, &BesraWindow::onReloadOrStop);
    bar->addWidget(reload_stop_button_);

    url_bar_ = new QLineEdit(bar);
    url_bar_->setPlaceholderText(tr("Enter address"));
    connect(url_bar_, &QLineEdit::returnPressed, this, &BesraWindow::onNavigate);
    bar->addWidget(url_bar_);

    /* Hamburger menu: folds every File/Edit/View/History/Bookmarks/
     * Tools/Help action into one button, since the menu bar itself is
     * hidden by default. Reuses the same QMenu objects buildMenus()
     * already attached to the (hidden) menu bar as submenus here --
     * Qt allows one QMenu to be the target of actions in more than one
     * parent menu, so nothing is built twice. */
    QToolButton *hamburger = new QToolButton(bar);
    hamburger->setText(QStringLiteral("☰"));
    hamburger->setToolTip(tr("Menu"));
    hamburger->setPopupMode(QToolButton::InstantPopup);
    /* The hamburger glyph already signals "opens a menu" -- the style's
     * default extra dropdown-arrow indicator next to it is redundant. */
    hamburger->setStyleSheet(QStringLiteral(
        "QToolButton::menu-indicator { image: none; width: 0; }"));
    QMenu *hamburger_menu = new QMenu(hamburger);
    for (QMenu *menu : menus) {
        hamburger_menu->addMenu(menu);
    }
    hamburger->setMenu(hamburger_menu);
    bar->addWidget(hamburger);
}

QList<QMenu *> BesraWindow::buildMenus()
{
    QMenu *file_menu = menuBar()->addMenu(tr("&File"));
    file_menu->addAction(tr("New &Tab"), this, &BesraWindow::onNewTab, QKeySequence::AddTab);
    file_menu->addAction(tr("New &Window"), this, &BesraWindow::onNewWindow, QKeySequence::New);
    file_menu->addAction(tr("&Open File..."), this, &BesraWindow::onOpenFile, QKeySequence::Open);
    file_menu->addSeparator();
    file_menu->addAction(tr("&Print..."), this, &BesraWindow::onPrint, QKeySequence::Print);
    file_menu->addSeparator();
    file_menu->addAction(tr("&Close Tab"), this, &BesraWindow::onCloseTab, QKeySequence::Close);
    file_menu->addAction(tr("&Quit"), qApp, &QApplication::quit, QKeySequence::Quit);

    QMenu *edit_menu = menuBar()->addMenu(tr("&Edit"));
    edit_menu->addAction(tr("&Find in Page..."), this, &BesraWindow::onFindInPage, QKeySequence::Find);

    QMenu *view_menu = menuBar()->addMenu(tr("&View"));
    view_menu->addAction(tr("Zoom &In"), this, &BesraWindow::onZoomIn, QKeySequence::ZoomIn);
    view_menu->addAction(tr("Zoom &Out"), this, &BesraWindow::onZoomOut, QKeySequence::ZoomOut);
    view_menu->addAction(tr("&Reset Zoom"), this, &BesraWindow::onZoomReset);
    view_menu->addSeparator();
    view_menu->addAction(tr("Page &Source"), this, &BesraWindow::onViewSource);

    QMenu *history_menu = menuBar()->addMenu(tr("&History"));
    history_menu->addAction(tr("&Back"), this, &BesraWindow::onBack, QKeySequence::Back);
    history_menu->addAction(tr("&Forward"), this, &BesraWindow::onForward, QKeySequence::Forward);
    history_menu->addSeparator();
    history_menu->addAction(tr("Show &All History"), this, &BesraWindow::onShowHistory);

    QMenu *bookmarks_menu = menuBar()->addMenu(tr("&Bookmarks"));
    bookmarks_menu->addAction(tr("&Add Bookmark"), this, &BesraWindow::onAddBookmark, QKeySequence::AddTab);
    bookmarks_menu->addAction(tr("Show &Bookmarks"), this, &BesraWindow::onShowBookmarks);

    QMenu *tools_menu = menuBar()->addMenu(tr("&Tools"));
    tools_menu->addAction(tr("&Cookies"), this, &BesraWindow::onShowCookies);
    tools_menu->addAction(tr("&Downloads"), this, &BesraWindow::onShowDownloads);
    tools_menu->addAction(tr("&Preferences"), this, &BesraWindow::onShowPreferences, QKeySequence::Preferences);

    QMenu *help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->addAction(tr("&About Besra"), this, &BesraWindow::onShowAbout);

    return {file_menu, edit_menu, view_menu, history_menu,
            bookmarks_menu, tools_menu, help_menu};
}

BrowserTab *BesraWindow::addTab(struct browser_window *bw, bool foreground)
{
    BrowserTab *tab = new BrowserTab(bw, this, tab_stack_);
    tab_stack_->addWidget(tab);
    int index = tab_bar_->addTab(tr("New Tab"));
    /* Placeholder icon until the real favicon (if any) arrives via
     * refreshChrome(), matching Firefox showing a generic icon rather
     * than a blank tab while a page is loading. */
    tab_bar_->setTabIcon(index, QIcon(QStringLiteral(":/res/besra-logo.png")));
    if (foreground) {
        tab_bar_->setCurrentIndex(index);
        tab_stack_->setCurrentIndex(index);
    }
    return tab;
}

void BesraWindow::removeTab(BrowserTab *tab)
{
    int index = tab_stack_->indexOf(tab);
    if (index >= 0) {
        tab_bar_->removeTab(index);
        tab_stack_->removeWidget(tab);
    }
    tab->deleteLater();

    if (tab_bar_->count() == 0) {
        close();
    }
}

BrowserTab *BesraWindow::currentTab() const
{
    return qobject_cast<BrowserTab *>(tab_stack_->currentWidget());
}

void BesraWindow::refreshChrome(BrowserTab *tab)
{
    int index = tab_stack_->indexOf(tab);
    if (index >= 0) {
        QString label = tab->title().isEmpty() ? tr("New Tab") : tab->title();
        tab_bar_->setTabText(index, label.left(32));
        if (!tab->favicon().isNull()) {
            tab_bar_->setTabIcon(index, tab->favicon());
        }
    }

    if (tab != currentTab()) {
        return;
    }

    setWindowTitle(tab->title().isEmpty()
        ? QStringLiteral("Besra")
        : tab->title() + QStringLiteral(" - Besra"));
    url_bar_->setText(tab->url());

    struct browser_window *bw = tab->browserWindow();
    back_button_->setEnabled(browser_window_history_back_available(bw));
    forward_button_->setEnabled(browser_window_history_forward_available(bw));

    reload_stop_button_->setText(tab->loading() ? QStringLiteral("✕") : QStringLiteral("↻"));
    reload_stop_button_->setToolTip(tab->loading() ? tr("Stop") : tr("Reload"));
}

BrowserTab *BesraWindow::createTabOrWindow(struct browser_window *bw,
        struct gui_window *existing, unsigned flags)
{
    (void)existing;

    BesraWindow *target = nullptr;
    if (flags & GW_CREATE_TAB) {
        target = activeWindow();
    }
    if (target == nullptr) {
        target = new BesraWindow();
        target->show();
    }

    bool foreground = (flags & GW_CREATE_FOREGROUND) || target->tab_bar_->count() == 0;
    BrowserTab *tab = target->addTab(bw, foreground);

    if ((flags & GW_CREATE_FOCUS_LOCATION) && foreground) {
        target->url_bar_->setFocus();
    }

    return tab;
}

void BesraWindow::navigateTo(const QString &text)
{
    BrowserTab *tab = currentTab();
    if (tab == nullptr || text.isEmpty()) {
        return;
    }

    QString target = text;
    QUrl as_url(target, QUrl::TolerantMode);
    if (as_url.scheme().isEmpty()) {
        /* No scheme: treat as a bare host/path, default to https. */
        target = QStringLiteral("https://") + target;
    }

    nsurl *url = nullptr;
    if (nsurl_create(target.toUtf8().constData(), &url) != NSERROR_OK) {
        return;
    }
    browser_window_navigate(tab->browserWindow(), url, nullptr, BW_NAVIGATE_HISTORY,
        nullptr, nullptr, nullptr);
    nsurl_unref(url);
}

void BesraWindow::onNavigate() { navigateTo(url_bar_->text().trimmed()); }

void BesraWindow::onBack()
{
    if (BrowserTab *tab = currentTab()) {
        browser_window_history_back(tab->browserWindow(), false);
        refreshChrome(tab);
    }
}

void BesraWindow::onForward()
{
    if (BrowserTab *tab = currentTab()) {
        browser_window_history_forward(tab->browserWindow(), false);
        refreshChrome(tab);
    }
}

void BesraWindow::onReloadOrStop()
{
    BrowserTab *tab = currentTab();
    if (tab == nullptr) {
        return;
    }
    if (tab->loading()) {
        browser_window_stop(tab->browserWindow());
    } else {
        browser_window_reload(tab->browserWindow(), true);
    }
}

void BesraWindow::onNewTab()
{
    nsurl *url = nullptr;
    if (nsurl_create("about:blank", &url) == NSERROR_OK) {
        /* BW_CREATE_TAB is required here, not just BW_CREATE_HISTORY --
         * without it there's nothing telling gui_window_create() to
         * open in the current window's tab strip, so it falls through
         * to opening a whole new BesraWindow instead. */
        browser_window_create(
            static_cast<browser_window_create_flags>(BW_CREATE_HISTORY | BW_CREATE_TAB),
            url, nullptr, nullptr, nullptr);
        nsurl_unref(url);
    }
    url_bar_->setFocus();
}

void BesraWindow::onNewWindow()
{
    nsurl *url = nullptr;
    if (nsurl_create("about:blank", &url) == NSERROR_OK) {
        struct browser_window *new_bw = nullptr;
        browser_window_create(
            static_cast<browser_window_create_flags>(BW_CREATE_HISTORY),
            url, nullptr, nullptr, &new_bw);
        nsurl_unref(url);
    }
}

void BesraWindow::onCloseTab()
{
    if (BrowserTab *tab = currentTab()) {
        browser_window_destroy(tab->browserWindow());
    }
}

void BesraWindow::onOpenFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"));
    if (!path.isEmpty()) {
        navigateTo(QUrl::fromLocalFile(path).toString());
    }
}

void BesraWindow::onPrint()
{
    if (BrowserTab *tab = currentTab()) {
        besra::showPrint(this, tab->browserWindow());
    }
}

void BesraWindow::onFindInPage()
{
    if (BrowserTab *tab = currentTab()) {
        besra::showFindInPage(this, tab->browserWindow());
    }
}

void BesraWindow::onViewSource()
{
    if (BrowserTab *tab = currentTab()) {
        besra::showViewSource(this, tab->browserWindow());
    }
}

void BesraWindow::onZoomIn()
{
    if (BrowserTab *tab = currentTab()) {
        browser_window_set_scale(tab->browserWindow(), 0.1f, false);
    }
}

void BesraWindow::onZoomOut()
{
    if (BrowserTab *tab = currentTab()) {
        browser_window_set_scale(tab->browserWindow(), -0.1f, false);
    }
}

void BesraWindow::onZoomReset()
{
    if (BrowserTab *tab = currentTab()) {
        browser_window_set_scale(tab->browserWindow(), 1.0f, true);
    }
}

void BesraWindow::onShowHistory() { besra::showHistory(this); }
void BesraWindow::onShowBookmarks() { besra::showBookmarks(this); }

void BesraWindow::onAddBookmark()
{
    if (BrowserTab *tab = currentTab()) {
        besra::addBookmark(tab->browserWindow());
    }
}

void BesraWindow::onShowCookies() { besra::showCookies(this); }
void BesraWindow::onShowDownloads() { besra::showDownloads(this); }
void BesraWindow::onShowPreferences() { besra::showPreferences(this); }
void BesraWindow::onShowAbout() { besra::showAbout(this); }

void BesraWindow::onCurrentTabChanged(int index)
{
    tab_stack_->setCurrentIndex(index);
    if (BrowserTab *tab = currentTab()) {
        refreshChrome(tab);
        tab->renderWidget()->setFocus();
    }
}

void BesraWindow::onTabCloseRequested(int index)
{
    if (auto *tab = qobject_cast<BrowserTab *>(tab_stack_->widget(index))) {
        browser_window_destroy(tab->browserWindow());
    }
}

void BesraWindow::onTabMoved(int from, int to)
{
    /* Keep tab_stack_'s widget order matching tab_bar_'s (drag-to-
     * reorder), since every other lookup assumes index parity between
     * the two. QStackedWidget has no direct "move" call; remove +
     * re-insert at the target index does the same thing. */
    QWidget *widget = tab_stack_->widget(from);
    if (widget != nullptr) {
        tab_stack_->removeWidget(widget);
        tab_stack_->insertWidget(to, widget);
        tab_stack_->setCurrentIndex(tab_bar_->currentIndex());
    }
}

void BesraWindow::onMinimize()
{
    showMinimized();
}

void BesraWindow::onMaximizeRestore()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void BesraWindow::updateMaximizeIcon()
{
    if (maximize_button_ == nullptr) {
        return;
    }
    maximize_button_->setText(isMaximized() ? QStringLiteral("❐") : QStringLiteral("☐"));
    maximize_button_->setToolTip(isMaximized() ? tr("Restore") : tr("Maximize"));
}

void BesraWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        /* Catches maximize/restore triggered any way other than our own
         * button/double-click (a window-manager shortcut, snapping to
         * an edge, etc.), so the button's glyph never goes stale. */
        updateMaximizeIcon();
    }
}

Qt::Edges BesraWindow::resizeEdgeAt(const QPoint &pos) const
{
    Qt::Edges edges;
    if (pos.x() <= kResizeMargin) {
        edges |= Qt::LeftEdge;
    } else if (pos.x() >= width() - kResizeMargin) {
        edges |= Qt::RightEdge;
    }
    if (pos.y() <= kResizeMargin) {
        edges |= Qt::TopEdge;
    } else if (pos.y() >= height() - kResizeMargin) {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

bool BesraWindow::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *w = qobject_cast<QWidget *>(watched);
    if (w == nullptr || (w != this && !isAncestorOf(w))) {
        return QMainWindow::eventFilter(watched, event);
    }
    if (isMaximized() || isFullScreen()) {
        return QMainWindow::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseMove) {
        auto *me = static_cast<QMouseEvent *>(event);
        QPoint local = mapFromGlobal(me->globalPosition().toPoint());
        if (!rect().contains(local)) {
            return QMainWindow::eventFilter(watched, event);
        }
        Qt::Edges edges = resizeEdgeAt(local);
        if (edges == (Qt::TopEdge | Qt::LeftEdge) || edges == (Qt::BottomEdge | Qt::RightEdge)) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (edges == (Qt::TopEdge | Qt::RightEdge) || edges == (Qt::BottomEdge | Qt::LeftEdge)) {
            setCursor(Qt::SizeBDiagCursor);
        } else if (edges & (Qt::LeftEdge | Qt::RightEdge)) {
            setCursor(Qt::SizeHorCursor);
        } else if (edges & (Qt::TopEdge | Qt::BottomEdge)) {
            setCursor(Qt::SizeVerCursor);
        } else {
            unsetCursor();
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            QPoint local = mapFromGlobal(me->globalPosition().toPoint());
            Qt::Edges edges = rect().contains(local) ? resizeEdgeAt(local) : Qt::Edges();
            if (edges != Qt::Edges()) {
                if (QWindow *wh = windowHandle()) {
                    wh->startSystemResize(edges);
                    return true;
                }
            }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void BesraWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
}

void BesraWindow::focusInEvent(QFocusEvent *event)
{
    QMainWindow::focusInEvent(event);
    registry().removeAll(this);
    registry().prepend(this);
}

void BesraWindow::keyReleaseEvent(QKeyEvent *event)
{
    /* Bare Alt press/release (no other modifiers, not a mnemonic combo
     * like Alt+F) toggles the traditional menu bar, which is otherwise
     * hidden -- matches Firefox/Chrome/Explorer's "press Alt to reveal
     * the hidden menu bar" convention. */
    if (event->key() == Qt::Key_Alt && !event->isAutoRepeat()) {
        menuBar()->setVisible(!menuBar()->isVisible());
        return;
    }
    QMainWindow::keyReleaseEvent(event);
}
