#include "BlashApp.h"
#include "Config.h"
#include <QDir>
#include <QStandardPaths>
#include <iostream>

int main(int argc, char *argv[]) {
    BlashApp app(argc, argv);
    
    // Load configuration
    if (!Config::instance().loadConfig()) {
        std::cerr << "Warning: Could not load configuration file. Using defaults." << std::endl;
    }
    
    return app.run();
}