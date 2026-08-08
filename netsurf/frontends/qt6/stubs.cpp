#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
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

extern "C" void gui_search_forward_state(bool active, void *p) {
    
}

extern "C" void gui_search_back_state(bool active, void *p) {
    
}









extern "C" nserror gui_window_event(struct gui_window *gw, enum gui_window_event event) {
    return NSERROR_OK;
}

extern "C" void gui_window_set_title(struct gui_window *gw, const char *title) {
    
}

extern "C" nserror gui_window_set_url(struct gui_window *gw, struct nsurl *url) {
    return NSERROR_OK;
}

extern "C" void gui_window_set_icon(struct gui_window *gw, struct hlcache_handle *icon) {
    
}

extern "C" void gui_window_set_status(struct gui_window *g, const char *text) {
    
}

extern "C" void gui_window_set_pointer(struct gui_window *g, enum gui_pointer_shape shape) {
    
}

extern "C" void gui_window_place_caret(struct gui_window *g, int x, int y, int height, const struct rect *clip) {
    
}

extern "C" void gui_window_create_form_select_menu(struct gui_window *gw, struct form_control *control) {
    
}

extern "C" void gui_window_file_gadget_open(struct gui_window *gw, struct hlcache_handle *hl, struct form_control *gadget) {
    
}

extern "C" const char *gui_fetch_filetype(const char *unix_path) {
    if (strstr(unix_path, ".css")) return "text/css";
    if (strstr(unix_path, ".png")) return "image/png";
    return "text/html";
}

extern "C" struct nsurl * gui_fetch_get_resource_url(const char *path) {
    struct nsurl *url = NULL;
    char buf[1024];
    snprintf(buf, sizeof(buf), "file:///home/patrick/Webs/Besra/netsurf/frontends/gtk/res/%s", path);
    nsurl_create(buf, &url);
    return url;
}

extern "C" nserror gui_fetch_get_resource_data(const char *path, const uint8_t **data, size_t *data_len) {
    return NSERROR_NOT_FOUND;
}

extern "C" void gui_clipboard_get(char **buffer, size_t *length) {
    
}

extern "C" void gui_clipboard_set(const char *buffer, size_t length, nsclipboard_styles styles[], int n_styles) {
    
}

extern "C" struct gui_download_window * gui_download_create(struct download_context *ctx, struct gui_window *parent) {
    return NULL;
}

extern "C" nserror gui_download_data(struct gui_download_window *dw, const char *data, unsigned int size) {
    return NSERROR_OK;
}

extern "C" void gui_download_error(struct gui_download_window *dw, const char *error_msg) {
    
}

extern "C" void gui_download_done(struct gui_download_window *dw) {
    
}

extern "C" nserror gui_misc_launch_url(struct nsurl *url) {
    return NSERROR_OK;
}

extern "C" nserror gui_misc_present_cookies(const char *search_term) {
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_invalidate(struct core_window *cw, const struct rect *rect) {
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_set_extent(struct core_window *cw, int width, int height) {
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_set_scroll(struct core_window *cw, int x, int y) {
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_get_scroll(const struct core_window *cw, int *x, int *y) {
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_get_dimensions(const struct core_window *cw,
		int *width, int *height) {
    return NSERROR_OK;
}

extern "C" nserror gui_corewindow_drag_status(struct core_window *cw,
		core_window_drag_status ds) {
    return NSERROR_OK;
}


extern "C" nserror gui_search_web_provider_update(const char *provider_name, struct bitmap *ico_bitmap) {
    return NSERROR_OK;
}

