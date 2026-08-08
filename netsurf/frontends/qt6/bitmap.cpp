#include <QImage>

extern "C" {
#include "utils/errors.h"
#include "netsurf/bitmap.h"
#include "netsurf/plotters.h"
}

struct bitmap {
    QImage *image;
    bool opaque;
};

extern "C" void * gui_bitmap_create(int width, int height, enum gui_bitmap_flags flags) {
    if (width == 0 || height == 0) return nullptr;
    
    struct bitmap *gbitmap = new bitmap();
    gbitmap->opaque = (flags & BITMAP_OPAQUE);
    gbitmap->image = new QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    if (gbitmap->image->isNull()) {
        delete gbitmap->image;
        delete gbitmap;
        return nullptr;
    }
    
    gbitmap->image->fill(0);
    
    return gbitmap;
}

extern "C" void gui_bitmap_destroy(void *vbitmap) {
    if (!vbitmap) return;
    struct bitmap *gbitmap = (struct bitmap *)vbitmap;
    delete gbitmap->image;
    delete gbitmap;
}

extern "C" void gui_bitmap_set_opaque(void *vbitmap, bool opaque) {
    if (!vbitmap) return;
    struct bitmap *gbitmap = (struct bitmap *)vbitmap;
    gbitmap->opaque = opaque;
}

extern "C" bool gui_bitmap_get_opaque(void *vbitmap) {
    if (!vbitmap) return true;
    struct bitmap *gbitmap = (struct bitmap *)vbitmap;
    return gbitmap->opaque;
}

extern "C" unsigned char * gui_bitmap_get_buffer(void *vbitmap) {
    if (!vbitmap) return nullptr;
    struct bitmap *gbitmap = (struct bitmap *)vbitmap;
    return gbitmap->image->bits();
}

extern "C" size_t gui_bitmap_get_rowstride(void *vbitmap) {
    if (!vbitmap) return 0;
    struct bitmap *gbitmap = (struct bitmap *)vbitmap;
    return gbitmap->image->bytesPerLine();
}

QImage *gui_bitmap_get_qimage(struct bitmap *vbitmap) {
    if (!vbitmap) return nullptr;
    return vbitmap->image;
}

extern "C" int gui_bitmap_get_width(void *vbitmap) {
    if (!vbitmap) return 0;
    struct bitmap *gbitmap = (struct bitmap *)vbitmap;
    return gbitmap->image->width();
}

extern "C" int gui_bitmap_get_height(void *vbitmap) {
    if (!vbitmap) return 0;
    struct bitmap *gbitmap = (struct bitmap *)vbitmap;
    return gbitmap->image->height();
}

extern "C" void gui_bitmap_modified(void *vbitmap) {
    // No-op for QImage
}

extern "C" nserror gui_bitmap_render(struct bitmap *bitmap, struct hlcache_handle *content) {
    // Stub for rendering a hlcache_handle into a bitmap (e.g. for thumbnails)
    return NSERROR_OK;
}
