#ifndef BESRA_QT6_MAINWINDOW_H
#define BESRA_QT6_MAINWINDOW_H

#include <QMainWindow>
#include <QList>

extern "C" {
struct browser_window;
struct gui_window;
}

class QLineEdit;
class QTabWidget;
class QToolButton;
class QLabel;
class BrowserTab;

/**
 * A top-level Besra browser window: menu bar, navigation toolbar (back/
 * forward/reload-stop/url bar), a tab strip, and a status bar. Each tab
 * holds one browsing context (struct browser_window).
 */
class BesraWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit BesraWindow(QWidget *parent = nullptr);
    ~BesraWindow() override;

    /** Add a new tab for the given browsing context and make it current. */
    BrowserTab *addTab(struct browser_window *bw, bool foreground = true);

    /** Remove and delete a tab (does not touch the core browser_window). */
    void removeTab(BrowserTab *tab);

    BrowserTab *currentTab() const;

    /** Refresh toolbar/status/title chrome if `tab` is the active tab. */
    void refreshChrome(BrowserTab *tab);

    /** The most recently active Besra window, or nullptr if none exist. */
    static BesraWindow *activeWindow();

    /**
     * The single entry point the gui_window_create() C hook routes through:
     * decides whether to open a new tab in an existing window or a whole
     * new window, per the create flags, and returns the BrowserTab to wrap.
     */
    static BrowserTab *createTabOrWindow(struct browser_window *bw,
                                          struct gui_window *existing,
                                          unsigned flags);

protected:
    void closeEvent(QCloseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private slots:
    void onNavigate();
    void onBack();
    void onForward();
    void onReloadOrStop();
    void onNewTab();
    void onNewWindow();
    void onCloseTab();
    void onOpenFile();
    void onPrint();
    void onFindInPage();
    void onViewSource();
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onShowHistory();
    void onShowBookmarks();
    void onAddBookmark();
    void onShowCookies();
    void onShowDownloads();
    void onShowPreferences();
    void onShowAbout();
    void onCurrentTabChanged(int index);
    void onTabCloseRequested(int index);

private:
    void buildMenus();
    void buildToolbar();
    void navigateTo(const QString &text);

    QTabWidget *tabs_;
    QLineEdit *url_bar_;
    QToolButton *back_button_;
    QToolButton *forward_button_;
    QToolButton *reload_stop_button_;
    QLabel *status_label_;

    static QList<BesraWindow *> &registry();
};

#endif
