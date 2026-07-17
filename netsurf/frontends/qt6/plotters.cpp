#include <QPainter>
#include <QImage>
#include <QColor>
#include <QRect>
#include <QPen>
#include <QBrush>
#include <QPainterPath>
#include <QString>

#include "qt_font.h"

extern "C" {
#include "utils/errors.h"
#include "utils/log.h"
#include "netsurf/plotters.h"
#include "netsurf/bitmap.h"
}

QPainter *qt_current_painter = nullptr;

static QColor qt_color(colour c) {
    return QColor(c & 0xff, (c >> 8) & 0xff, (c >> 16) & 0xff, 255);
}

static void apply_style(const plot_style_t *pstyle, QPen &pen, QBrush &brush) {
    if (pstyle->fill_type != PLOT_OP_TYPE_NONE) {
        brush.setColor(qt_color(pstyle->fill_colour));
        brush.setStyle(Qt::SolidPattern);
    } else {
        brush.setStyle(Qt::NoBrush);
    }
    
    if (pstyle->stroke_type != PLOT_OP_TYPE_NONE) {
        pen.setColor(qt_color(pstyle->stroke_colour));
        qreal width = (qreal)pstyle->stroke_width / PLOT_STYLE_SCALE;
        if (width == 0) width = 1;
        pen.setWidthF(width);
        
        // Qt handles joints and caps, default is usually fine
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setCapStyle(Qt::SquareCap);

        switch (pstyle->stroke_type) {
            case PLOT_OP_TYPE_SOLID: pen.setStyle(Qt::SolidLine); break;
            case PLOT_OP_TYPE_DOT: pen.setStyle(Qt::DotLine); break;
            case PLOT_OP_TYPE_DASH: pen.setStyle(Qt::DashLine); break;
            default: pen.setStyle(Qt::SolidLine); break;
        }
    } else {
        pen.setStyle(Qt::NoPen);
    }
}

static nserror qt_clip(const struct redraw_context *ctx, const struct rect *clip) {
    if (qt_current_painter) {
        qt_current_painter->setClipRect(QRect(clip->x0, clip->y0, clip->x1 - clip->x0, clip->y1 - clip->y0), Qt::ReplaceClip);
    }
    return NSERROR_OK;
}

static nserror qt_arc(const struct redraw_context *ctx, const plot_style_t *pstyle, int x, int y, int radius, int angle1, int angle2) {
    if (!qt_current_painter) return NSERROR_OK;
    QPen pen;
    QBrush brush;
    apply_style(pstyle, pen, brush);
    qt_current_painter->setPen(pen);
    qt_current_painter->setBrush(brush);
    
    int spanAngle = angle2 - angle1;
    if (spanAngle < 0) spanAngle += 360;
    
    if (brush.style() != Qt::NoBrush) {
        qt_current_painter->drawPie(x - radius, y - radius, radius * 2, radius * 2, angle1 * 16, spanAngle * 16);
    } else {
        qt_current_painter->drawArc(x - radius, y - radius, radius * 2, radius * 2, angle1 * 16, spanAngle * 16);
    }
    return NSERROR_OK;
}

static nserror qt_disc(const struct redraw_context *ctx, const plot_style_t *pstyle, int x, int y, int radius) {
    if (!qt_current_painter) return NSERROR_OK;
    QPen pen;
    QBrush brush;
    apply_style(pstyle, pen, brush);
    qt_current_painter->setPen(pen);
    qt_current_painter->setBrush(brush);
    
    qt_current_painter->drawEllipse(QPoint(x, y), radius, radius);
    return NSERROR_OK;
}

static nserror qt_line(const struct redraw_context *ctx, const plot_style_t *pstyle, const struct rect *line) {
    if (!qt_current_painter) return NSERROR_OK;
    QPen pen;
    QBrush brush;
    apply_style(pstyle, pen, brush);
    qt_current_painter->setPen(pen);
    qt_current_painter->setBrush(brush);
    
    qt_current_painter->drawLine(line->x0, line->y0, line->x1, line->y1);
    return NSERROR_OK;
}

