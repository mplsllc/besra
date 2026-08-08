#ifndef BESRA_QT6_BROWSERTAB_H
#define BESRA_QT6_BROWSERTAB_H

#include <QScrollArea>
#include <QString>

extern "C" {
struct browser_window;
}

class BesraWindow;

/**
 * The per-tab render surface: an NSWidget (the actual plotter target) inside
 * a scroll area, plus enough state for BesraWindow to reflect this tab's
 * navigation status in the chrome (title, url, throbber, back/forward).
 *
 * This is the sole owner of the browser_window's frontend-side identity: the
 * opaque `struct gui_window` the core is given back is nothing more than a
 * pointer to a BrowserTab (see window.cpp).
 */
class BrowserTab : public QScrollArea {
    Q_OBJECT

public:
    BrowserTab(struct browser_window *bw, BesraWindow *window, QWidget *parent = nullptr);
    ~BrowserTab() override;

    struct browser_window *browserWindow() const { return bw_; }
    /** The plotter render surface. Only QWidget-level operations (update(),
     * setCursor(), setFocus(), ...) are meant to be used from outside
     * browsertab.cpp, hence the QWidget* return type. */
    QWidget *renderWidget() const;
    BesraWindow *window() const { return window_; }

    const QString &title() const { return title_; }
    const QString &url() const { return url_; }
    bool loading() const { return loading_; }

    int scrollX() const { return scroll_x_; }
    int scrollY() const { return scroll_y_; }
    void setScroll(int x, int y);

    void setTitle(const QString &title);
    void setUrl(const QString &url);
    void setLoading(bool loading);

    /** Re-query the core for the full content size and update scrollbars. */
    void updateContentExtent();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onScrolled();

private:
    struct browser_window *bw_;
    class NSWidget *render_widget_;
    BesraWindow *window_;
    QString title_;
    QString url_;
    bool loading_ = false;
    int scroll_x_ = 0;
    int scroll_y_ = 0;
};

#endif
