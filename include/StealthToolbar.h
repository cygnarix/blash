#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QTimer>
#include <memory>

class HookManager;
class DialogBox;
class ScreenshotViewer;
class ResponseDialog;

class StealthToolbar : public QWidget {
    Q_OBJECT

public:
    explicit StealthToolbar(QWidget* parent = nullptr);
    ~StealthToolbar();

    void setHookManager(HookManager* hookManager) { m_hookManager = hookManager; }
    bool isDialogVisible() const;
    bool isViewerVisible() const;
    bool isResponseVisible() const;

public slots:
    void takeScreenshot();
    void toggleVisibility();
    void handleMoveRequest(int dx, int dy);

signals:
    void quitRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onTakeScreenshotImpl();
    void onGrabAndShowDialog(bool toolbarVisible, bool viewerVisible, bool responseVisible);
    void onDialogFinished(int result);
    void onSettingsClicked();

private:
    void setupUI();
    void applyCaptureProtection();
    void applyCaptureProtectionToWindow(QWidget* window);
    void showScreenshotViewer();
    void hideAllWindows(bool& toolbarVisible, bool& viewerVisible, bool& responseVisible);
    void restoreWindows(bool toolbarVisible, bool viewerVisible, bool responseVisible);

    HookManager* m_hookManager = nullptr;
    std::unique_ptr<DialogBox> m_dialogBox;
    std::unique_ptr<ScreenshotViewer> m_viewerWindow;
    std::unique_ptr<ResponseDialog> m_responseDialog;
    
    QList<QPixmap> m_screenshotPixmaps;
    int m_moveStep = 10;
    
    // UI state tracking for toggle visibility
    bool m_dialogWasVisible = false;
    bool m_viewerWasVisible = false;
    bool m_responseWasVisible = false;
    
    // UI components
    QHBoxLayout* m_layout;
    QLabel* m_ctrlLabel;
    QLabel* m_altLabel;
    QLabel* m_textLabel;
    QLabel* m_hyphenLabel;
    QLabel* m_iconLabel;
    QFrame* m_separator;
    QPushButton* m_settingsButton;
    QTimer* m_screenshotTimer;
};