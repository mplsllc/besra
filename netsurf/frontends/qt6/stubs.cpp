/*
 * Frontend hooks not yet implemented natively. Each is scoped to its own
 * upcoming milestone (see plan.md); until then they're safe, minimal
 * fallbacks rather than gtk3-borrowed or hardcoded behaviour.
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>

extern "C" {
#include "netsurf/netsurf.h"
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "netsurf/bitmap.h"
#include "netsurf/clipboard.h"
#include "netsurf/core_window.h"
#include "netsurf/download.h"
#include "netsurf/fetch.h"
#include "netsurf/layout.h"
#include "netsurf/misc.h"
#include "netsurf/search.h"
#include "netsurf/mouse.h"
#include "netsurf/window.h"
#include "desktop/searchweb.h"
}

/* Find-in-page forward/back button state: wired once the find dialog
 * (BesraWindow::onFindInPage) exists. */
extern "C" void gui_search_forward_state(bool active, void *p) {
    (void)active;
    (void)p;
}

extern "C" void gui_search_back_state(bool active, void *p) {
    (void)active;
    (void)p;
}

extern "C" void gui_clipboard_get(char **buffer, size_t *length) {
    QString text = QApplication::clipboard()->text();
    QByteArray utf8 = text.toUtf8();
    *length = static_cast<size_t>(utf8.size());
    *buffer = static_cast<char *>(malloc(*length));
    if (*buffer) {
        memcpy(*buffer, utf8.constData(), *length);
    } else {
        *length = 0;
    }
}

extern "C" void gui_clipboard_set(const char *buffer, size_t length,
        nsclipboard_styles styles[], int n_styles) {
    (void)styles;
    (void)n_styles;
    QApplication::clipboard()->setText(QString::fromUtf8(buffer, static_cast<int>(length)));
}

/* Download manager: wired in the downloads milestone. */
extern "C" struct gui_download_window * gui_download_create(struct download_context *ctx, struct gui_window *parent) {
    (void)ctx;
    (void)parent;
    return NULL;
}

extern "C" nserror gui_download_data(struct gui_download_window *dw, const char *data, unsigned int size) {
    (void)dw;
    (void)data;
    (void)size;
    return NSERROR_OK;
}

extern "C" void gui_download_error(struct gui_download_window *dw, const char *error_msg) {
    (void)dw;
    (void)error_msg;
}

extern "C" void gui_download_done(struct gui_download_window *dw) {
    (void)dw;
}

extern "C" nserror gui_misc_launch_url(struct nsurl *url) {
    return QDesktopServices::openUrl(QUrl(QString::fromUtf8(nsurl_access(url))))
        ? NSERROR_OK : NSERROR_NO_FETCH_HANDLER;
}

/* Cookie manager: wired in the corewindow/history/hotlist/cookies milestone. */
extern "C" nserror gui_misc_present_cookies(const char *search_term) {
    (void)search_term;
    return NSERROR_OK;
}

/* core_window (the generic list-canvas the core draws history/hotlist/
 * cookies/page-info through): wired in the corewindow milestone. */
extern "C" nserror gui_corewindow_invalidate(struct core_window *cw, const struct rect *rect) {
    (void)cw;
    (void)rect;
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_set_extent(struct core_window *cw, int width, int height) {
    (void)cw;
    (void)width;
    (void)height;
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_set_scroll(struct core_window *cw, int x, int y) {
    (void)cw;
    (void)x;
    (void)y;
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_get_scroll(const struct core_window *cw, int *x, int *y) {
    (void)cw;
    if (x) *x = 0;
    if (y) *y = 0;
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_get_dimensions(const struct core_window *cw,
		int *width, int *height) {
    (void)cw;
    if (width) *width = 0;
    if (height) *height = 0;
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_drag_status(struct core_window *cw,
		core_window_drag_status ds) {
    (void)cw;
    (void)ds;
    return NSERROR_OK;
}

extern "C" nserror gui_search_web_provider_update(const char *provider_name, struct bitmap *ico_bitmap) {
    (void)provider_name;
    (void)ico_bitmap;
    return NSERROR_OK;
}
