#include "dialogs.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QList>

extern "C" {
#include "utils/errors.h"
#include "utils/nsurl.h"
#include "netsurf/download.h"
#include "desktop/download.h"
}

struct gui_download_window {
    QFile *file;
    QListWidgetItem *item;
    QString filename;
    unsigned long long received = 0;
    unsigned long long total = 0;
};

namespace {

QListWidget *&downloadsList()
{
    static QListWidget *list = nullptr;
    return list;
}

QDialog *&downloadsDialog()
{
    static QDialog *dialog = nullptr;
    return dialog;
}

void ensureDownloadsDialog(QWidget *parent)
{
    if (downloadsDialog() != nullptr) {
        return;
    }
    auto *dialog = new QDialog(parent);
    dialog->setWindowTitle(QObject::tr("Downloads"));
    dialog->resize(500, 300);

    auto *layout = new QVBoxLayout(dialog);
    auto *list = new QListWidget(dialog);
    layout->addWidget(list);

    downloadsDialog() = dialog;
    downloadsList() = list;
}

void refreshItem(struct gui_download_window *dw, const QString &status)
{
    if (dw->item == nullptr) {
        return;
    }
    QString label = QStringLiteral("%1 - %2").arg(dw->filename, status);
    if (dw->total > 0) {
        label += QStringLiteral(" (%1 / %2 bytes)").arg(dw->received).arg(dw->total);
    } else if (dw->received > 0) {
        label += QStringLiteral(" (%1 bytes)").arg(dw->received);
    }
    dw->item->setText(label);
}

} // namespace

extern "C" struct gui_download_window *gui_download_create(struct download_context *ctx,
        struct gui_window *parent)
{
    (void)parent;

    const char *suggested = download_context_get_filename(ctx);
    QString default_name = suggested ? QString::fromUtf8(suggested) : QStringLiteral("download");
    QString default_dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString default_path = default_dir.isEmpty() ? default_name
        : QDir(default_dir).filePath(default_name);

    QString path = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save File"), default_path);
    if (path.isEmpty()) {
        return nullptr;
    }

    auto *dw = new gui_download_window();
    dw->file = new QFile(path);
    if (!dw->file->open(QIODevice::WriteOnly)) {
        delete dw->file;
        delete dw;
        return nullptr;
    }
    dw->filename = QFileInfo(path).fileName();
    dw->total = download_context_get_total_length(ctx);

    ensureDownloadsDialog(nullptr);
    dw->item = new QListWidgetItem(downloadsList());
    refreshItem(dw, QObject::tr("Starting"));
    downloadsDialog()->show();

    return dw;
}

extern "C" nserror gui_download_data(struct gui_download_window *dw, const char *data,
        unsigned int size)
{
    dw->file->write(data, size);
    dw->received += size;
    refreshItem(dw, QObject::tr("Downloading"));
    return NSERROR_OK;
}

extern "C" void gui_download_error(struct gui_download_window *dw, const char *error_msg)
{
    refreshItem(dw, QObject::tr("Failed: %1").arg(QString::fromUtf8(error_msg ? error_msg : "")));
    dw->file->close();
    delete dw->file;
    delete dw;
}

extern "C" void gui_download_done(struct gui_download_window *dw)
{
    refreshItem(dw, QObject::tr("Done"));
    dw->file->close();
    delete dw->file;
    delete dw;
}

namespace besra {

void showDownloads(QWidget *parent)
{
    ensureDownloadsDialog(parent);
    downloadsDialog()->show();
    downloadsDialog()->raise();
    downloadsDialog()->activateWindow();
}

} // namespace besra
