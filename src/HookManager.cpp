#include "HookManager.h"
#include "StealthToolbar.h"
#include "Config.h"
#include <QDebug>
#include <QThread>

#ifdef Q_OS_WIN
#include <windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#endif

// Platform-specific key mappings implementation
namespace KeyCodes {
    int getKeyCode(const QString& keyName) {
#ifdef Q_OS_WIN
        if (keyName == "F2") return VK_F2;
        if (keyName == "RETURN") return VK_RETURN;
        if (keyName == "UP") return VK_UP;
        if (keyName == "DOWN") return VK_DOWN;
        if (keyName == "LEFT") return VK_LEFT;
        if (keyName == "RIGHT") return VK_RIGHT;
        if (keyName == "CONTROL") return VK_CONTROL;
        if (keyName == "MENU") return VK_MENU;
        if (keyName.length() == 1) return keyName[0].toUpper().toLatin1();
#elif defined(Q_OS_LINUX)
        if (keyName == "F2") return XK_F2;
        if (keyName == "RETURN") return XK_Return;
        if (keyName == "UP") return XK_Up;
        if (keyName == "DOWN") return XK_Down;
        if (keyName == "LEFT") return XK_Left;
        if (keyName == "RIGHT") return XK_Right;
        if (keyName == "CONTROL") return XK_Control_L;
        if (keyName == "MENU") return XK_Alt_L;
        if (keyName.length() == 1) return keyName[0].toUpper().toLatin1();
#endif
        return 0;
    }
}

// Platform-specific hook worker
class HookManager::HookWorker : public QObject {
    Q_OBJECT

public:
    HookWorker(HookManager* manager) : m_manager(manager) {}

signals:
    void keyEvent(int keyCode, bool isDown);

public slots:
    void startHook();
    void stopHook();

private:
    HookManager* m_manager;
    bool m_running = false;

#ifdef Q_OS_WIN
    HHOOK m_keyboardHook = nullptr;
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static HookWorker* s_instance;
#elif defined(Q_OS_LINUX)
    Display* m_display = nullptr;
    void processX11Events();
#endif
};

#ifdef Q_OS_WIN
HookManager::HookWorker* HookManager::HookWorker::s_instance = nullptr;

LRESULT CALLBACK HookManager::HookWorker::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && s_instance) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        bool isDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        emit s_instance->keyEvent(kbStruct->vkCode, isDown);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void HookManager::HookWorker::startHook() {
    if (m_running) return;
    
    s_instance = this;
    m_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, GetModuleHandle(nullptr), 0);
    
    if (!m_keyboardHook) {
        qDebug() << "Failed to set keyboard hook";
        return;
    }
    
    m_running = true;
    
    MSG msg;
    while (m_running && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void HookManager::HookWorker::stopHook() {
    m_running = false;
    if (m_keyboardHook) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = nullptr;
    }
    s_instance = nullptr;
    PostQuitMessage(0);
}

#elif defined(Q_OS_LINUX)

void HookManager::HookWorker::startHook() {
    if (m_running) return;
    
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        qDebug() << "Failed to open X11 display";
        return;
    }
    
    m_running = true;
    processX11Events();
}

void HookManager::HookWorker::stopHook() {
    m_running = false;
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
}

void HookManager::HookWorker::processX11Events() {
    // Simplified X11 event processing
    // In a real implementation, you'd need to set up proper key event monitoring
    // This is a placeholder for the X11 implementation
    while (m_running) {
        QThread::msleep(10);
        // TODO: Implement proper X11 global key hook
    }
}

#endif

HookManager::HookManager(StealthToolbar* toolbar, QObject* parent)
    : QObject(parent)
    , m_toolbar(toolbar)
    , m_shortcuts(Config::instance().getShortcuts()) {
}

HookManager::~HookManager() {
    stopListeners();
}

void HookManager::startListeners() {
    if (m_thread && m_thread->isRunning()) {
        return;
    }

    m_thread = std::make_unique<QThread>();
    m_worker = std::make_unique<HookWorker>(this);
    
    m_worker->moveToThread(m_thread.get());
    
    connect(m_worker.get(), &HookWorker::keyEvent,
            this, &HookManager::onKeyEvent);
    
    connect(m_thread.get(), &QThread::started,
            m_worker.get(), &HookWorker::startHook);
    
    connect(m_thread.get(), &QThread::finished,
            m_worker.get(), &HookWorker::stopHook);
    
    m_thread->start();
    
    qDebug() << "Hook manager started";
}

void HookManager::stopListeners() {
    if (m_thread && m_thread->isRunning()) {
        if (m_worker) {
            m_worker->stopHook();
        }
        m_thread->quit();
        m_thread->wait(2000);
    }
    
    m_worker.reset();
    m_thread.reset();
    
    qDebug() << "Hook manager stopped";
}

void HookManager::onKeyEvent(int keyCode, bool isDown) {
    if (isDown) {
        processKeyDown(keyCode);
    } else {
        processKeyUp(keyCode);
    }
}

