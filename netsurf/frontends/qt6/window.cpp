#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <iostream>

extern "C" {
#include "utils/errors.h"
#include "netsurf/mouse.h"
#include "netsurf/content.h"
#include "netsurf/window.h"
#include "netsurf/browser_window.h"
#include "netsurf/plotters.h"
}

extern const struct plotter_table nsqt_plotters;
extern QPainter *qt_current_painter;

class NSWidget;

struct gui_window {
    struct browser_window *bw;
    NSWidget *widget;
    int scroll_x;
    int scroll_y;
};

class NSWidget : public QWidget {
public:
    struct gui_window *gw;
    
    NSWidget(struct gui_window *g, QWidget *parent = nullptr) : QWidget(parent), gw(g) {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
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
        
        // Pass negative scroll offsets so the core draws at the correct viewport position
        extern int get_scroll_x(struct gui_window *gw);
        extern int get_scroll_y(struct gui_window *gw);
        
        browser_window_redraw(gw->bw, -get_scroll_x(gw), -get_scroll_y(gw), &clip, &ctx);
        
        qt_current_painter = nullptr;
    }
};



int get_scroll_x(struct gui_window *gw) { return gw->scroll_x; }
int get_scroll_y(struct gui_window *gw) { return gw->scroll_y; }

extern "C" struct gui_window * gui_window_create(struct browser_window *bw, struct gui_window *existing, gui_window_create_flags flags) {
    struct gui_window *gw = new gui_window();
    gw->bw = bw;
    gw->scroll_x = 0;
    gw->scroll_y = 0;
    gw->widget = new NSWidget(gw);
    
    // Set a sensible default size to ensure we have a viewport for the layout engine
    gw->widget->resize(1024, 768);
    gw->widget->show();
    
    return gw;
}

extern "C" void gui_window_destroy(struct gui_window *gw) {
    delete gw->widget;
    delete gw;
}

extern "C" nserror gui_window_invalidate(struct gui_window *gw, const struct rect *rect) {
    gw->widget->update(QRect(rect->x0, rect->y0, rect->x1 - rect->x0, rect->y1 - rect->y0));
    return NSERROR_OK;
}

extern "C" bool gui_window_get_scroll(struct gui_window *gw, int *sx, int *sy) {
    if (sx) *sx = gw->scroll_x;
    if (sy) *sy = gw->scroll_y;
    return true;
}

extern "C" nserror gui_window_set_scroll(struct gui_window *gw, const struct rect *rect) {
    gw->scroll_x = rect->x0;
    gw->scroll_y = rect->y0;
    gw->widget->update();
    return NSERROR_OK;
}

extern "C" nserror gui_window_get_dimensions(struct gui_window *gw, int *width, int *height) {
    if (width) *width = gw->widget->width();
    if (height) *height = gw->widget->height();
    return NSERROR_OK;
}
