#pragma once

#include <QApplication>
#include <memory>

class StealthToolbar;
class HookManager;

class BlashApp : public QApplication {
    Q_OBJECT

public:
    BlashApp(int& argc, char** argv);
    ~BlashApp();

    int run();

private slots:
    void onQuitRequested();

private:
    std::unique_ptr<StealthToolbar> m_toolbar;
    std::unique_ptr<HookManager> m_hookManager;
};