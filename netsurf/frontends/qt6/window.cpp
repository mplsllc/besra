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
#include <QImage>
#include <QPixmap>
#include <QIcon>

extern "C" {
#include "utils/errors.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/mouse.h"
#include "netsurf/window.h"
#include "netsurf/form.h"
#include "utils/nsurl.h"
#include "desktop/browser_history.h"
}

/* bitmap.cpp's QImage accessor for a struct bitmap*, used to build a
 * favicon QIcon directly rather than duplicating pixel-buffer plumbing. */
extern QImage *gui_bitmap_get_qimage(struct bitmap *vbitmap);

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
    if (icon == nullptr) {
        return;
    }
    struct bitmap *bmp = content_get_bitmap(icon);
    if (bmp == nullptr) {
        return;
    }
    QImage *image = gui_bitmap_get_qimage(bmp);
    if (image == nullptr || image->isNull()) {
        return;
    }
    gw->tab->setFavicon(QIcon(QPixmap::fromImage(*image)));
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
    QWidget *surface = gw->tab->renderWidget();

    QMenu menu(surface);
    for (int i = 0; ; i++) {
        struct form_option *option = form_select_get_option(control, i);
        if (option == nullptr) {
            break;
        }
        QAction *action = menu.addAction(QString::fromUtf8(option->text));
        action->setCheckable(true);
        action->setChecked(option->selected);
        QObject::connect(action, &QAction::triggered, surface,
            [control, i] { form_select_process_selection(control, i); });
    }

    if (menu.isEmpty()) {
        return;
    }

    struct rect bounds;
    QPoint popup_pos = surface->mapToGlobal(QPoint(0, 0));
    if (form_control_bounding_rect(control, &bounds) == NSERROR_OK) {
        popup_pos = surface->mapToGlobal(QPoint(bounds.x0 - gw->tab->scrollX(),
            bounds.y1 - gw->tab->scrollY()));
    }
    menu.exec(popup_pos);
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
