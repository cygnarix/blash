#pragma once

#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>
#include <QJsonDocument>

struct ShortcutConfig {
    QStringList keys;
    QString triggerOn;
};

struct F2ModeConfig {
    QString modifier;
    QMap<QString, QString> actions;
};

struct KeyboardShortcuts {
    ShortcutConfig screenshot;
    F2ModeConfig f2Mode;
};

class Config {
public:
    static Config& instance();
    
    bool loadConfig(const QString& filePath = "kb_sht.json");
    const KeyboardShortcuts& getShortcuts() const { return shortcuts; }
    
    // AI Configuration
    QString getGeminiApiKey() const { return geminiApiKey; }
    QString getGeminiModel() const { return geminiModel; }
    QString getOpenAiApiKey() const { return openAiApiKey; }
    QString getOpenAiModel() const { return openAiModel; }
    QString getSelectedModel() const { return selectedModel; }
    
    void setGeminiApiKey(const QString& key) { geminiApiKey = key; }
    void setGeminiModel(const QString& model) { geminiModel = model; }
    void setOpenAiApiKey(const QString& key) { openAiApiKey = key; }
    void setOpenAiModel(const QString& model) { openAiModel = model; }
    void setSelectedModel(const QString& model) { selectedModel = model; }
    
    QString getPrompt() const;

private:
    Config() = default;
    KeyboardShortcuts shortcuts;
    
    // AI Configuration
    QString geminiApiKey = "YOUR_API_KEY_HERE";
    QString geminiModel = "gemini-2.0-flash";
    QString openAiApiKey = "YOUR_API_KEY_HERE";
    QString openAiModel = "gpt-4o";
    QString selectedModel = "gpt"; // "gpt" or "gemini"
};