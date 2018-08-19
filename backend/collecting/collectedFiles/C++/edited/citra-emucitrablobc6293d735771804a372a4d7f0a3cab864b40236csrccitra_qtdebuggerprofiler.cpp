
#include <QAction>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QString>
#include "citra_qt/debugger/profiler.h"
#include "citra_qt/util/util.h"
#include "common/common_types.h"
#include "common/microprofile.h"

#if MICROPROFILE_ENABLED
#define MICROPROFILEUI_IMPL 1
#include "common/microprofileui.h"

class MicroProfileWidget : public QWidget {
public:
    MicroProfileWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* ev) override;
    void showEvent(QShowEvent* ev) override;
    void hideEvent(QHideEvent* ev) override;

    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;
    void wheelEvent(QWheelEvent* ev) override;

    void keyPressEvent(QKeyEvent* ev) override;
    void keyReleaseEvent(QKeyEvent* ev) override;

private:
            QTimer update_timer;
        qreal x_scale = 1.0, y_scale = 1.0;
};

#endif

MicroProfileDialog::MicroProfileDialog(QWidget* parent) : QWidget(parent, Qt::Dialog) {
    setObjectName("MicroProfile");
    setWindowTitle(tr("MicroProfile"));
    resize(1000, 600);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMaximizeButtonHint);

#if MICROPROFILE_ENABLED

    MicroProfileWidget* widget = new MicroProfileWidget(this);

    QLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(widget);
    setLayout(layout);

            setFocusProxy(widget);
    widget->setFocusPolicy(Qt::StrongFocus);
    widget->setFocus();
#endif
}

QAction* MicroProfileDialog::toggleViewAction() {
    if (toggle_view_action == nullptr) {
        toggle_view_action = new QAction(windowTitle(), this);
        toggle_view_action->setCheckable(true);
        toggle_view_action->setChecked(isVisible());
        connect(toggle_view_action, &QAction::toggled, this, &MicroProfileDialog::setVisible);
    }

    return toggle_view_action;
}

void MicroProfileDialog::showEvent(QShowEvent* ev) {
    if (toggle_view_action) {
        toggle_view_action->setChecked(isVisible());
    }
    QWidget::showEvent(ev);
}

void MicroProfileDialog::hideEvent(QHideEvent* ev) {
    if (toggle_view_action) {
        toggle_view_action->setChecked(isVisible());
    }
    QWidget::hideEvent(ev);
}

#if MICROPROFILE_ENABLED

static QPainter* mp_painter = nullptr;

MicroProfileWidget::MicroProfileWidget(QWidget* parent) : QWidget(parent) {
        setMouseTracking(true);

    MicroProfileSetDisplayMode(1);     MicroProfileInitUI();

    connect(&update_timer, &QTimer::timeout, this,
            static_cast<void (MicroProfileWidget::*)()>(&MicroProfileWidget::update));
}

void MicroProfileWidget::paintEvent(QPaintEvent* ev) {
    QPainter painter(this);

        x_scale = qreal(painter.device()->logicalDpiX()) / 96.0;
    y_scale = qreal(painter.device()->logicalDpiY()) / 96.0;
    painter.scale(x_scale, y_scale);

    painter.setBackground(Qt::black);
    painter.eraseRect(rect());

    QFont font = GetMonospaceFont();
    font.setPixelSize(MICROPROFILE_TEXT_HEIGHT);
    painter.setFont(font);

    mp_painter = &painter;
    MicroProfileDraw(rect().width() / x_scale, rect().height() / y_scale);
    mp_painter = nullptr;
}

void MicroProfileWidget::showEvent(QShowEvent* ev) {
    update_timer.start(15);     QWidget::showEvent(ev);
}

void MicroProfileWidget::hideEvent(QHideEvent* ev) {
    update_timer.stop();
    QWidget::hideEvent(ev);
}

void MicroProfileWidget::mouseMoveEvent(QMouseEvent* ev) {
    MicroProfileMousePosition(ev->x() / x_scale, ev->y() / y_scale, 0);
    ev->accept();
}

void MicroProfileWidget::mousePressEvent(QMouseEvent* ev) {
    MicroProfileMousePosition(ev->x() / x_scale, ev->y() / y_scale, 0);
    MicroProfileMouseButton(ev->buttons() & Qt::LeftButton, ev->buttons() & Qt::RightButton);
    ev->accept();
}

void MicroProfileWidget::mouseReleaseEvent(QMouseEvent* ev) {
    MicroProfileMousePosition(ev->x() / x_scale, ev->y() / y_scale, 0);
    MicroProfileMouseButton(ev->buttons() & Qt::LeftButton, ev->buttons() & Qt::RightButton);
    ev->accept();
}

void MicroProfileWidget::wheelEvent(QWheelEvent* ev) {
    MicroProfileMousePosition(ev->x() / x_scale, ev->y() / y_scale, ev->delta() / 120);
    ev->accept();
}

void MicroProfileWidget::keyPressEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Control) {
                MicroProfileModKey(1);
    }
    QWidget::keyPressEvent(ev);
}

void MicroProfileWidget::keyReleaseEvent(QKeyEvent* ev) {
    if (ev->key() == Qt::Key_Control) {
        MicroProfileModKey(0);
    }
    QWidget::keyReleaseEvent(ev);
}


void MicroProfileDrawText(int x, int y, u32 hex_color, const char* text, u32 text_length) {
        mp_painter->setPen(QColor::fromRgb(hex_color));

                for (u32 i = 0; i < text_length; ++i) {
                        mp_painter->drawText(x, y + MICROPROFILE_TEXT_HEIGHT - 2, QChar(text[i]));
        x += MICROPROFILE_TEXT_WIDTH + 1;
    }
}

void MicroProfileDrawBox(int left, int top, int right, int bottom, u32 hex_color,
                         MicroProfileBoxType type) {
    QColor color = QColor::fromRgba(hex_color);
    QBrush brush = color;
    if (type == MicroProfileBoxTypeBar) {
        QLinearGradient gradient(left, top, left, bottom);
        gradient.setColorAt(0.f, color.lighter(125));
        gradient.setColorAt(1.f, color.darker(125));
        brush = gradient;
    }
    mp_painter->fillRect(left, top, right - left, bottom - top, brush);
}

void MicroProfileDrawLine2D(u32 vertices_length, float* vertices, u32 hex_color) {
            static std::vector<QPointF> point_buf;

    for (u32 i = 0; i < vertices_length; ++i) {
        point_buf.emplace_back(vertices[i * 2 + 0], vertices[i * 2 + 1]);
    }

        mp_painter->setPen(QColor::fromRgb(hex_color));
    mp_painter->drawPolyline(point_buf.data(), vertices_length);
    point_buf.clear();
}
#endif
