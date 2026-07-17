#include <stdlib.h>
#include <stdbool.h>
extern "C" {
#include "utils/errors.h"
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

extern "C" nserror gui_layout_width(const struct plot_font_style *fstyle, const char *string, size_t length, int *width) {
    return NSERROR_OK;
}

extern "C" nserror gui_layout_position(const struct plot_font_style *fstyle, const char *string, size_t length, int x, size_t *char_offset, int *actual_x) {
    return NSERROR_OK;
}

extern "C" nserror gui_layout_split(const struct plot_font_style *fstyle, const char *string, size_t length, int x, size_t *char_offset, int *actual_x) {
    return NSERROR_OK;
}

extern "C" struct gui_window * gui_window_create(struct browser_window *bw,
		struct gui_window *existing,
		gui_window_create_flags flags) {
    return NULL;
}

extern "C" void gui_window_destroy(struct gui_window *gw) {
    
}

extern "C" nserror gui_window_invalidate(struct gui_window *gw, const struct rect *rect) {
    return NSERROR_OK;
}

extern "C" bool gui_window_get_scroll(struct gui_window *gw, int *sx, int *sy) {
    return false;
}

extern "C" nserror gui_window_set_scroll(struct gui_window *gw, const struct rect *rect) {
    return NSERROR_OK;
}

extern "C" nserror gui_window_get_dimensions(struct gui_window *gw, int *width, int *height) {
    return NSERROR_OK;
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

extern "C" const char * gui_fetch_filetype(const char *unix_path) {
    return NULL;
}

extern "C" struct nsurl * gui_fetch_get_resource_url(const char *path) {
    return NULL;
}

extern "C" nserror gui_fetch_get_resource_data(const char *path, const uint8_t **data, size_t *data_len) {
    return NSERROR_OK;
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

extern "C" void * gui_bitmap_create(int width, int height, enum gui_bitmap_flags flags) {
    return NULL;
}

extern "C" void gui_bitmap_destroy(void *bitmap) {
    
}

extern "C" void gui_bitmap_set_opaque(void *bitmap, bool opaque) {
    
}

extern "C" bool gui_bitmap_get_opaque(void *bitmap) {
    return false;
}

extern "C" unsigned char * gui_bitmap_get_buffer(void *bitmap) {
    return NULL;
}

extern "C" size_t gui_bitmap_get_rowstride(void *bitmap) {
    return 0;
}

extern "C" int gui_bitmap_get_width(void *bitmap) {
    return 0;
}

extern "C" int gui_bitmap_get_height(void *bitmap) {
    return 0;
}

extern "C" void gui_bitmap_modified(void *bitmap) {
    
}

extern "C" nserror gui_bitmap_render(struct bitmap *bitmap, struct hlcache_handle *content) {
    return NSERROR_OK;
}

extern "C" nserror gui_search_web_provider_update(const char *provider_name, struct bitmap *ico_bitmap) {
    return NSERROR_OK;
}

