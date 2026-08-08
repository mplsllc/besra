#include <QFont>
#include <QString>
#include <QTextLayout>
#include <map>
#include <list>
#include <memory>

#include "qt_font.h"

extern "C" {
#include "utils/errors.h"
#include "utils/log.h"
#include "netsurf/layout.h"
#include "netsurf/plot_style.h"
}

struct FontCacheKey {
    QString family;
    plot_style_fixed size;
    int weight;
    int flags;

    bool operator<(const FontCacheKey& o) const {
        if (family != o.family) return family < o.family;
        if (size != o.size) return size < o.size;
        if (weight != o.weight) return weight < o.weight;
        return flags < o.flags;
    }
};

struct LayoutCacheKey {
    FontCacheKey font_key;
    std::string text;

    bool operator<(const LayoutCacheKey& o) const {
        if (font_key < o.font_key) return true;
        if (o.font_key < font_key) return false;
        return text < o.text;
    }
};

// Caches are allocated on the heap to avoid static destruction order crashes.
// Qt objects inside these caches would crash if destroyed after QApplication exits.
static std::map<FontCacheKey, QFont>* font_cache = nullptr;
static std::list<LayoutCacheKey>* lru_list = nullptr;
static std::map<LayoutCacheKey, std::pair<std::unique_ptr<QTextLayout>, std::list<LayoutCacheKey>::iterator>>* layout_cache = nullptr;

static QFont get_cached_font(const plot_font_style_t *fstyle) {
    if (!font_cache) font_cache = new std::map<FontCacheKey, QFont>();
    
    QString family = qt_font_family(fstyle);
    FontCacheKey key = { family, fstyle->size, fstyle->weight, fstyle->flags };
    auto it = font_cache->find(key);
    if (it != font_cache->end()) {
        return it->second;
    }
    QFont font = qt_font(fstyle);
    (*font_cache)[key] = font;
    return font;
}

// WARNING: Returns a raw pointer into the cache. The next call to this function
// could evict the layout, leaving you with a dangling pointer (use-after-free).
// Do not hold this pointer across any subsequent calls into the layout system.
static QTextLayout* get_cached_layout(const plot_font_style_t *fstyle, const char *string, size_t length) {
    if (!layout_cache) {
        layout_cache = new std::map<LayoutCacheKey, std::pair<std::unique_ptr<QTextLayout>, std::list<LayoutCacheKey>::iterator>>();
        lru_list = new std::list<LayoutCacheKey>();
    }

    QString family = qt_font_family(fstyle);
    FontCacheKey key = { family, fstyle->size, fstyle->weight, fstyle->flags };
    LayoutCacheKey lck = { key, std::string(string, length) };

    auto it = layout_cache->find(lck);
    if (it != layout_cache->end()) {
        lru_list->erase(it->second.second);
        lru_list->push_front(lck);
        it->second.second = lru_list->begin();
        return it->second.first.get();
    }

    QString qs = QString::fromUtf8(string, length);
    auto layout = std::make_unique<QTextLayout>(qs, get_cached_font(fstyle));
    layout->beginLayout();
    QTextLine line = layout->createLine();
    if (line.isValid()) {
        line.setLineWidth(1e9);
    }
    layout->endLayout();

    lru_list->push_front(lck);
    QTextLayout* raw_ptr = layout.get();
    (*layout_cache)[lck] = std::make_pair(std::move(layout), lru_list->begin());

    if (layout_cache->size() > 512) {
        auto last = lru_list->back();
        layout_cache->erase(last);
        lru_list->pop_back();
    }

    return raw_ptr;
}

static size_t utf16_to_utf8_offset(const QString& qs, int cursor) {
    size_t len = 0;
    for (int i = 0; i < cursor && i < qs.length(); ++i) {
        uint c = qs.at(i).unicode();
        if (c < 0x80) len += 1;
        else if (c < 0x800) len += 2;
        else if (QChar::isHighSurrogate(c)) {
            len += 4;
            i++; // skip low surrogate
        }
        else len += 3;
    }
    return len;
}

extern "C" nserror gui_layout_width(const struct plot_font_style *fstyle, const char *string, size_t length, int *width) {
    if (length == 0) {
        *width = 0;
        return NSERROR_OK;
    }
    
    QTextLayout* layout = get_cached_layout(fstyle, string, length);
    
    if (layout->lineCount() > 0) {
        QTextLine line = layout->lineAt(0);
        *width = qRound(line.naturalTextWidth());
    } else {
        *width = 0;
    }
    
    return NSERROR_OK;
}

static nserror layout_position_internal(const plot_font_style_t *fstyle, const char *string, size_t length, int x, size_t *char_offset, int *actual_x) {
    QTextLayout* layout = get_cached_layout(fstyle, string, length);
    
    if (layout->lineCount() == 0) {
        *char_offset = 0;
        *actual_x = 0;
        return NSERROR_OK;
    }
    
    QTextLine line = layout->lineAt(0);
    int cursor = line.xToCursor(x, QTextLine::CursorOnCharacter);
    qreal ax = line.cursorToX(cursor);
    
    *char_offset = utf16_to_utf8_offset(layout->text(), cursor);
    *actual_x = qRound(ax);
    
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
    
    if (split_len >= length) {
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
    nserror err = gui_layout_width(fstyle, string, str_len, actual_x);
    
    return err;
}
