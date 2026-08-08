#ifndef BESRA_QT6_COREWINDOW_H
#define BESRA_QT6_COREWINDOW_H

#include <QScrollArea>
#include <functional>

extern "C" {
#include "netsurf/mouse.h"
struct rect;
struct redraw_context;
}

/**
 * A generic Qt implementation of NetSurf's `core_window` contract
 * (include/netsurf/core_window.h): the canvas the core draws generic
 * treeview-based UI through (global history, hotlist/bookmarks, the cookie
 * manager, page info all share this one drawing/input contract).
 *
 * The core never dereferences a `struct core_window *`; by convention it's
 * simply the pointer the frontend originally handed to the feature's
 * `..._init(core_window_handle)` call, handed back opaquely on every
 * gui_corewindow_* callback. So a CoreWindowWidget's `this`, reinterpreted
 * as `struct core_window *`, IS its own handle -- see handle() below and
 * corewindow.cpp's gui_corewindow_* definitions.
 *
 * Each feature (history/hotlist/cookies) supplies the three functions that
 * differ per feature: redraw the canvas, forward a keypress, forward a
 * mouse action. Everything else (scrolling, sizing, invalidation) is
 * handled identically here for all of them.
 */
class CoreWindowWidget : public QScrollArea {
    Q_OBJECT

public:
    using RedrawFn = std::function<void(int x, int y, const struct rect *clip,
                                         const struct redraw_context *ctx)>;
    using KeyFn = std::function<bool(uint32_t nskey)>;
    using MouseFn = std::function<void(browser_mouse_state mouse, int x, int y)>;

    CoreWindowWidget(RedrawFn redraw, KeyFn key, MouseFn mouse, QWidget *parent = nullptr);

    /** The opaque core_window_handle to pass to a feature's ..._init(). */
    struct core_window *handle();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    class Canvas;
    Canvas *canvas_;
};

#endif
