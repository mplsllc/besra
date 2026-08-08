#include "browsertab.h"
#include "mainwindow.h"

#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QCursor>

extern "C" {
#include "utils/errors.h"
#include "netsurf/mouse.h"
#include "netsurf/keypress.h"
#include "netsurf/content.h"
#include "netsurf/window.h"
#include "netsurf/browser_window.h"
#include "netsurf/plotters.h"
}

extern const struct plotter_table nsqt_plotters;
extern QPainter *qt_current_painter;

namespace {

/** Map a Qt key event to NetSurf's NS_KEY_* / plain-codepoint key model. */
uint32_t qt_key_to_nskey(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Backspace: return NS_KEY_DELETE_LEFT;
    case Qt::Key_Delete:    return NS_KEY_DELETE_RIGHT;
    case Qt::Key_Tab:       return (event->modifiers() & Qt::ShiftModifier) ? NS_KEY_SHIFT_TAB : NS_KEY_TAB;
    case Qt::Key_Return:
    case Qt::Key_Enter:     return NS_KEY_CR;
    case Qt::Key_Escape:    return NS_KEY_ESCAPE;
    case Qt::Key_Left:
        if (event->modifiers() & Qt::ControlModifier) return NS_KEY_WORD_LEFT;
        return NS_KEY_LEFT;
    case Qt::Key_Right:
        if (event->modifiers() & Qt::ControlModifier) return NS_KEY_WORD_RIGHT;
        return NS_KEY_RIGHT;
    case Qt::Key_Up:        return NS_KEY_UP;
    case Qt::Key_Down:      return NS_KEY_DOWN;
    case Qt::Key_Home:
        if (event->modifiers() & Qt::ControlModifier) return NS_KEY_TEXT_START;
        return NS_KEY_LINE_START;
    case Qt::Key_End:
        if (event->modifiers() & Qt::ControlModifier) return NS_KEY_TEXT_END;
        return NS_KEY_LINE_END;
    case Qt::Key_PageUp:    return NS_KEY_PAGE_UP;
    case Qt::Key_PageDown:  return NS_KEY_PAGE_DOWN;
    default:
        break;
    }

    if (event->modifiers() & Qt::ControlModifier) {
        switch (event->key()) {
        case Qt::Key_A: return NS_KEY_SELECT_ALL;
        case Qt::Key_C: return NS_KEY_COPY_SELECTION;
        case Qt::Key_V: return NS_KEY_PASTE;
        case Qt::Key_X: return NS_KEY_CUT_SELECTION;
        case Qt::Key_Z: return NS_KEY_UNDO;
        case Qt::Key_Y: return NS_KEY_REDO;
        default: break;
        }
    }

    QString text = event->text();
    if (!text.isEmpty()) {
        return text.at(0).unicode();
    }
    return 0;
}

browser_mouse_state qt_buttons_to_mouse_state(Qt::MouseButtons buttons)
{
    unsigned state = 0;
    if (buttons & Qt::LeftButton) state |= BROWSER_MOUSE_PRESS_1;
    if (buttons & Qt::RightButton) state |= BROWSER_MOUSE_PRESS_2;
    if (buttons & Qt::MiddleButton) state |= BROWSER_MOUSE_PRESS_3;
    return static_cast<browser_mouse_state>(state);
}

} // namespace

class NSWidget : public QWidget {
public:
    explicit NSWidget(BrowserTab *tab, QWidget *parent = nullptr)
        : QWidget(parent), tab_(tab)
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        qt_current_painter = &painter;

        struct rect clip;
        QRect r = event->rect();
        clip.x0 = r.left();
        clip.y0 = r.top();
        clip.x1 = r.right();
        clip.y1 = r.bottom();

        struct redraw_context ctx;
        ctx.interactive = true;
        ctx.background_images = true;
        ctx.plot = &nsqt_plotters;

        browser_window_redraw(tab_->browserWindow(), -tab_->scrollX(), -tab_->scrollY(), &clip, &ctx);

        qt_current_painter = nullptr;
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        setFocus(Qt::MouseFocusReason);
        browser_window_mouse_click(tab_->browserWindow(),
            qt_buttons_to_mouse_state(event->buttons()), event->pos().x(), event->pos().y());
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        unsigned state = 0;
        switch (event->button()) {
        case Qt::LeftButton:   state = BROWSER_MOUSE_CLICK_1; break;
        case Qt::RightButton:  state = BROWSER_MOUSE_CLICK_2; break;
        case Qt::MiddleButton: state = BROWSER_MOUSE_CLICK_3; break;
        default: break;
        }
        browser_window_mouse_click(tab_->browserWindow(),
            static_cast<browser_mouse_state>(state), event->pos().x(), event->pos().y());
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        browser_window_mouse_track(tab_->browserWindow(),
            qt_buttons_to_mouse_state(event->buttons()), event->pos().x(), event->pos().y());
    }

    void wheelEvent(QWheelEvent *event) override
    {
        int dy = event->angleDelta().y() / 2;
        tab_->setScroll(tab_->scrollX(), tab_->scrollY() - dy);
        event->accept();
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        uint32_t key = qt_key_to_nskey(event);
        if (key == 0 || !browser_window_key_press(tab_->browserWindow(), key)) {
            QWidget::keyPressEvent(event);
        }
    }

private:
    BrowserTab *tab_;
};

QWidget *BrowserTab::renderWidget() const
{
    return render_widget_;
}

BrowserTab::BrowserTab(struct browser_window *bw, BesraWindow *window, QWidget *parent)
    : QScrollArea(parent), bw_(bw), window_(window)
{
    render_widget_ = new NSWidget(this);
    render_widget_->resize(1024, 768);
    setWidget(render_widget_);
    setWidgetResizable(false);
    setFrameShape(QFrame::NoFrame);

    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &BrowserTab::onScrolled);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &BrowserTab::onScrolled);
}

BrowserTab::~BrowserTab() = default;

void BrowserTab::setScroll(int x, int y)
{
    x = std::max(0, x);
    y = std::max(0, y);
    scroll_x_ = x;
    scroll_y_ = y;
    /* Reflect into the real scrollbars without re-entering onScrolled's core
     * round trip (blockSignals avoids a redundant browser_window notify). */
    horizontalScrollBar()->blockSignals(true);
    verticalScrollBar()->blockSignals(true);
    horizontalScrollBar()->setValue(x);
    verticalScrollBar()->setValue(y);
    horizontalScrollBar()->blockSignals(false);
    verticalScrollBar()->blockSignals(false);
    render_widget_->update();
}

void BrowserTab::onScrolled()
{
    scroll_x_ = horizontalScrollBar()->value();
    scroll_y_ = verticalScrollBar()->value();
    render_widget_->update();
}

void BrowserTab::setTitle(const QString &title)
{
    title_ = title;
    if (window_) {
        window_->refreshChrome(this);
    }
}

void BrowserTab::setUrl(const QString &url)
{
    url_ = url;
    if (window_) {
        window_->refreshChrome(this);
    }
}

void BrowserTab::setLoading(bool loading)
{
    loading_ = loading;
    if (window_) {
        window_->refreshChrome(this);
    }
}

void BrowserTab::updateContentExtent()
{
    int width = 0, height = 0;
    if (browser_window_get_extents(bw_, false, &width, &height) == NSERROR_OK) {
        render_widget_->resize(std::max(width, viewport()->width()),
                                std::max(height, viewport()->height()));
    }
}

void BrowserTab::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    updateContentExtent();
    browser_window_schedule_reformat(bw_);
}
