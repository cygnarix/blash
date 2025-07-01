#include "ResponseDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QClipboard>
#include <QApplication>
#include <QShowEvent>
#include <QCloseEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ResponseDialog::ResponseDialog(QWidget* parent)
    : QDialog(parent) {
    
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
    
    setupUI();
}

void ResponseDialog::setupUI() {
    setFixedSize(600, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);
    
    // Title label
    m_titleLabel = new QLabel("AI Response");
    m_titleLabel->setStyleSheet(
        "QLabel {"
        "color: white;"
        "background-color: transparent;"
        "font-size: 14pt;"
        "font-weight: bold;"
        "padding: 5px;"
        "}");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    
    // Text edit for response
    m_textEdit = new QTextEdit();
    m_textEdit->setStyleSheet(
        "QTextEdit {"
        "background-color: rgba(60, 60, 60, 200);"
        "color: white;"
        "border: 1px solid rgba(255, 255, 255, 100);"
        "border-radius: 8px;"
        "padding: 10px;"
        "font-size: 11pt;"
        "font-family: 'Consolas', 'Monaco', 'Courier New', monospace;"
        "}"
        "QScrollBar:vertical {"
        "border: none;"
        "background: rgba(80, 80, 80, 150);"
        "width: 12px;"
        "margin: 0;"
        "border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "background: rgba(120, 120, 120, 200);"
        "min-height: 20px;"
        "border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "background: rgba(140, 140, 140, 220);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "border: none;"
        "background: none;"
        "height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "background: none;"
        "}"
    );
    m_textEdit->setReadOnly(true);
    
    // Button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    // Copy button
    m_copyButton = new QPushButton("📋 Copy");
    m_copyButton->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(70, 130, 180, 200);"
        "color: white;"
        "border: 1px solid rgba(255, 255, 255, 100);"
        "border-radius: 6px;"
        "padding: 8px 16px;"
        "font-size: 11pt;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(100, 149, 237, 220);"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(50, 110, 160, 240);"
        "}");
    m_copyButton->setCursor(Qt::PointingHandCursor);
    
    // Close button
    m_closeButton = new QPushButton("✗ Close");
    m_closeButton->setStyleSheet(
        "QPushButton {"
        "background-color: rgba(220, 20, 60, 200);"
        "color: white;"
        "border: 1px solid rgba(255, 255, 255, 100);"
        "border-radius: 6px;"
        "padding: 8px 16px;"
        "font-size: 11pt;"
        "}"
        "QPushButton:hover {"
        "background-color: rgba(240, 50, 90, 220);"
        "}"
        "QPushButton:pressed {"
        "background-color: rgba(200, 0, 40, 240);"
        "}");
    m_closeButton->setCursor(Qt::PointingHandCursor);
    
    // Connect buttons
    connect(m_copyButton, &QPushButton::clicked, this, &ResponseDialog::onCopyClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::close);
    
    // Add buttons to layout
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_copyButton);
    buttonLayout->addWidget(m_closeButton);
    
    // Add to main layout
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_textEdit, 1); // Give text edit most of the space
    mainLayout->addLayout(buttonLayout);
}

void ResponseDialog::setResponseText(const QString& text) {
    if (m_textEdit) {
        m_textEdit->setPlainText(text);
        
        // Move cursor to beginning
        QTextCursor cursor = m_textEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        m_textEdit->setTextCursor(cursor);
    }
}

void ResponseDialog::onCopyClicked() {
    if (m_textEdit) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(m_textEdit->toPlainText());
        
        // Provide visual feedback
        QString originalText = m_copyButton->text();
        m_copyButton->setText("✓ Copied!");
        m_copyButton->setEnabled(false);
        
        QTimer::singleShot(1500, [this, originalText]() {
            m_copyButton->setText(originalText);
            m_copyButton->setEnabled(true);
        });
    }
}

void ResponseDialog::applyCaptureProtection() {
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

void ResponseDialog::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    painter.setBrush(QColor(30, 30, 30, 220));
    painter.setPen(QPen(QColor(100, 100, 100, 150), 2));
    painter.drawRoundedRect(rect(), 10, 10);
    
    // Draw subtle border
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(255, 255, 255, 50), 1));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
}

void ResponseDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    QTimer::singleShot(10, this, &ResponseDialog::applyCaptureProtection);
}

void ResponseDialog::closeEvent(QCloseEvent* event) {
    emit closed();
    QDialog::closeEvent(event);
}

#include "ResponseDialog.moc"