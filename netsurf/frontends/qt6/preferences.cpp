#include "dialogs.h"

#include <QDialog>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QFontComboBox>

extern "C" {
#include "utils/nsoption.h"
}

namespace besra {

void showPreferences(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Preferences"));

    auto *layout = new QVBoxLayout(&dialog);
    auto *form = new QFormLayout();
    layout->addLayout(form);

    auto *homepage = new QLineEdit(&dialog);
    homepage->setText(QString::fromUtf8(nsoption_charp(homepage_url)
        ? nsoption_charp(homepage_url) : ""));
    homepage->setPlaceholderText(QStringLiteral("resource:welcome.html"));
    form->addRow(QObject::tr("Homepage:"), homepage);

    auto *font_size = new QSpinBox(&dialog);
    font_size->setRange(50, 500);
    font_size->setSuffix(QStringLiteral("%"));
    font_size->setValue(nsoption_int(font_size) / 10);
    form->addRow(QObject::tr("Default font size:"), font_size);

    auto *font_min_size = new QSpinBox(&dialog);
    font_min_size->setRange(10, 500);
    font_min_size->setSuffix(QStringLiteral("%"));
    font_min_size->setValue(nsoption_int(font_min_size) / 10);
    form->addRow(QObject::tr("Minimum font size:"), font_min_size);

    auto *font_sans = new QFontComboBox(&dialog);
    if (nsoption_charp(font_sans)) {
        font_sans->setCurrentFont(QFont(QString::fromUtf8(nsoption_charp(font_sans))));
    }
    form->addRow(QObject::tr("Sans-serif font:"), font_sans);

    auto *font_serif = new QFontComboBox(&dialog);
    if (nsoption_charp(font_serif)) {
        font_serif->setCurrentFont(QFont(QString::fromUtf8(nsoption_charp(font_serif))));
    }
    form->addRow(QObject::tr("Serif font:"), font_serif);

    auto *font_mono = new QFontComboBox(&dialog);
    if (nsoption_charp(font_mono)) {
        font_mono->setCurrentFont(QFont(QString::fromUtf8(nsoption_charp(font_mono))));
    }
    form->addRow(QObject::tr("Monospace font:"), font_mono);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QByteArray homepage_bytes = homepage->text().toUtf8();
    nsoption_set_charp(homepage_url, strdup(homepage_bytes.constData()));

    nsoption_set_int(font_size, font_size->value() * 10);
    nsoption_set_int(font_min_size, font_min_size->value() * 10);

    QByteArray sans_bytes = font_sans->currentFont().family().toUtf8();
    nsoption_set_charp(font_sans, strdup(sans_bytes.constData()));

    QByteArray serif_bytes = font_serif->currentFont().family().toUtf8();
    nsoption_set_charp(font_serif, strdup(serif_bytes.constData()));

    QByteArray mono_bytes = font_mono->currentFont().family().toUtf8();
    nsoption_set_charp(font_mono, strdup(mono_bytes.constData()));
}

} // namespace besra
