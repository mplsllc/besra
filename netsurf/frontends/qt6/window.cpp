/*
 * Implements the gui_window_* contract (include/netsurf/window.h) on top of
 * BrowserTab/NSWidget. struct gui_window is nothing more than the BrowserTab
 * the core is talking about; the actual widget/state lives there.
 */

#include "browsertab.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCursor>
#include <QMenu>
#include <QFileDialog>
#include <QAction>
#include <QStatusBar>

extern "C" {
#include "utils/errors.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/mouse.h"
#include "netsurf/window.h"
#include "utils/nsurl.h"
#include "desktop/browser_history.h"
}

struct gui_window {
    BrowserTab *tab;
};

extern "C" struct gui_window *gui_window_create(struct browser_window *bw,
        struct gui_window *existing, gui_window_create_flags flags)
{
    BrowserTab *tab = BesraWindow::createTabOrWindow(bw, existing, static_cast<unsigned>(flags));
    if (tab == nullptr) {
        return nullptr;
    }

    struct gui_window *gw = new gui_window();
    gw->tab = tab;
    return gw;
}

extern "C" void gui_window_destroy(struct gui_window *gw)
{
    if (gw->tab && gw->tab->window()) {
        gw->tab->window()->removeTab(gw->tab);
    }
    delete gw;
}

extern "C" nserror gui_window_invalidate(struct gui_window *gw, const struct rect *rect)
{
    if (rect == nullptr) {
        gw->tab->renderWidget()->update();
    } else {
        gw->tab->renderWidget()->update(QRect(rect->x0, rect->y0,
            rect->x1 - rect->x0, rect->y1 - rect->y0));
    }
    return NSERROR_OK;
}

extern "C" bool gui_window_get_scroll(struct gui_window *gw, int *sx, int *sy)
{
    if (sx) *sx = gw->tab->scrollX();
    if (sy) *sy = gw->tab->scrollY();
    return true;
}

extern "C" nserror gui_window_set_scroll(struct gui_window *gw, const struct rect *rect)
{
    gw->tab->setScroll(rect->x0, rect->y0);
    return NSERROR_OK;
}

extern "C" nserror gui_window_get_dimensions(struct gui_window *gw, int *width, int *height)
{
    if (width) *width = gw->tab->viewport()->width();
    if (height) *height = gw->tab->viewport()->height();
    return NSERROR_OK;
}

extern "C" nserror gui_window_event(struct gui_window *gw, enum gui_window_event event)
{
    switch (event) {
    case GW_EVENT_START_THROBBER:
        gw->tab->setLoading(true);
        break;
    case GW_EVENT_STOP_THROBBER:
        gw->tab->setLoading(false);
        break;
    case GW_EVENT_UPDATE_EXTENT:
    case GW_EVENT_NEW_CONTENT:
        gw->tab->updateContentExtent();
        break;
    default:
        break;
    }
    return NSERROR_OK;
}

extern "C" void gui_window_set_title(struct gui_window *gw, const char *title)
{
    gw->tab->setTitle(QString::fromUtf8(title ? title : ""));
}

extern "C" nserror gui_window_set_url(struct gui_window *gw, struct nsurl *url)
{
    gw->tab->setUrl(QString::fromUtf8(nsurl_access(url)));
    return NSERROR_OK;
}

extern "C" void gui_window_set_icon(struct gui_window *gw, struct hlcache_handle *icon)
{
    (void)gw;
    (void)icon;
    /* Favicon rendering: deferred, not required for functional parity. */
}

extern "C" void gui_window_set_status(struct gui_window *gw, const char *text)
{
    if (gw->tab->window()) {
        gw->tab->window()->statusBar()->showMessage(QString::fromUtf8(text ? text : ""));
    }
}

extern "C" void gui_window_set_pointer(struct gui_window *gw, gui_pointer_shape shape)
{
    Qt::CursorShape cursor = Qt::ArrowCursor;
    switch (shape) {
    case GUI_POINTER_POINT:      cursor = Qt::PointingHandCursor; break;
    case GUI_POINTER_CARET:      cursor = Qt::IBeamCursor; break;
    case GUI_POINTER_MOVE:       cursor = Qt::SizeAllCursor; break;
    case GUI_POINTER_RIGHT:
    case GUI_POINTER_LEFT:       cursor = Qt::SizeHorCursor; break;
    case GUI_POINTER_UP:
    case GUI_POINTER_DOWN:       cursor = Qt::SizeVerCursor; break;
    case GUI_POINTER_LD:
    case GUI_POINTER_RU:         cursor = Qt::SizeBDiagCursor; break;
    case GUI_POINTER_LU:
    case GUI_POINTER_RD:         cursor = Qt::SizeFDiagCursor; break;
    case GUI_POINTER_CROSS:      cursor = Qt::CrossCursor; break;
    case GUI_POINTER_WAIT:       cursor = Qt::WaitCursor; break;
    case GUI_POINTER_PROGRESS:   cursor = Qt::BusyCursor; break;
    case GUI_POINTER_NO_DROP:
    case GUI_POINTER_NOT_ALLOWED: cursor = Qt::ForbiddenCursor; break;
    case GUI_POINTER_MENU:       cursor = Qt::ArrowCursor; break;
    case GUI_POINTER_HELP:       cursor = Qt::WhatsThisCursor; break;
    default: break;
    }
    gw->tab->renderWidget()->setCursor(cursor);
}

extern "C" void gui_window_place_caret(struct gui_window *gw, int x, int y, int height,
        const struct rect *clip)
{
    (void)gw;
    (void)x;
    (void)y;
    (void)height;
    (void)clip;
    /* Text-entry caret is drawn by the core's own redraw of the focused
     * text box; nothing extra is required here for basic form editing. */
}

extern "C" void gui_window_create_form_select_menu(struct gui_window *gw, struct form_control *control)
{
    (void)control;
    /* A full <select> popup needs the option-list accessor from
     * desktop/textarea.h / render/form.h; deferred alongside forms polish. */
    (void)gw;
}

extern "C" void gui_window_file_gadget_open(struct gui_window *gw, struct hlcache_handle *hl,
        struct form_control *gadget)
{
    (void)hl;
    QString path = QFileDialog::getOpenFileName(gw->tab->window(), QObject::tr("Choose File"));
    if (!path.isEmpty()) {
        browser_window_set_gadget_filename(gw->tab->browserWindow(), gadget,
            path.toUtf8().constData());
    }
}