static nserror qt_rectangle(const struct redraw_context *ctx, const plot_style_t *pstyle, const struct rect *rect) {
    if (!qt_current_painter) return NSERROR_OK;
    QPen pen;
    QBrush brush;
    apply_style(pstyle, pen, brush);
    qt_current_painter->setPen(pen);
    qt_current_painter->setBrush(brush);
    
    qt_current_painter->drawRect(QRect(rect->x0, rect->y0, rect->x1 - rect->x0, rect->y1 - rect->y0));
    return NSERROR_OK;
}

static nserror qt_polygon(const struct redraw_context *ctx, const plot_style_t *pstyle, const int *p, unsigned int n) {
    if (!qt_current_painter) return NSERROR_OK;
    QPen pen;
    QBrush brush;
    apply_style(pstyle, pen, brush);
    qt_current_painter->setPen(pen);
    qt_current_painter->setBrush(brush);
    
    QPolygon poly;
    for (unsigned int i = 0; i < n; i++) {
        poly << QPoint(p[i * 2], p[i * 2 + 1]);
    }
    
    qt_current_painter->drawPolygon(poly, Qt::WindingFill);
    return NSERROR_OK;
}

static nserror qt_path(const struct redraw_context *ctx, const plot_style_t *pstyle, const float *p, unsigned int n, const float transform[6]) {
    if (!qt_current_painter) return NSERROR_OK;
    QPen pen;
    QBrush brush;
    apply_style(pstyle, pen, brush);
    qt_current_painter->setPen(pen);
    qt_current_painter->setBrush(brush);
    
    QPainterPath path;
    unsigned int i = 0;
    while (i < n) {
        int cmd = p[i++];
        switch (cmd) {
            case PLOTTER_PATH_MOVE:
                path.moveTo(p[i], p[i+1]);
                i += 2;
                break;
            case PLOTTER_PATH_LINE:
                path.lineTo(p[i], p[i+1]);
                i += 2;
                break;
            case PLOTTER_PATH_BEZIER:
                path.cubicTo(p[i], p[i+1], p[i+2], p[i+3], p[i+4], p[i+5]);
                i += 6;
                break;
            case PLOTTER_PATH_CLOSE:
                path.closeSubpath();
                break;
        }
    }
    
    qt_current_painter->save();
    // QTransform uses row-major like Cairo, but expects m11, m12, m21, m22, dx, dy
    // transform[] array mapping:
    // [0]=m11, [1]=m12, [2]=m21, [3]=m22, [4]=dx, [5]=dy
    QTransform qtTransform(transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
    qt_current_painter->setTransform(qtTransform, true);
    
    qt_current_painter->drawPath(path);
    qt_current_painter->restore();
    return NSERROR_OK;
}

static nserror qt_bitmap(const struct redraw_context *ctx, struct bitmap *bitmap, int x, int y, int width, int height, colour bg, bitmap_flags_t flags) {
    if (!qt_current_painter || !bitmap) return NSERROR_OK;
    
    // We cast struct bitmap to QImage*. This requires our bitmap.cpp implementation to return QImage* from create.
    QImage *img = (QImage *)bitmap;
    
    qt_current_painter->save();
    
    if (flags & (BITMAPF_REPEAT_X | BITMAPF_REPEAT_Y)) {
        // Handle tiling if necessary
        // For now, draw scaled
        qt_current_painter->drawImage(QRect(x, y, width, height), *img);
    } else {
        qt_current_painter->drawImage(QRect(x, y, width, height), *img);
    }
    
    qt_current_painter->restore();
    return NSERROR_OK;
}

static nserror qt_text(const struct redraw_context *ctx, const plot_font_style_t *fstyle, int x, int y, const char *text, size_t length) {
    if (!qt_current_painter) return NSERROR_OK;
    
    QFont font = qt_font(fstyle);
    qt_current_painter->setFont(font);
    qt_current_painter->setPen(qt_color(fstyle->foreground));
    
    QString qs = QString::fromUtf8(text, length);
    // x, y is the baseline for QPainter::drawText
    qt_current_painter->drawText(x, y, qs);
    return NSERROR_OK;
}

extern "C" const struct plotter_table nsqt_plotters = {
    qt_clip,
    qt_arc,
    qt_disc,
    qt_line,
    qt_rectangle,
    qt_polygon,
    qt_path,
    qt_bitmap,
    qt_text,
    nullptr, // group_start
    nullptr, // group_end
    nullptr, // flush
    false    // option_knockout
};
