#include "DialogBox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QShowEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

DialogBox::DialogBox(QWidget* parent)
    : QDialog(parent) {
    
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
    
    setupUI();
}

void DialogBox::setupUI() {
    setFixedSize(280, 120);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);
    
    // Message label
    m_messageLabel = new QLabel("Screenshot taken successfully!\nWould you like to process it?");
    m_messageLabel->setStyleSheet(
        "QLabel {"
        "color: white;"
        "background-color: transparent;"
        "font-size: 11pt;"
        "padding: 5px;"
        "}");
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setWordWrap(true);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    // Screenshot button
    m_screenshotButton = new QPushButton("📷 Take Another");
    m_screenshotButton->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(70, 130, 180, 200);"
        "color: white;"
        "border: 1px solid rgba(255, 255, 255, 100);"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(100, 149, 237, 220);"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(50, 110, 160, 240);"
        "}");
    
    // Yes button
    m_yesButton = new QPushButton("✓ Yes");
    m_yesButton->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(60, 179, 113, 200);"
        "color: white;"
        "border: 1px solid rgba(255, 255, 255, 100);"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(90, 199, 143, 220);"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(40, 159, 93, 240);"
        "}");
    
    // No button
    m_noButton = new QPushButton("✗ No");
    m_noButton->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(220, 20, 60, 200);"
        "color: white;"
        "border: 1px solid rgba(255, 255, 255, 100);"
        "border-radius: 6px;"
        "padding: 8px 12px;"
        "font-size: 10pt;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(240, 50, 90, 220);"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(200, 0, 40, 240);"
        "}");
    
    // Connect buttons
    connect(m_screenshotButton, &QPushButton::clicked, [this]() {
        emit screenshotRequested();
    });
    
    connect(m_yesButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_noButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // Add buttons to layout
    buttonLayout->addWidget(m_screenshotButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_noButton);
    buttonLayout->addWidget(m_yesButton);
    
    // Add to main layout
    mainLayout->addWidget(m_messageLabel);
    mainLayout->addLayout(buttonLayout);
    
    // Set cursor for all buttons
    m_screenshotButton->setCursor(Qt::PointingHandCursor);
    m_yesButton->setCursor(Qt::PointingHandCursor);
    m_noButton->setCursor(Qt::PointingHandCursor);
}

void DialogBox::applyCaptureProtection() {
#ifdef Q_OS_WIN
    try {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        if (hwnd) {
            BOOL success = SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
            if (!success) {
                SetWindowDisplayAffinity(hwnd, WDA_MONITOR);
            }
        }
    } catch (...) {
        // Silently handle errors
    }
#endif
}

void DialogBox::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.setBrush(QColor(40, 40, 40, 220));
    painter.setPen(QPen(QColor(100, 100, 100, 150), 1));
    painter.drawRoundedRect(rect(), 8, 8);
    
    // Draw subtle border
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(255, 255, 255, 50), 1));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

void DialogBox::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QTimer::singleShot(10, this, &DialogBox::applyCaptureProtection);
}

#include "DialogBox.moc"