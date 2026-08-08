#include "corewindow.h"

#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QScrollBar>

extern "C" {
#include "utils/errors.h"
#include "netsurf/mouse.h"
#include "netsurf/keypress.h"
#include "netsurf/plotters.h"
#include "netsurf/core_window.h"
}

extern const struct plotter_table nsqt_plotters;
extern QPainter *qt_current_painter;

namespace {

browser_mouse_state qt_buttons_to_mouse_state(Qt::MouseButtons buttons)
{
    unsigned state = 0;
    if (buttons & Qt::LeftButton) state |= BROWSER_MOUSE_PRESS_1;
    if (buttons & Qt::RightButton) state |= BROWSER_MOUSE_PRESS_2;
    if (buttons & Qt::MiddleButton) state |= BROWSER_MOUSE_PRESS_3;
    return static_cast<browser_mouse_state>(state);
}

uint32_t qt_key_to_nskey(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Backspace: return NS_KEY_DELETE_LEFT;
    case Qt::Key_Delete:    return NS_KEY_DELETE_RIGHT;
    case Qt::Key_Tab:       return NS_KEY_TAB;
    case Qt::Key_Return:
    case Qt::Key_Enter:     return NS_KEY_CR;
    case Qt::Key_Escape:    return NS_KEY_ESCAPE;
    case Qt::Key_Left:      return NS_KEY_LEFT;
    case Qt::Key_Right:     return NS_KEY_RIGHT;
    case Qt::Key_Up:        return NS_KEY_UP;
    case Qt::Key_Down:      return NS_KEY_DOWN;
    case Qt::Key_Home:      return NS_KEY_LINE_START;
    case Qt::Key_End:       return NS_KEY_LINE_END;
    case Qt::Key_PageUp:    return NS_KEY_PAGE_UP;
    case Qt::Key_PageDown:  return NS_KEY_PAGE_DOWN;
    default: break;
    }
    QString text = event->text();
    return text.isEmpty() ? 0 : text.at(0).unicode();
}

} // namespace

class CoreWindowWidget::Canvas : public QWidget {
public:
    Canvas(CoreWindowWidget::RedrawFn redraw, CoreWindowWidget::KeyFn key,
           CoreWindowWidget::MouseFn mouse, QWidget *parent)
        : QWidget(parent), redraw_(std::move(redraw)), key_(std::move(key)),
          mouse_(std::move(mouse))
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setMouseTracking(true);
        setFocusPolicy(Qt::StrongFocus);
        resize(600, 800);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        QPainter painter(this);
        qt_current_painter = &painter;

        QRect r = event->rect();
        struct rect clip;
        clip.x0 = r.left();
        clip.y0 = r.top();
        clip.x1 = r.right();
        clip.y1 = r.bottom();

        struct redraw_context ctx;
        ctx.interactive = true;
        ctx.background_images = true;
        ctx.plot = &nsqt_plotters;

        redraw_(0, 0, &clip, &ctx);

        qt_current_painter = nullptr;
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        setFocus(Qt::MouseFocusReason);
        mouse_(qt_buttons_to_mouse_state(event->buttons()), event->pos().x(), event->pos().y());
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        unsigned state = 0;
        switch (event->button()) {
        case Qt::LeftButton:  state = BROWSER_MOUSE_CLICK_1; break;
        case Qt::RightButton: state = BROWSER_MOUSE_CLICK_2; break;
        default: break;
        }
        mouse_(static_cast<browser_mouse_state>(state), event->pos().x(), event->pos().y());
    }

    void mouseMoveEvent(QMouseEvent *event) override
    {
        mouse_(qt_buttons_to_mouse_state(event->buttons()), event->pos().x(), event->pos().y());
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        uint32_t nskey = qt_key_to_nskey(event);
        if (nskey == 0 || !key_(nskey)) {
            QWidget::keyPressEvent(event);
        }
    }

private:
    CoreWindowWidget::RedrawFn redraw_;
    CoreWindowWidget::KeyFn key_;
    CoreWindowWidget::MouseFn mouse_;
};

CoreWindowWidget::CoreWindowWidget(RedrawFn redraw, KeyFn key, MouseFn mouse, QWidget *parent)
    : QScrollArea(parent)
{
    canvas_ = new Canvas(std::move(redraw), std::move(key), std::move(mouse), this);
    setWidget(canvas_);
    setWidgetResizable(false);
    setFrameShape(QFrame::NoFrame);
}

struct core_window *CoreWindowWidget::handle()
{
    return reinterpret_cast<struct core_window *>(this);
}

void CoreWindowWidget::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    if (canvas_->width() < viewport()->width() || canvas_->height() < viewport()->height()) {
        canvas_->resize(std::max(canvas_->width(), viewport()->width()),
                         std::max(canvas_->height(), viewport()->height()));
    }
}

/* gui_corewindow_* -- the core-facing contract (include/netsurf/core_window.h).
 * `cw` is always exactly the pointer a CoreWindowWidget::handle() produced. */

extern "C" nserror gui_corewindow_invalidate(struct core_window *cw, const struct rect *rect)
{
    auto *widget = reinterpret_cast<CoreWindowWidget *>(cw);
    if (rect == nullptr) {
        widget->widget()->update();
    } else {
        widget->widget()->update(QRect(rect->x0, rect->y0,
            rect->x1 - rect->x0, rect->y1 - rect->y0));
    }
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_set_extent(struct core_window *cw, int width, int height)
{
    auto *widget = reinterpret_cast<CoreWindowWidget *>(cw);
    widget->widget()->resize(std::max(width, widget->viewport()->width()),
                              std::max(height, widget->viewport()->height()));
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_set_scroll(struct core_window *cw, int x, int y)
{
    auto *widget = reinterpret_cast<CoreWindowWidget *>(cw);
    widget->horizontalScrollBar()->setValue(x);
    widget->verticalScrollBar()->setValue(y);
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_get_scroll(const struct core_window *cw, int *x, int *y)
{
    auto *widget = reinterpret_cast<CoreWindowWidget *>(const_cast<struct core_window *>(cw));
    if (x) *x = widget->horizontalScrollBar()->value();
    if (y) *y = widget->verticalScrollBar()->value();
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_get_dimensions(const struct core_window *cw,
        int *width, int *height)
{
    auto *widget = reinterpret_cast<CoreWindowWidget *>(const_cast<struct core_window *>(cw));
    if (width) *width = widget->viewport()->width();
    if (height) *height = widget->viewport()->height();
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_drag_status(struct core_window *cw, core_window_drag_status ds)
{
    (void)cw;
    (void)ds;
    return NSERROR_OK;
}
