#include "Config.h"
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QDebug>

Config& Config::instance() {
    static Config instance;
    return instance;
}

bool Config::loadConfig(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open config file:" << filePath;
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON parse error:" << error.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    // Parse screenshot configuration
    QJsonObject screenshotObj = root["screenshot"].toObject();
    QJsonArray keysArray = screenshotObj["keys"].toArray();
    
    shortcuts.screenshot.keys.clear();
    for (const QJsonValue& value : keysArray) {
        shortcuts.screenshot.keys.append(value.toString());
    }
    shortcuts.screenshot.triggerOn = screenshotObj["trigger_on"].toString();

    // Parse F2 mode configuration
    QJsonObject f2ModeObj = root["f2_mode"].toObject();
    shortcuts.f2Mode.modifier = f2ModeObj["modifier"].toString();
    
    QJsonObject actionsObj = f2ModeObj["actions"].toObject();
    shortcuts.f2Mode.actions.clear();
    for (auto it = actionsObj.begin(); it != actionsObj.end(); ++it) {
        shortcuts.f2Mode.actions[it.key()] = it.value().toString();
    }

    return true;
}

QString Config::getPrompt() const {
    return QString(
        "Greetings. Imagine you're a senior software engineer, a very skilled individual, in terms of programming, "
        "so your approach of ANY coding task would be to break down the problem, "
        "and try your best to solve those problems, and code the entire problem. "
        "In all of these screenshots, piece ALL of the text of the screenshots together "
        "to identify the coding task(s). Now solve the coding task(s) in Python. "
        "Here is your response format:\n"
        "1. Thinking process (By the way, organize the thinking process into points)\n"
        "2. The code/solution\n"
        "3. A detailed explanation\n"
        "4. A summary\n"
        "Be as ACCURATE in your code solution as possible. Also do the bonus challenge(s).\n"
        "Try to achieve ALL of the requirements in the coding task(s), and put in AS MUCH effort AS POSSIBLE\n"
        "Write clean and simple code, and if the problem requires, be creative as well."
    );
}