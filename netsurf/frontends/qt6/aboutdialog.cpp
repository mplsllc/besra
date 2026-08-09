#include "aboutdialog.h"

#include <QPainter>
#include <QLinearGradient>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QRandomGenerator>
#include <QFont>
#include <QtMath>

#ifndef BESRA_VERSION
#define BESRA_VERSION "0.0.0"
#endif

namespace besra {

namespace {
constexpr int kWidth = 380;
constexpr int kHeight = 300;
constexpr int kStarCount = 48;
constexpr int kFadeFrames = 15; // ~0.5s at 30fps

QColor scaled(int r, int g, int b, qreal bri)
{
    return QColor(qRound(r * bri), qRound(g * bri), qRound(b * bri));
}

} // namespace

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("About Besra"));
    setFixedSize(kWidth, kHeight);

    stars_.reserve(kStarCount);
    auto *rng = QRandomGenerator::global();
    for (int i = 0; i < kStarCount; i++) {
        Star s;
        s.x = rng->bounded(kWidth);
        s.y0 = rng->bounded(kHeight);
        s.v = 0.25 + rng->generateDouble() * 1.25;
        s.brightness = 90 + rng->bounded(165);
        stars_.push_back(s);
    }

    roll_ = {
        {tr("Patreon supporters"), 1},
        {QStringLiteral("Shlooom"), 0},
        {QStringLiteral("Kestral"), 0},
        {QStringLiteral("Mothra"), 0},
        {QString(), 2},
        {tr("Ko-Fi supporters"), 1},
        {QStringLiteral("kilgeist"), 0},
        {QStringLiteral("Turuun"), 0},
        {QStringLiteral("Rogue"), 0},
        {QString(), 2},
        {tr("Besra is a branch of MacSurf, and everyone"), 0},
        {tr("above has supported this work all along."), 0},
    };

    logo_ = QPixmap(QStringLiteral(":/res/besra-logo.png"));

    okButton_ = new QPushButton(tr("OK"), this);
    okButton_->setFixedSize(70, 26);
    okButton_->move((kWidth - okButton_->width()) / 2, 260);
    okButton_->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: rgb(46, 68, 120);"
        "  color: rgb(240, 246, 255);"
        "  border: 1px solid rgb(120, 160, 230);"
        "  border-radius: 12px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover { background-color: rgb(58, 84, 144); }"
        "QPushButton:pressed { background-color: rgb(36, 54, 96); }"));
    connect(okButton_, &QPushButton::clicked, this, &QDialog::accept);

    timer_ = new QTimer(this);
    timer_->setInterval(33); // ~30fps
    connect(timer_, &QTimer::timeout, this, [this] {
        elapsed_++;
        update();
    });
    timer_->start();
}

void AboutDialog::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    qreal bri = qMin(1.0, elapsed_ / qreal(kFadeFrames));

    drawBackdrop(p, bri);
    drawStars(p, bri);
    drawLogo(p);
    drawTitle(p, bri);
    drawSupporterRoll(p, bri);
}

void AboutDialog::drawBackdrop(QPainter &p, qreal bri)
{
    QLinearGradient grad(0, 0, 0, kHeight);
    grad.setColorAt(0.0, scaled(20, 28, 70, bri));
    grad.setColorAt(1.0, scaled(2, 4, 16, bri));
    p.fillRect(rect(), grad);
}

void AboutDialog::drawStars(QPainter &p, qreal bri)
{
    p.save();
    for (const Star &s : stars_) {
        qreal y = std::fmod(s.y0 + elapsed_ * s.v, qreal(kHeight));
        if (y < 0) y += kHeight;
        int sb = s.brightness;
        p.setPen(Qt::NoPen);
        p.setBrush(scaled(sb, sb, qMin(255, sb + 20), bri));
        qreal size = sb > 200 ? 2.0 : 1.0;
        p.drawRect(QRectF(s.x, y, size, size));
    }
    p.restore();
}

void AboutDialog::drawLogo(QPainter &p)
{
    if (logo_.isNull()) {
        return;
    }
    const int size = 52;
    QRect target((kWidth - size) / 2, 14, size, size);
    p.drawPixmap(target, logo_);
}

void AboutDialog::drawTitle(QPainter &p, qreal bri)
{
    const QString title = tr("Besra %1").arg(QStringLiteral(BESRA_VERSION));
    QFont titleFont = p.font();
    titleFont.setBold(true);
    titleFont.setPointSize(20);
    p.setFont(titleFont);

    QFontMetrics fm(titleFont);
    int tw = fm.horizontalAdvance(title);
    int tx = (kWidth - tw) / 2;
    int ty = 100;

    p.setPen(scaled(198, 216, 244, bri));
    p.drawText(tx, ty, title);

    // Sweeping metallic shine: redraw the title in bright white, clipped to
    // a narrow band that sweeps left to right and loops.
    const int period = 90; // frames per sweep
    int phase = elapsed_ % period;
    int span = tw + 80;
    int bandCenter = tx - 40 + phase * span / period;

    p.save();
    p.setClipRect(QRect(bandCenter - 10, ty - 24, 20, 32));
    p.setPen(scaled(255, 255, 255, bri));
    p.drawText(tx, ty, title);
    p.restore();

    // Accent underline
    p.fillRect(QRectF(96, 110, kWidth - 192, 2), scaled(70, 140, 255, bri));

    QFont bodyFont = p.font();
    bodyFont.setBold(false);
    bodyFont.setPointSize(9);
    p.setFont(bodyFont);
    p.setPen(scaled(120, 145, 190, bri));
    p.drawText(QRect(0, 120, kWidth, 16), Qt::AlignHCenter,
        tr("Independent web engine — NetSurf core"));
    p.setPen(scaled(150, 165, 200, bri));
    p.drawText(QRect(0, 135, kWidth, 16), Qt::AlignHCenter, tr("by mplsllc"));
}

void AboutDialog::drawSupporterRoll(QPainter &p, qreal bri)
{
    QRect viewport(24, 152, kWidth - 48, 98);
    p.save();
    p.setClipRect(viewport);

    const int lineH = 16;
    const int cx = kWidth / 2;
    const int n = static_cast<int>(roll_.size());
    const int cycle = n * lineH + viewport.height();
    int offs = (elapsed_ / 3) % cycle;

    QFont nameFont = p.font();
    nameFont.setPointSize(10);

    for (int i = 0; i < n; i++) {
        const RollLine &line = roll_[i];
        if (line.kind == 2) {
            continue;
        }
        int y = viewport.bottom() - offs + i * lineH;
        if (y < viewport.top() - lineH || y > viewport.bottom() + lineH) {
            continue;
        }
        nameFont.setBold(line.kind == 1);
        p.setFont(nameFont);
        p.setPen(line.kind == 1 ? scaled(120, 185, 255, bri)
                                 : scaled(232, 240, 252, bri));
        QFontMetrics fm(nameFont);
        int tw = fm.horizontalAdvance(line.text);
        p.drawText(cx - tw / 2, y, line.text);
    }

    p.restore();
}

} // namespace besra
