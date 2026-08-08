#include "dialogs.h"

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QPainter>

extern "C" {
#include "utils/errors.h"
#include "netsurf/content.h"
#include "netsurf/browser_window.h"
#include "netsurf/plotters.h"
#include "desktop/search.h"
}

extern const struct plotter_table nsqt_plotters;
extern QPainter *qt_current_painter;

namespace besra {

void showAbout(QWidget *parent)
{
    QMessageBox::about(parent, QObject::tr("About Besra"),
        QObject::tr("<h3>Besra</h3>"
                     "<p>A lightweight, independent web engine for the readable modern web.</p>"
                     "<p>Built on the NetSurf engine core.</p>"));
}

void showViewSource(QWidget *parent, struct browser_window *bw)
{
    struct hlcache_handle *content = browser_window_get_content(bw);
    if (content == nullptr) {
        return;
    }

    size_t size = 0;
    const uint8_t *data = content_get_source_data(content, &size);
    if (data == nullptr) {
        return;
    }

    auto *dialog = new QDialog(parent);
    dialog->setWindowTitle(QObject::tr("Page Source"));
    dialog->resize(800, 600);
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QVBoxLayout(dialog);
    auto *text = new QPlainTextEdit(dialog);
    text->setReadOnly(true);
    text->setLineWrapMode(QPlainTextEdit::NoWrap);
    QFont mono(QStringLiteral("Monospace"));
    mono.setStyleHint(QFont::TypeWriter);
    text->setFont(mono);
    text->setPlainText(QString::fromUtf8(reinterpret_cast<const char *>(data),
        static_cast<int>(size)));
    layout->addWidget(text);

    dialog->show();
}

void showFindInPage(QWidget *parent, struct browser_window *bw)
{
    auto *dialog = new QDialog(parent);
    dialog->setWindowTitle(QObject::tr("Find in Page"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    auto *layout = new QHBoxLayout(dialog);
    auto *field = new QLineEdit(dialog);
    field->setPlaceholderText(QObject::tr("Find..."));
    auto *case_box = new QCheckBox(QObject::tr("Case sensitive"), dialog);
    auto *prev_button = new QPushButton(QObject::tr("Previous"), dialog);
    auto *next_button = new QPushButton(QObject::tr("Next"), dialog);

    layout->addWidget(field);
    layout->addWidget(case_box);
    layout->addWidget(prev_button);
    layout->addWidget(next_button);

    auto do_search = [=](bool forwards) {
        unsigned flags = forwards ? SEARCH_FLAG_FORWARDS : SEARCH_FLAG_BACKWARDS;
        if (case_box->isChecked()) {
            flags |= SEARCH_FLAG_CASE_SENSITIVE;
        }
        browser_window_search(bw, dialog, static_cast<search_flags_t>(flags),
            field->text().toUtf8().constData());
    };

    QObject::connect(next_button, &QPushButton::clicked, dialog, [=] { do_search(true); });
    QObject::connect(prev_button, &QPushButton::clicked, dialog, [=] { do_search(false); });
    QObject::connect(field, &QLineEdit::returnPressed, dialog, [=] { do_search(true); });

    dialog->show();
    field->setFocus();
}

void showPrint(QWidget *parent, struct browser_window *bw)
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog print_dialog(&printer, parent);
    if (print_dialog.exec() != QDialog::Accepted) {
        return;
    }

    QPainter painter(&printer);
    qt_current_painter = &painter;

    QRect page = printer.pageRect(QPrinter::DevicePixel).toRect();

    struct rect clip;
    clip.x0 = 0;
    clip.y0 = 0;
    clip.x1 = page.width();
    clip.y1 = page.height();

    struct redraw_context ctx;
    ctx.interactive = false;
    ctx.background_images = true;
    ctx.plot = &nsqt_plotters;

    browser_window_redraw(bw, 0, 0, &clip, &ctx);

    qt_current_painter = nullptr;
}

/* showHistory/showBookmarks/showCookies/addBookmark: implemented in
 * panels.cpp (they share the CoreWindowWidget machinery).
 * showPreferences: implemented in preferences.cpp. */

/* showDownloads: implemented in downloads.cpp. */

} // namespace besra
