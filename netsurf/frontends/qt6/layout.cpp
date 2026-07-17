#include <QFont>
#include <QFontMetrics>
#include <QString>

extern "C" {
#include "utils/errors.h"
#include "utils/log.h"
#include "netsurf/layout.h"
#include "netsurf/plot_style.h"
}

static QFont qt_font(const plot_font_style_t *fstyle) {
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

extern "C" nserror gui_layout_width(const struct plot_font_style *fstyle, const char *string, size_t length, int *width) {
    if (length == 0) {
        *width = 0;
        return NSERROR_OK;
    }
    
    QFont font = qt_font(fstyle);
    QFontMetrics fm(font);
    
    QString qs = QString::fromUtf8(string, length);
    *width = fm.horizontalAdvance(qs);
    
    return NSERROR_OK;
}

static nserror layout_position_internal(const plot_font_style_t *fstyle, const char *string, size_t length, int x, size_t *char_offset, int *actual_x) {
    QFont font = qt_font(fstyle);
    QFontMetrics fm(font);
    QString qs = QString::fromUtf8(string, length);
    
    int low = 0;
    int high = qs.length();
    int best_offset = 0;
    int best_x = 0;
    
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int w = fm.horizontalAdvance(qs.left(mid));
        
        if (w <= x) {
            best_offset = mid;
            best_x = w;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    
    *char_offset = qs.left(best_offset).toUtf8().length();
    *actual_x = best_x;
    
    return NSERROR_OK;
}

extern "C" nserror gui_layout_position(const struct plot_font_style *fstyle, const char *string, size_t length, int x, size_t *char_offset, int *actual_x) {
    return layout_position_internal(fstyle, string, length, x, char_offset, actual_x);
}

extern "C" nserror gui_layout_split(const struct plot_font_style *fstyle, const char *string, size_t length, int x, size_t *char_offset, int *actual_x) {
    size_t split_len;
    int split_x;
    
    nserror res = layout_position_internal(fstyle, string, length, x, &split_len, &split_x);
    if (res != NSERROR_OK) return res;
    
    if (split_len < 1 || split_len >= length) {
        *char_offset = length;
        *actual_x = split_x;
        return NSERROR_OK;
    }
    
    if (string[split_len] == ' ') {
        *char_offset = split_len;
        *actual_x = split_x;
        return NSERROR_OK;
    }
    
    size_t str_len = split_len;
    while (str_len > 0 && string[str_len] != ' ') {
        str_len--;
    }
    
    if (str_len == 0) {
        str_len = split_len;
        while (str_len < length && string[str_len] != ' ') {
            str_len++;
        }
    }
    
    if (str_len < length && string[str_len] == ' ') {
        str_len++;
    }
    
    *char_offset = str_len;
    
    QFont font = qt_font(fstyle);
    QFontMetrics fm(font);
    QString qs = QString::fromUtf8(string, str_len);
    *actual_x = fm.horizontalAdvance(qs);
    
    return NSERROR_OK;
}
