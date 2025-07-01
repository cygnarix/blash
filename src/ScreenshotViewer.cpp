#include "ScreenshotViewer.h"
#include "StealthToolbar.h"
#include "ThumbnailLabel.h"
#include "AIWorker.h"
#include "ResponseDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QShowEvent>
#include <QCloseEvent>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ScreenshotViewer::ScreenshotViewer(const QList<QPixmap>& pixmaps, StealthToolbar* parent, const QString& modelChoice)
    : QWidget(parent)
    , m_toolbar(parent)
    , m_pixmaps(pixmaps)
    , m_modelSelection(modelChoice) {
    
    // Filter valid pixmaps
    QList<QPixmap> validPixmaps;
    for (const QPixmap& pixmap : m_pixmaps) {
        if (!pixmap.isNull()) {
            validPixmaps.append(pixmap);
        }
    }
    m_pixmaps = validPixmaps;
    
    if (m_pixmaps.isEmpty()) {
        QTimer::singleShot(0, this, &QWidget::close);
        return;
    }
    
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
    
    setupUI();
    setupButtons();
}

void ScreenshotViewer::setupUI() {
    // Create scroll area for thumbnails
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // Style scroll area
    m_scrollArea->setStyleSheet(
        "QScrollArea {"
        "background-color: transparent;"
        "border: none;"
        "}"
        "QScrollBar:horizontal {"
        "border: none;"
        "background: #1A1A1A;"
        "height: 9px;"
        "margin: 0 10px;"
        "border-radius: 4px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "background: rgba(90,90,90,0.85);"
        "min-width: 25px;"
        "border-radius: 4px;"
        "border: none;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "background: rgba(110,110,110,0.9);"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "border: none;"
        "background: none;"
        "width: 0px;"
        "}"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
        "background: none;"
        "}"
        "QScrollBar:vertical {"
        "border: none;"
        "background: transparent;"
        "width: 0px;"
        "margin: 0;"
        "}"
    );
    
    m_scrollArea->viewport()->setStyleSheet("background-color: transparent;");
    
    // Create container widget for thumbnails
    QWidget* container = new QWidget();
    container->setStyleSheet("background-color: transparent;");
    
    QHBoxLayout* thumbnailLayout = new QHBoxLayout(container);
    thumbnailLayout->setSpacing(12);
    thumbnailLayout->setContentsMargins(12, 8, 12, 8);
    thumbnailLayout->setAlignment(Qt::AlignLeft);
    
    int maxHeight = 0;
    const int thumbnailWidth = 180;
    
    // Add thumbnails
    for (const QPixmap& pixmap : m_pixmaps) {
        ThumbnailLabel* thumbnail = new ThumbnailLabel(pixmap, thumbnailWidth);
        thumbnailLayout->addWidget(thumbnail);
        maxHeight = qMax(maxHeight, thumbnail->height());
    }
    
    thumbnailLayout->addStretch(1);
    m_scrollArea->setWidget(container);
    
    // Create loading label
    m_loadingLabel = new QLabel("Processing...", this);
    m_loadingLabel->setStyleSheet(
        "color: white;"
        "background: rgba(0,0,0,150);"
        "padding: 5px;"
        "border-radius: 3px;"
    );
    m_loadingLabel->setAlignment(Qt::AlignCenter);
    m_loadingLabel->setFixedSize(100, 30);
    m_loadingLabel->hide();
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);
    
    // Calculate total height
    int totalHeight = maxHeight + 8 + 8 + 9 + 4; // thumbnail + margins + scrollbar + padding
    setFixedHeight(totalHeight);
    
    // Calculate width
    int estimatedWidth = thumbnailWidth + 12; // thumbnail + spacing
    int count = qMin(m_pixmaps.size(), 4);
    int totalWidth = count * estimatedWidth - 12 + 24; // count * (thumbnail + spacing) - last spacing + margins
    totalWidth = qMax(totalWidth, estimatedWidth + 24);
    setFixedWidth(totalWidth);
}

void ScreenshotViewer::setupButtons() {
    // Create buttons
    m_closeButton = new QPushButton("✗", this);
    m_confirmButton = new QPushButton("✓", this);
    m_restartButton = new QPushButton("↻", this);
    
    QSize buttonSize(19, 19);
    
    // Style buttons
    QString buttonStyleTemplate = 
        "QPushButton {"
        "background: transparent;"
        "border: none;"
        "border-radius: %1px;"
        "color: %2;"
        "font-size: 10pt;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background: %3;"
        "color: white;"
        "}"
        "QPushButton:pressed {"
        "background: %4;"
        "}";
    
    m_closeButton->setStyleSheet(buttonStyleTemplate
        .arg(buttonSize.height() / 2)
        .arg("#D0D0D0")
        .arg("rgba(230,0,0,0.9)")
        .arg("rgba(180,0,0,1)"));
    
    m_confirmButton->setStyleSheet(buttonStyleTemplate
        .arg(buttonSize.height() / 2)
        .arg("#77DD77")
        .arg("rgba(0,200,0,0.8)")
        .arg("rgba(0,150,0,1)"));
    
    m_restartButton->setStyleSheet(buttonStyleTemplate
        .arg(buttonSize.height() / 2)
        .arg("#ADD8E6")
        .arg("rgba(0,100,200,0.8)")
        .arg("rgba(0,70,150,1)"));
    
    // Set button properties
    for (QPushButton* button : {m_closeButton, m_confirmButton, m_restartButton}) {
        button->setFixedSize(buttonSize);
        button->setCursor(Qt::PointingHandCursor);
    }
    
    // Connect buttons
    connect(m_closeButton, &QPushButton::clicked, this, &QWidget::close);
    connect(m_confirmButton, &QPushButton::clicked, this, &ScreenshotViewer::onConfirm);
    connect(m_restartButton, &QPushButton::clicked, this, &ScreenshotViewer::onRestart);
    
    m_confirmButton->setEnabled(true);
}

