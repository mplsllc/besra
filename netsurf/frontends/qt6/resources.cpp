/*
 * Resource loading for the Qt6 frontend.
 *
 * All resources (default.css, internal.css, icons, the message catalogue)
 * are embedded into the binary via res/besra.qrc, so there is no filesystem
 * path discovery to get wrong on any platform.
 */

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QString>

extern "C" {
#include "utils/errors.h"
#include "utils/messages.h"
#include "utils/nsurl.h"
#include "netsurf/fetch.h"
}

#include "resources.h"

namespace {

QMap<QString, QByteArray> &resource_cache()
{
    static QMap<QString, QByteArray> cache;
    return cache;
}

const QByteArray *load_resource(const char *path)
{
    QString key = QString::fromUtf8(path);
    auto &cache = resource_cache();
    auto it = cache.find(key);
    if (it != cache.end()) {
        return &it.value();
    }

    QFile f(QStringLiteral(":/res/") + key);
    if (!f.open(QIODevice::ReadOnly)) {
        return nullptr;
    }

    it = cache.insert(key, f.readAll());
    return &it.value();
}

} // namespace

/* exported function documented in netsurf/fetch.h */
extern "C" nserror gui_fetch_get_resource_data(const char *path,
        const uint8_t **data, size_t *data_len)
{
    const QByteArray *bytes = load_resource(path);
    if (bytes == nullptr) {
        return NSERROR_NOT_FOUND;
    }

    *data = reinterpret_cast<const uint8_t *>(bytes->constData());
    *data_len = static_cast<size_t>(bytes->size());
    return NSERROR_OK;
}

/* exported function documented in netsurf/fetch.h */
extern "C" struct nsurl *gui_fetch_get_resource_url(const char *path)
{
    /* Every resource this frontend actually ships is served directly out of
     * gui_fetch_get_resource_data() above; anything not embedded genuinely
     * isn't available, so there's nothing to redirect to. */
    (void)path;
    return nullptr;
}

/* exported function documented in netsurf/fetch.h */
extern "C" const char *gui_fetch_filetype(const char *unix_path)
{
    QString suffix = QFileInfo(QString::fromUtf8(unix_path)).suffix().toLower();

    static const QMap<QString, const char *> table = {
        {QStringLiteral("html"), "text/html"},
        {QStringLiteral("htm"), "text/html"},
        {QStringLiteral("css"), "text/css"},
        {QStringLiteral("js"), "text/javascript"},
        {QStringLiteral("mjs"), "text/javascript"},
        {QStringLiteral("json"), "application/json"},
        {QStringLiteral("xml"), "text/xml"},
        {QStringLiteral("svg"), "image/svg+xml"},
        {QStringLiteral("png"), "image/png"},
        {QStringLiteral("jpg"), "image/jpeg"},
        {QStringLiteral("jpeg"), "image/jpeg"},
        {QStringLiteral("jxl"), "image/jxl"},
        {QStringLiteral("gif"), "image/gif"},
        {QStringLiteral("webp"), "image/webp"},
        {QStringLiteral("bmp"), "image/bmp"},
        {QStringLiteral("ico"), "image/x-icon"},
        {QStringLiteral("txt"), "text/plain"},
        {QStringLiteral("pdf"), "application/pdf"},
    };

    auto it = table.constFind(suffix);
    if (it != table.constEnd()) {
        return it.value();
    }
    return "text/plain";
}

namespace besra {

/**
 * Load the embedded message catalogue into the core's Messages hash.
 * Call once at startup, before any UI that calls messages_get().
 */
void load_messages()
{
    const QByteArray *bytes = load_resource("Messages");
    if (bytes == nullptr) {
        return;
    }
    messages_add_from_inline(
        reinterpret_cast<const uint8_t *>(bytes->constData()),
        static_cast<size_t>(bytes->size()));
}

} // namespace besra
