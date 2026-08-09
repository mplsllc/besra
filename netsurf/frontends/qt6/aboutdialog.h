#ifndef BESRA_QT6_ABOUTDIALOG_H
#define BESRA_QT6_ABOUTDIALOG_H

#include <QDialog>
#include <QPixmap>
#include <vector>

class QTimer;
class QPushButton;

namespace besra {

/**
 * Animated "space glass" about box, styled after MacSurf's classic-Mac
 * About window: a drifting starfield over a deep-blue-to-black gradient, a
 * sweeping metallic shine on the title, and an auto-scrolling supporter
 * credits roll. Ported from QuickDraw to QPainter rather than translated
 * line-for-line -- see macos9_chrome_extras.c's about_draw() in the MacSurf
 * tree for the original.
 */
class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Star {
        qreal x;
        qreal y0;
        qreal v;      // drift speed, px/tick
        int brightness; // 0..255 base brightness
    };

    struct RollLine {
        QString text;
        int kind; // 0 = name, 1 = section header, 2 = blank spacer
    };

    void tick();
    void drawBackdrop(QPainter &p, qreal bri);
    void drawStars(QPainter &p, qreal bri);
    void drawLogo(QPainter &p);
    void drawTitle(QPainter &p, qreal bri);
    void drawSupporterRoll(QPainter &p, qreal bri);

    QTimer *timer_;
    QPushButton *okButton_;
    int elapsed_ = 0;
    std::vector<Star> stars_;
    std::vector<RollLine> roll_;
    QPixmap logo_;
};

} // namespace besra

#endif
