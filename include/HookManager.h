#pragma once

#include <QObject>
#include <QSet>
#include <QThread>
#include <memory>
#include "Config.h"

class StealthToolbar;

class HookManager : public QObject {
    Q_OBJECT

public:
    explicit HookManager(StealthToolbar* toolbar, QObject* parent = nullptr);
    ~HookManager();

    void startListeners();
    void stopListeners();

signals:
    void screenshotRequested();
    void moveRequested(int dx, int dy);
    void toggleVisibilityRequested();
    void quitRequested();

private slots:
    void onKeyEvent(int keyCode, bool isDown);

private:
    void processKeyDown(int keyCode);
    void processKeyUp(int keyCode);
    bool isScreenshotCombination();
    void handleF2Mode(int keyCode, bool isDown);
    void handleMovement();

    StealthToolbar* m_toolbar;
    KeyboardShortcuts m_shortcuts;
    
    // Key state tracking
    QSet<int> m_pressedKeys;
    bool m_f2Pressed = false;
    bool m_ctrlPressed = false;
    bool m_altPressed = false;
    bool m_upPressed = false;
    bool m_downPressed = false;
    bool m_leftPressed = false;
    bool m_rightPressed = false;
    
    int m_moveStep = 10;
    
    class HookWorker;
    std::unique_ptr<HookWorker> m_worker;
    std::unique_ptr<QThread> m_thread;
};

// Platform-specific key mappings
namespace KeyCodes {
#ifdef Q_OS_WIN
    constexpr int VK_F2 = 0x71;
    constexpr int VK_RETURN = 0x0D;
    constexpr int VK_UP = 0x26;
    constexpr int VK_DOWN = 0x28;
    constexpr int VK_LEFT = 0x25;
    constexpr int VK_RIGHT = 0x27;
    constexpr int VK_CONTROL = 0x11;
    constexpr int VK_LCONTROL = 0xA2;
    constexpr int VK_RCONTROL = 0xA3;
    constexpr int VK_MENU = 0x12;
    constexpr int VK_LMENU = 0xA4;
    constexpr int VK_RMENU = 0xA5;
#elif defined(Q_OS_LINUX)
    // X11 keysyms
    constexpr int XK_F2 = 0xFFBF;
    constexpr int XK_Return = 0xFF0D;
    constexpr int XK_Up = 0xFF52;
    constexpr int XK_Down = 0xFF54;
    constexpr int XK_Left = 0xFF51;
    constexpr int XK_Right = 0xFF53;
    constexpr int XK_Control_L = 0xFFE3;
    constexpr int XK_Control_R = 0xFFE4;
    constexpr int XK_Alt_L = 0xFFE9;
    constexpr int XK_Alt_R = 0xFFEA;
#endif
    
    int getKeyCode(const QString& keyName);
}