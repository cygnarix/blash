#pragma once

#include <QWidget>
#include <QPixmap>
#include <QPointF>

class ThumbnailLabel : public QWidget {
    Q_OBJECT

public:
    explicit ThumbnailLabel(const QPixmap& pixmap, int thumbnailWidth = 180, QWidget* parent = nullptr);

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QPixmap m_fullPixmap;
    QPixmap m_thumbnail;
    bool m_zoomed = false;
    QPointF m_mousePos;
    static constexpr double ZOOM_FACTOR = 0.6;
};