void ScreenshotViewer::onConfirm() {
    if (m_thread && m_thread->isRunning()) {
        return;
    }
    
    // Show loading
    m_loadingLabel->move((width() - m_loadingLabel->width()) / 2, (height() - m_loadingLabel->height()) / 2);
    m_loadingLabel->show();
    
    // Disable buttons
    m_confirmButton->setEnabled(false);
    m_restartButton->setEnabled(false);
    m_closeButton->setEnabled(false);
    
    // Create AI worker thread
    m_thread = std::make_unique<QThread>(this);
    m_worker = std::make_unique<AIWorker>(m_pixmaps, m_modelSelection);
    
    m_worker->moveToThread(m_thread.get());
    
    connect(m_worker.get(), &AIWorker::finished, this, &ScreenshotViewer::onResult);
    connect(m_thread.get(), &QThread::started, m_worker.get(), &AIWorker::run);
    connect(m_thread.get(), &QThread::finished, m_thread.get(), &QThread::deleteLater);
    connect(m_worker.get(), &AIWorker::finished, m_worker.get(), &AIWorker::deleteLater);
    
    m_thread->start();
}

void ScreenshotViewer::onRestart() {
    if (m_thread && m_thread->isRunning()) {
        return;
    }
    
    close();
    
    if (m_toolbar) {
        m_toolbar->takeScreenshot();
    }
}

void ScreenshotViewer::onResult(const QString& text) {
    // Hide loading
    m_loadingLabel->hide();
    
    // Re-enable buttons
    m_confirmButton->setEnabled(true);
    m_restartButton->setEnabled(true);
    m_closeButton->setEnabled(true);
    
    // Clean up thread
    m_thread.reset();
    m_worker.reset();
    
    // Close this viewer
    close();
    
    // Create and show response dialog
    m_responseDialog = std::make_unique<ResponseDialog>(m_toolbar);
    m_responseDialog->setResponseText(text);
    connect(m_responseDialog.get(), &ResponseDialog::closed, this, &ScreenshotViewer::onResponseClosed);
    
    if (m_toolbar) {
        int x = m_toolbar->x();
        int y = m_toolbar->y() + m_toolbar->height() + 10;
        m_responseDialog->move(x, y);
    }
    
    m_responseDialog->show();
}

void ScreenshotViewer::onResponseClosed() {
    m_responseDialog.reset();
}

void ScreenshotViewer::applyCaptureProtection() {
#ifdef Q_OS_WIN
    try {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        if (hwnd) {
            SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
        }
    } catch (...) {
        qDebug() << "Failed to set display affinity for viewer";
    }
#endif
}

void ScreenshotViewer::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    
    // Position buttons
    int spacing = 3;
    int buttonWidth = m_closeButton->width();
    int totalButtonWidth = buttonWidth * 3 + spacing * 2;
    int startX = width() - totalButtonWidth - 5;
    
    m_restartButton->move(startX, 3);
    m_confirmButton->move(startX + buttonWidth + spacing, 3);
    m_closeButton->move(startX + (buttonWidth + spacing) * 2, 3);
    
    m_loadingLabel->move((width() - m_loadingLabel->width()) / 2, (height() - m_loadingLabel->height()) / 2);
    
    applyCaptureProtection();
}

void ScreenshotViewer::closeEvent(QCloseEvent* event) {
    if (m_thread && m_thread->isRunning()) {
        m_thread->quit();
        m_thread->wait(1000);
    }
    
    QWidget::closeEvent(event);
}

void ScreenshotViewer::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    
    // Draw main background
    painter.setBrush(QColor(30, 30, 30, 128));
    painter.drawRoundedRect(rect(), 6, 6);
    
    // Draw button area background
    painter.setBrush(QColor(55, 55, 55, 160));
    int buttonWidth = m_closeButton->width();
    int spacing = 3;
    int totalButtonWidth = buttonWidth * 3 + spacing * 2 + 8;
    QRectF buttonAreaRect(width() - totalButtonWidth, 0, totalButtonWidth, buttonWidth + 6);
    painter.drawRect(buttonAreaRect);
}

#include "ScreenshotViewer.moc"