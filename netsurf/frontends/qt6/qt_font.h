#ifndef QT_FONT_H
#define QT_FONT_H

#include <QFont>
#include <QString>

extern "C" {
#include "netsurf/plot_style.h"
#include "utils/nsoption.h"
}

static inline QString qt_font_family(const plot_font_style_t *fstyle) {
    switch (fstyle->family) {
    case PLOT_FONT_FAMILY_SERIF:
        return QString::fromUtf8(nsoption_charp(font_serif));
    case PLOT_FONT_FAMILY_MONOSPACE:
        return QString::fromUtf8(nsoption_charp(font_mono));
    case PLOT_FONT_FAMILY_CURSIVE:
        return QString::fromUtf8(nsoption_charp(font_cursive));
    case PLOT_FONT_FAMILY_FANTASY:
        return QString::fromUtf8(nsoption_charp(font_fantasy));
    case PLOT_FONT_FAMILY_SANS_SERIF:
    default:
        return QString::fromUtf8(nsoption_charp(font_sans));
    }
}

static inline QFont qt_font(const plot_font_style_t *fstyle) {
    QFont font;
    font.setFamily(qt_font_family(fstyle));
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
