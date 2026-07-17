#include <QFont>
#include <QString>
#include <QTextLayout>

#include "qt_font.h"

extern "C" {
#include "utils/errors.h"
#include "utils/log.h"
#include "netsurf/layout.h"
#include "netsurf/plot_style.h"
}

extern "C" nserror gui_layout_width(const struct plot_font_style *fstyle, const char *string, size_t length, int *width) {
    if (length == 0) {
        *width = 0;
        return NSERROR_OK;
    }
    
    QFont font = qt_font(fstyle);
    QString qs = QString::fromUtf8(string, length);
    
    QTextLayout layout(qs, font);
    layout.beginLayout();
    QTextLine line = layout.createLine();
    if (line.isValid()) {
        line.setLineWidth(1e9);
    }
    layout.endLayout();
    
    if (line.isValid()) {
        *width = line.naturalTextWidth();
    } else {
        *width = 0;
    }
    
    return NSERROR_OK;
}

static nserror layout_position_internal(const plot_font_style_t *fstyle, const char *string, size_t length, int x, size_t *char_offset, int *actual_x) {
    QFont font = qt_font(fstyle);
    QString qs = QString::fromUtf8(string, length);
    
    QTextLayout layout(qs, font);
    layout.beginLayout();
    QTextLine line = layout.createLine();
    if (line.isValid()) {
        line.setLineWidth(1e9);
    }
    layout.endLayout();
    
    if (!line.isValid()) {
        *char_offset = 0;
        *actual_x = 0;
        return NSERROR_OK;
    }
    
    int cursor = line.xToCursor(x, QTextLine::CursorOnCharacter);
    qreal ax = line.cursorToX(cursor);
    
    *char_offset = qs.left(cursor).toUtf8().length();
    *actual_x = ax;
    
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
    
    // We must recalculate actual_x at the new offset to give an honest width.
    // It's cheaper to just measure it since we know exactly where we split.
    gui_layout_width(fstyle, string, str_len, actual_x);
    
    return NSERROR_OK;
}
