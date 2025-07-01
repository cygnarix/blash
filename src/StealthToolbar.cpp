#include "StealthToolbar.h"
#include "DialogBox.h"
#include "ScreenshotViewer.h"
#include "ResponseDialog.h"
#include "Config.h"
#include <QApplication>
#include <QScreen>
#include <QPainter>
#include <QTimer>
#include <QPixmap>
#include <QPaintEvent>
#include <QCloseEvent>
#include <QShowEvent>
#include <QIcon>
#include <QFont>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#endif

StealthToolbar::StealthToolbar(QWidget* parent)
    : QWidget(parent)
    , m_screenshotTimer(new QTimer(this)) {
    
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, true);
    
    setupUI();
    
    // Connect timer
    connect(m_screenshotTimer, &QTimer::timeout,
            this, &StealthToolbar::onTakeScreenshotImpl);
    
    // Position window
    setFixedSize(340, 50);
    QScreen* screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        move((screenGeometry.width() - width()) / 2, 50);
    }
    
    // Apply capture protection after a short delay
    QTimer::singleShot(10, this, &StealthToolbar::applyCaptureProtection);
}

StealthToolbar::~StealthToolbar() = default;

void StealthToolbar::setupUI() {
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(10, 0, 10, 0);
    m_layout->setSpacing(2);
    
    // Key style
    QString keyStyle = 
        "QLabel {"
        "color: white;"
        "background-color: rgba(255, 255, 255, 25);"
        "border: 1px solid rgba(255, 255, 255, 100);"
        "border-radius: 4px;"
        "padding: 1.3px;"
        "margin: 0;"
        "font-size: 9pt;"
        "}";
    
    QSize keySize(28, 28);
    
    // Create Ctrl label
    m_ctrlLabel = new QLabel("Ctrl");
    m_ctrlLabel->setStyleSheet(keyStyle);
    m_ctrlLabel->setFixedSize(keySize);
    m_ctrlLabel->setAlignment(Qt::AlignCenter);
    
    // Create Alt label
    m_altLabel = new QLabel("Alt");
    m_altLabel->setStyleSheet(keyStyle);
    m_altLabel->setFixedSize(keySize);
    m_altLabel->setAlignment(Qt::AlignCenter);
    
    // Create text label
    m_textLabel = new QLabel(" to take a screenshot");
    m_textLabel->setStyleSheet("color: white; padding-left: 3px; font-size: 9pt;");
    
    // Create hyphen label
    m_hyphenLabel = new QLabel("   –   ");
    m_hyphenLabel->setStyleSheet("color: white; font-weight: bold; font-size: 9pt;");
    
    // Create screenshot icon (simplified)
    m_iconLabel = new QLabel("📷");
    m_iconLabel->setStyleSheet("color: white; font-size: 18pt;");
    m_iconLabel->setAlignment(Qt::AlignCenter);
    
    // Create separator
    m_separator = new QFrame();
    m_separator->setFixedHeight(20);
    m_separator->setFixedWidth(1);
    m_separator->setStyleSheet(
        "QFrame { border: none; background-color: rgba(255, 255, 255, 240); border-radius: 15px; }");
    
    // Create settings button
    m_settingsButton = new QPushButton("⚙");
    m_settingsButton->setStyleSheet(
        "QPushButton { background-color: transparent; border: none; padding: 3px; margin: 0px; color: white; font-size: 16pt; }"
        "QPushButton:hover { background-color: rgba(255, 255, 255, 30); border-radius: 4px; }"
        "QPushButton:pressed { background-color: rgba(255, 255, 255, 50); border-radius: 4px; }");
    m_settingsButton->setCursor(Qt::PointingHandCursor);
    m_settingsButton->setFixedSize(22, 22);
    
    connect(m_settingsButton, &QPushButton::clicked,
            this, &StealthToolbar::onSettingsClicked);
    
    // Add widgets to layout
    m_layout->addStretch(1);
    m_layout->addWidget(m_ctrlLabel);
    m_layout->addSpacing(5);
    m_layout->addWidget(m_altLabel);
    m_layout->addWidget(m_textLabel);
    m_layout->addWidget(m_hyphenLabel);
    m_layout->addWidget(m_iconLabel);
    m_layout->addSpacing(16);
    m_layout->addWidget(m_separator);
    m_layout->addSpacing(16);
    m_layout->addWidget(m_settingsButton);
    m_layout->addStretch(1);
}