void HookManager::processKeyDown(int keyCode) {
    m_pressedKeys.insert(keyCode);
    
    // Update modifier states
#ifdef Q_OS_WIN
    if (keyCode == VK_F2) m_f2Pressed = true;
    else if (keyCode == VK_CONTROL || keyCode == VK_LCONTROL || keyCode == VK_RCONTROL) m_ctrlPressed = true;
    else if (keyCode == VK_MENU || keyCode == VK_LMENU || keyCode == VK_RMENU) m_altPressed = true;
    else if (keyCode == VK_UP) m_upPressed = true;
    else if (keyCode == VK_DOWN) m_downPressed = true;
    else if (keyCode == VK_LEFT) m_leftPressed = true;
    else if (keyCode == VK_RIGHT) m_rightPressed = true;
#elif defined(Q_OS_LINUX)
    if (keyCode == XK_F2) m_f2Pressed = true;
    else if (keyCode == XK_Control_L || keyCode == XK_Control_R) m_ctrlPressed = true;
    else if (keyCode == XK_Alt_L || keyCode == XK_Alt_R) m_altPressed = true;
    else if (keyCode == XK_Up) m_upPressed = true;
    else if (keyCode == XK_Down) m_downPressed = true;
    else if (keyCode == XK_Left) m_leftPressed = true;
    else if (keyCode == XK_Right) m_rightPressed = true;
#endif
    
    // Check for screenshot combination
    if (isScreenshotCombination()) {
        if (!m_toolbar->isDialogVisible() && !m_toolbar->isViewerVisible() && !m_toolbar->isResponseVisible()) {
            emit screenshotRequested();
        }
    }
    
    // Handle F2 mode
    if (m_f2Pressed) {
        handleF2Mode(keyCode, true);
    }
}

void HookManager::processKeyUp(int keyCode) {
    m_pressedKeys.remove(keyCode);
    
    // Update modifier states
#ifdef Q_OS_WIN
    if (keyCode == VK_F2) m_f2Pressed = false;
    else if (keyCode == VK_CONTROL || keyCode == VK_LCONTROL || keyCode == VK_RCONTROL) m_ctrlPressed = false;
    else if (keyCode == VK_MENU || keyCode == VK_LMENU || keyCode == VK_RMENU) m_altPressed = false;
    else if (keyCode == VK_UP) m_upPressed = false;
    else if (keyCode == VK_DOWN) m_downPressed = false;
    else if (keyCode == VK_LEFT) m_leftPressed = false;
    else if (keyCode == VK_RIGHT) m_rightPressed = false;
#elif defined(Q_OS_LINUX)
    if (keyCode == XK_F2) m_f2Pressed = false;
    else if (keyCode == XK_Control_L || keyCode == XK_Control_R) m_ctrlPressed = false;
    else if (keyCode == XK_Alt_L || keyCode == XK_Alt_R) m_altPressed = false;
    else if (keyCode == XK_Up) m_upPressed = false;
    else if (keyCode == XK_Down) m_downPressed = false;
    else if (keyCode == XK_Left) m_leftPressed = false;
    else if (keyCode == XK_Right) m_rightPressed = false;
#endif
}

bool HookManager::isScreenshotCombination() {
    if (m_shortcuts.screenshot.triggerOn == "second_modifier") {
        bool ctrlRequired = m_shortcuts.screenshot.keys.contains("CONTROL");
        bool altRequired = m_shortcuts.screenshot.keys.contains("MENU");
        
        if (ctrlRequired && altRequired) {
            return m_ctrlPressed && m_altPressed;
        }
    }
    return false;
}

void HookManager::handleF2Mode(int keyCode, bool isDown) {
    if (!isDown) return;
    
#ifdef Q_OS_WIN
    if (keyCode == VK_UP || keyCode == VK_DOWN || keyCode == VK_LEFT || keyCode == VK_RIGHT) {
#elif defined(Q_OS_LINUX)
    if (keyCode == XK_Up || keyCode == XK_Down || keyCode == XK_Left || keyCode == XK_Right) {
#endif
        handleMovement();
        return;
    }
    
    // Check for F2 actions
    for (auto it = m_shortcuts.f2Mode.actions.begin(); it != m_shortcuts.f2Mode.actions.end(); ++it) {
        int actionKeyCode = KeyCodes::getKeyCode(it.key());
        if (actionKeyCode == keyCode) {
            QString action = it.value();
            if (action == "toggle_visibility") {
                emit toggleVisibilityRequested();
            } else if (action == "quit_app") {
                emit quitRequested();
            }
            break;
        }
    }
}

void HookManager::handleMovement() {
    int dx = 0, dy = 0;
    
    if (m_upPressed) dy -= m_moveStep;
    if (m_downPressed) dy += m_moveStep;
    if (m_leftPressed) dx -= m_moveStep;
    if (m_rightPressed) dx += m_moveStep;
    
    if (dx != 0 || dy != 0) {
        emit moveRequested(dx, dy);
    }
}

#include "HookManager.moc"