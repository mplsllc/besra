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
#include <QTabWidget>
#include <QTabBar>
#include <QStatusBar>
#include <QCloseEvent>
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

    tabs_ = new QTabWidget(this);
    tabs_->setTabsClosable(true);
    tabs_->setMovable(true);
    setCentralWidget(tabs_);

    status_label_ = new QLabel(this);
    statusBar()->addWidget(status_label_, 1);

    buildToolbar();
    buildMenus();

    connect(tabs_, &QTabWidget::currentChanged, this, &BesraWindow::onCurrentTabChanged);
    connect(tabs_, &QTabWidget::tabCloseRequested, this, &BesraWindow::onTabCloseRequested);

    resize(1200, 800);
    setWindowTitle(QStringLiteral("Besra"));
}

BesraWindow::~BesraWindow()
{
    registry().removeAll(this);
}

void BesraWindow::buildToolbar()
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
}

void BesraWindow::buildMenus()
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
}

BrowserTab *BesraWindow::addTab(struct browser_window *bw, bool foreground)
{
    BrowserTab *tab = new BrowserTab(bw, this, tabs_);
    int index = tabs_->addTab(tab, tr("New Tab"));
    if (foreground) {
        tabs_->setCurrentIndex(index);
    }
    return tab;
}

void BesraWindow::removeTab(BrowserTab *tab)
{
    int index = tabs_->indexOf(tab);
    if (index >= 0) {
        tabs_->removeTab(index);
    }
    tab->deleteLater();

    if (tabs_->count() == 0) {
        close();
    }
}

BrowserTab *BesraWindow::currentTab() const
{
    return qobject_cast<BrowserTab *>(tabs_->currentWidget());
}

void BesraWindow::refreshChrome(BrowserTab *tab)
{
    int index = tabs_->indexOf(tab);
    if (index >= 0) {
        QString label = tab->title().isEmpty() ? tr("New Tab") : tab->title();
        tabs_->setTabText(index, label.left(32));
        if (!tab->favicon().isNull()) {
            tabs_->setTabIcon(index, tab->favicon());
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

    bool foreground = (flags & GW_CREATE_FOREGROUND) || target->tabs_->count() == 0;
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
        browser_window_create(BW_CREATE_HISTORY, url, nullptr, nullptr, nullptr);
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
    (void)index;
    if (BrowserTab *tab = currentTab()) {
        refreshChrome(tab);
        tab->renderWidget()->setFocus();
    }
}

void BesraWindow::onTabCloseRequested(int index)
{
    if (auto *tab = qobject_cast<BrowserTab *>(tabs_->widget(index))) {
        browser_window_destroy(tab->browserWindow());
    }
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