void StealthToolbar::takeScreenshot() {
    if (isViewerVisible() || isResponseVisible()) {
        return;
    }
    
    // Delay screenshot to hide windows first
    QTimer::singleShot(100, this, &StealthToolbar::onTakeScreenshotImpl);
}

void StealthToolbar::onTakeScreenshotImpl() {
    if (isViewerVisible() || isResponseVisible()) {
        return;
    }
    
    // Store visibility states
    bool toolbarVisible = isVisible();
    bool viewerVisible = isViewerVisible();
    bool responseVisible = isResponseVisible();
    
    // Hide all windows
    hideAllWindows(toolbarVisible, viewerVisible, responseVisible);
    
    // Process events and take screenshot after delay
    QApplication::processEvents();
    QTimer::singleShot(50, [this, toolbarVisible, viewerVisible, responseVisible]() {
        onGrabAndShowDialog(toolbarVisible, viewerVisible, responseVisible);
    });
}

void StealthToolbar::hideAllWindows(bool& toolbarVisible, bool& viewerVisible, bool& responseVisible) {
    toolbarVisible = isVisible();
    viewerVisible = isViewerVisible();
    responseVisible = isResponseVisible();
    
    if (toolbarVisible) hide();
    if (viewerVisible && m_viewerWindow) m_viewerWindow->hide();
    if (responseVisible && m_responseDialog) m_responseDialog->hide();
    if (m_dialogBox) m_dialogBox->hide();
}

void StealthToolbar::restoreWindows(bool toolbarVisible, bool viewerVisible, bool responseVisible) {
    if (toolbarVisible) {
        show();
        applyCaptureProtection();
    }
    if (viewerVisible && m_viewerWindow) {
        m_viewerWindow->show();
        applyCaptureProtectionToWindow(m_viewerWindow.get());
    }
    if (responseVisible && m_responseDialog) {
        m_responseDialog->show();
        applyCaptureProtectionToWindow(m_responseDialog.get());
    }
}

void StealthToolbar::onGrabAndShowDialog(bool toolbarVisible, bool viewerVisible, bool responseVisible) {
    try {
        // Take screenshot of all screens
        QScreen* primaryScreen = QApplication::primaryScreen();
        if (primaryScreen) {
            QPixmap screenshot = primaryScreen->grabWindow(0);
            if (!screenshot.isNull()) {
                m_screenshotPixmaps.append(screenshot);
            }
        }
    } catch (...) {
        qDebug() << "Failed to take screenshot";
    }
    
    // Restore windows
    restoreWindows(toolbarVisible, viewerVisible, responseVisible);
    
    // Close existing dialog if any
    if (m_dialogBox) {
        disconnect(m_dialogBox.get(), &DialogBox::screenshotRequested,
                   this, &StealthToolbar::takeScreenshot);
        m_dialogBox.reset();
    }
    
    // Create new dialog
    m_dialogBox = std::make_unique<DialogBox>(this);
    connect(m_dialogBox.get(), &DialogBox::screenshotRequested,
            this, &StealthToolbar::takeScreenshot);
    connect(m_dialogBox.get(), &DialogBox::finished,
            this, &StealthToolbar::onDialogFinished);
    
    // Position dialog
    int dialogX = x();
    int dialogY = y() + height() + 10;
    m_dialogBox->move(dialogX, dialogY);
    m_dialogBox->open();
    
    // Apply capture protection
    QTimer::singleShot(100, [this]() {
        applyCaptureProtectionToWindow(m_dialogBox.get());
    });
}

void StealthToolbar::onDialogFinished(int result) {
    m_dialogBox.reset();
    
    if (result == QDialog::Rejected) {
        showScreenshotViewer();
    } else {
        if (m_viewerWindow) {
            m_viewerWindow.reset();
        }
    }
}

void StealthToolbar::showScreenshotViewer() {
    if (m_screenshotPixmaps.isEmpty()) {
        return;
    }
    
    if (m_viewerWindow) {
        m_viewerWindow.reset();
    }
    
    QString modelChoice = Config::instance().getSelectedModel();
    m_viewerWindow = std::make_unique<ScreenshotViewer>(m_screenshotPixmaps, this, modelChoice);
    
    if (m_viewerWindow) {
        int viewerX = x();
        int viewerY = y() + height() + 10;
        m_viewerWindow->move(viewerX, viewerY);
        m_viewerWindow->show();
        
        QTimer::singleShot(100, [this]() {
            applyCaptureProtectionToWindow(m_viewerWindow.get());
        });
    }
    
    m_screenshotPixmaps.clear();
}

