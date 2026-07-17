#ifndef QT_FONT_H
#define QT_FONT_H

#include <QFont>

extern "C" {
#include "netsurf/plot_style.h"
}

static inline QFont qt_font(const plot_font_style_t *fstyle) {
    QFont font;
    switch (fstyle->family) {
    case PLOT_FONT_FAMILY_SERIF:
        font.setStyleHint(QFont::Serif);
        break;
    case PLOT_FONT_FAMILY_MONOSPACE:
        font.setStyleHint(QFont::Monospace);
        break;
    case PLOT_FONT_FAMILY_CURSIVE:
        font.setStyleHint(QFont::Cursive);
        break;
    case PLOT_FONT_FAMILY_FANTASY:
        font.setStyleHint(QFont::Fantasy);
        break;
    case PLOT_FONT_FAMILY_SANS_SERIF:
    default:
        font.setStyleHint(QFont::SansSerif);
        break;
    }

    qreal ptSize = (qreal)fstyle->size / PLOT_STYLE_SCALE;
    font.setPointSizeF(ptSize);
    font.setWeight(QFont::Weight(fstyle->weight));

    if (fstyle->flags & FONTF_ITALIC) {
        font.setStyle(QFont::StyleItalic);
    } else if (fstyle->flags & FONTF_OBLIQUE) {
        font.setStyle(QFont::StyleOblique);
    }

    if (fstyle->flags & FONTF_SMALLCAPS) {
        font.setCapitalization(QFont::SmallCaps);
    }

    return font;
}

#endif
