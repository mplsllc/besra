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

/* Download manager: implemented in downloads.cpp. */

extern "C" nserror gui_misc_launch_url(struct nsurl *url) {
    return QDesktopServices::openUrl(QUrl(QString::fromUtf8(nsurl_access(url))))
        ? NSERROR_OK : NSERROR_NO_FETCH_HANDLER;
}

/* Cookie manager: wired in the corewindow/history/hotlist/cookies milestone. */
extern "C" nserror gui_misc_present_cookies(const char *search_term) {
    (void)search_term;
    return NSERROR_OK;
}

extern "C" nserror gui_search_web_provider_update(const char *provider_name, struct bitmap *ico_bitmap) {
    (void)provider_name;
    (void)ico_bitmap;
    return NSERROR_OK;
}