void StealthToolbar::toggleVisibility() {
    if (isVisible()) {
        // Store visibility states
        m_dialogWasVisible = isDialogVisible();
        m_viewerWasVisible = isViewerVisible();
        m_responseWasVisible = isResponseVisible();
        
        // Hide all windows
        hide();
        if (m_dialogBox) m_dialogBox->hide();
        if (m_viewerWindow) m_viewerWindow->hide();
        if (m_responseDialog) m_responseDialog->hide();
    } else {
        // Show main window
        show();
        applyCaptureProtection();
        
        // Restore other windows
        if (m_dialogWasVisible && m_dialogBox) {
            m_dialogBox->show();
            applyCaptureProtectionToWindow(m_dialogBox.get());
        }
        if (m_viewerWasVisible && m_viewerWindow) {
            m_viewerWindow->show();
            applyCaptureProtectionToWindow(m_viewerWindow.get());
        }
        if (m_responseWasVisible && m_responseDialog) {
            m_responseDialog->show();
            applyCaptureProtectionToWindow(m_responseDialog.get());
        }
    }
}

void StealthToolbar::handleMoveRequest(int dx, int dy) {
    QPoint currentPos = pos();
    QPoint newPos(currentPos.x() + dx, currentPos.y() + dy);
    move(newPos);
    
    int dialogOffsetY = height() + 10;
    
    // Move child windows
    if (m_dialogBox) {
        m_dialogBox->move(newPos.x(), newPos.y() + dialogOffsetY);
    }
    if (m_viewerWindow) {
        m_viewerWindow->move(newPos.x(), newPos.y() + dialogOffsetY);
    }
    if (m_responseDialog) {
        m_responseDialog->move(newPos.x(), newPos.y() + dialogOffsetY);
    }
}

bool StealthToolbar::isDialogVisible() const {
    return m_dialogBox && m_dialogBox->isVisible();
}

bool StealthToolbar::isViewerVisible() const {
    return m_viewerWindow && m_viewerWindow->isVisible();
}

bool StealthToolbar::isResponseVisible() const {
    return m_responseDialog && m_responseDialog->isVisible();
}

void StealthToolbar::onSettingsClicked() {
    qDebug() << "Settings clicked - configuration dialog would open here";
    // TODO: Implement settings dialog
}

void StealthToolbar::applyCaptureProtection() {
    if (!isVisible()) {
        return;
    }
    
#ifdef Q_OS_WIN
    try {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        if (hwnd) {
            // Try to exclude from capture
            BOOL success = SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
            if (!success) {
                // Fallback to monitor affinity
                SetWindowDisplayAffinity(hwnd, WDA_MONITOR);
            }
        }
    } catch (...) {
        qDebug() << "Failed to apply capture protection";
    }
#elif defined(Q_OS_LINUX)
    // TODO: Implement Linux capture protection if available
#endif
}

void StealthToolbar::applyCaptureProtectionToWindow(QWidget* window) {
    if (!window || !window->isVisible()) {
        return;
    }
    
#ifdef Q_OS_WIN
    try {
        HWND hwnd = reinterpret_cast<HWND>(window->winId());
        if (hwnd) {
            BOOL success = SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
            if (!success) {
                SetWindowDisplayAffinity(hwnd, WDA_MONITOR);
            }
        }
    } catch (...) {
        qDebug() << "Failed to apply capture protection to window";
    }
#elif defined(Q_OS_LINUX)
    // TODO: Implement Linux capture protection if available
#endif
}

void StealthToolbar::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(0, 0, 0, 160));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);
}

void StealthToolbar::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    QTimer::singleShot(10, this, &StealthToolbar::applyCaptureProtection);
}

void StealthToolbar::closeEvent(QCloseEvent* event) {
    if (m_dialogBox) m_dialogBox.reset();
    if (m_viewerWindow) m_viewerWindow.reset();
    if (m_responseDialog) m_responseDialog.reset();
    
    emit quitRequested();
    QWidget::closeEvent(event);
}

#include "StealthToolbar.moc"