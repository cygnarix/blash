#include "BlashApp.h"
#include "StealthToolbar.h"
#include "HookManager.h"
#include <QDebug>

BlashApp::BlashApp(int& argc, char** argv) 
    : QApplication(argc, argv) {
    
    setQuitOnLastWindowClosed(false);
    setApplicationName("Blash");
    setApplicationVersion("2.0.0");
    setOrganizationName("Blash Team");
}

BlashApp::~BlashApp() = default;

int BlashApp::run() {
    // Create the main toolbar
    m_toolbar = std::make_unique<StealthToolbar>();
    
    // Create the hook manager
    m_hookManager = std::make_unique<HookManager>(m_toolbar.get());
    
    // Connect signals
    connect(m_toolbar.get(), &StealthToolbar::quitRequested, 
            this, &BlashApp::onQuitRequested);
    
    connect(m_hookManager.get(), &HookManager::screenshotRequested,
            m_toolbar.get(), &StealthToolbar::takeScreenshot);
    
    connect(m_hookManager.get(), &HookManager::moveRequested,
            m_toolbar.get(), &StealthToolbar::handleMoveRequest);
    
    connect(m_hookManager.get(), &HookManager::toggleVisibilityRequested,
            m_toolbar.get(), &StealthToolbar::toggleVisibility);
    
    connect(m_hookManager.get(), &HookManager::quitRequested,
            this, &BlashApp::onQuitRequested);
    
    // Set up the hook manager reference in toolbar
    m_toolbar->setHookManager(m_hookManager.get());
    
    // Start global hooks
    m_hookManager->startListeners();
    
    // Show the toolbar
    m_toolbar->show();
    
    qDebug() << "Blash C++ started successfully!";
    
    return exec();
}

void BlashApp::onQuitRequested() {
    if (m_hookManager) {
        m_hookManager->stopListeners();
    }
    quit();
}

#include "BlashApp.moc"