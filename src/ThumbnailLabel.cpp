#include "ThumbnailLabel.h"
#include <QPainter>
#include <QEnterEvent>
#include <QMouseEvent>

ThumbnailLabel::ThumbnailLabel(const QPixmap& pixmap, int thumbnailWidth, QWidget* parent)
    : QWidget(parent)
    , m_fullPixmap(pixmap) {
    
    // Create thumbnail
    if (!m_fullPixmap.isNull()) {
        m_thumbnail = m_fullPixmap.scaledToWidth(thumbnailWidth, Qt::SmoothTransformation);
    }
    
    setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
    
    // Calculate size
    int height = thumbnailWidth;
    if (!m_thumbnail.isNull() && m_thumbnail.width() > 0) {
        height = static_cast<int>(thumbnailWidth * (static_cast<double>(m_thumbnail.height()) / m_thumbnail.width()));
    }
    
    setFixedSize(thumbnailWidth, height);
}

void ThumbnailLabel::enterEvent(QEnterEvent* event) {
    m_zoomed = true;
    m_mousePos = event->position();
    update();
}

void ThumbnailLabel::leaveEvent(QEvent* event) {
    Q_UNUSED(event)
    m_zoomed = false;
    update();
}

void ThumbnailLabel::mouseMoveEvent(QMouseEvent* event) {
    if (m_zoomed) {
        m_mousePos = event->position();
        update();
    }
}

void ThumbnailLabel::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(rect());
    
    if (!m_zoomed || m_fullPixmap.isNull()) {
        // Draw normal thumbnail
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        QPixmap scaledPixmap = m_thumbnail.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        int x = (width() - scaledPixmap.width()) / 2;
        int y = (height() - scaledPixmap.height()) / 2;
        painter.drawPixmap(x, y, scaledPixmap);
    } else {
        // Draw zoomed portion
        double zoomWidth = width() / ZOOM_FACTOR;
        double zoomHeight = height() / ZOOM_FACTOR;
        
        // Calculate center position based on mouse
        double centerX = (m_mousePos.x() / width()) * m_fullPixmap.width();
        double centerY = (m_mousePos.y() / height()) * m_fullPixmap.height();
        
        // Calculate source rectangle
        double sourceX = qMax(0.0, qMin(centerX - zoomWidth/2, m_fullPixmap.width() - zoomWidth));
        double sourceY = qMax(0.0, qMin(centerY - zoomHeight/2, m_fullPixmap.height() - zoomHeight));
        
        QRectF sourceRect(sourceX, sourceY, zoomWidth, zoomHeight);
        QRectF targetRect(0, 0, width(), height());
        
        painter.drawPixmap(targetRect, m_fullPixmap, sourceRect);
    }
}

#include "ThumbnailLabel.moc"