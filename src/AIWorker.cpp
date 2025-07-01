#include "AIWorker.h"
#include "Config.h"
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QEventLoop>

AIWorker::AIWorker(const QList<QPixmap>& pixmaps, const QString& modelChoice, QObject* parent)
    : QObject(parent)
    , m_pixmaps(pixmaps)
    , m_modelChoice(modelChoice)
    , m_networkManager(new QNetworkAccessManager(this)) {
}

void AIWorker::run() {
    QString result;
    
    // Filter valid pixmaps
    QList<QPixmap> validPixmaps;
    for (const QPixmap& pixmap : m_pixmaps) {
        if (!pixmap.isNull()) {
            validPixmaps.append(pixmap);
        }
    }
    
    if (validPixmaps.isEmpty()) {
        result = "Error: No valid screenshots to send.";
    } else {
        if (m_modelChoice == "gemini") {
            result = processWithGemini();
        } else if (m_modelChoice == "gpt") {
            result = processWithOpenAI();
        } else {
            result = QString("Error: Unknown model selection '%1'").arg(m_modelChoice);
        }
    }
    
    emit finished(result);
}

QString AIWorker::processWithGemini() {
    QString apiKey = Config::instance().getGeminiApiKey();
    QString model = Config::instance().getGeminiModel();
    
    if (apiKey == "YOUR_API_KEY_HERE" || apiKey.isEmpty()) {
        return "Error: Gemini API Key not configured. Please set your API key in the configuration.";
    }
    
    // Create request JSON
    QJsonObject requestJson;
    QJsonArray contents;
    
    // Add text prompt
    QJsonObject textPart;
    textPart["text"] = Config::instance().getPrompt();
    
    QJsonArray parts;
    parts.append(textPart);
    
    // Add images
    for (const QPixmap& pixmap : m_pixmaps) {
        if (!pixmap.isNull()) {
            QJsonObject imagePart;
            QJsonObject inlineData;
            inlineData["mime_type"] = "image/png";
            inlineData["data"] = QString(pixmapToBase64(pixmap));
            imagePart["inline_data"] = inlineData;
            parts.append(imagePart);
        }
    }
    
    QJsonObject content;
    content["parts"] = parts;
    contents.append(content);
    requestJson["contents"] = contents;
    
    // Safety settings
    QJsonArray safetySettings;
    QStringList categories = {
        "HARM_CATEGORY_HARASSMENT",
        "HARM_CATEGORY_HATE_SPEECH", 
        "HARM_CATEGORY_SEXUALLY_EXPLICIT",
        "HARM_CATEGORY_DANGEROUS_CONTENT"
    };
    
    for (const QString& category : categories) {
        QJsonObject safety;
        safety["category"] = category;
        safety["threshold"] = "BLOCK_NONE";
        safetySettings.append(safety);
    }
    requestJson["safetySettings"] = safetySettings;
    
    // Make request
    QString url = QString("https://generativelanguage.googleapis.com/v1beta/models/%1:generateContent?key=%2")
                  .arg(model, apiKey);
    
    QNetworkRequest request(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonDocument doc(requestJson);
    QByteArray data = doc.toJson();
    
    QEventLoop loop;
    connect(m_networkManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    loop.exec();
    
    QString result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = responseDoc.object();
        
        QJsonArray candidates = responseObj["candidates"].toArray();
        if (!candidates.isEmpty()) {
            QJsonObject candidate = candidates[0].toObject();
            QJsonObject content = candidate["content"].toObject();
            QJsonArray parts = content["parts"].toArray();
            if (!parts.isEmpty()) {
                QJsonObject part = parts[0].toObject();
                result = part["text"].toString();
            }
        }
        
        if (result.isEmpty()) {
            result = "Error: No response text received from Gemini API.";
        }
    } else {
        result = QString("Error during Gemini API call: %1").arg(reply->errorString());
    }
    
    reply->deleteLater();
    return result;
}

QString AIWorker::processWithOpenAI() {
    QString apiKey = Config::instance().getOpenAiApiKey();
    QString model = Config::instance().getOpenAiModel();
    
    if (apiKey == "YOUR_API_KEY_HERE" || apiKey.isEmpty()) {
        return "Error: OpenAI API Key not configured. Please set your API key in the configuration.";
    }
    
    // Create request JSON
    QJsonObject requestJson;
    requestJson["model"] = model;
    requestJson["max_tokens"] = 4000;
    
    QJsonArray messages;
    QJsonObject message;
    message["role"] = "user";
    
    QJsonArray content;
    
    // Add text prompt
    QJsonObject textContent;
    textContent["type"] = "text";
    textContent["text"] = Config::instance().getPrompt();
    content.append(textContent);
    
    // Add images
    for (const QPixmap& pixmap : m_pixmaps) {
        if (!pixmap.isNull()) {
            QJsonObject imageContent;
            imageContent["type"] = "image_url";
            
            QJsonObject imageUrl;
            QString base64Data = pixmapToBase64(pixmap);
            imageUrl["url"] = QString("data:image/png;base64,%1").arg(base64Data);
            imageContent["image_url"] = imageUrl;
            
            content.append(imageContent);
        }
    }
    
    message["content"] = content;
    messages.append(message);
    requestJson["messages"] = messages;
    
    // Make request
    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    
    QJsonDocument doc(requestJson);
    QByteArray data = doc.toJson();
    
    QEventLoop loop;
    connect(m_networkManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    loop.exec();
    
    QString result;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);
        QJsonObject responseObj = responseDoc.object();
        
        QJsonArray choices = responseObj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            QJsonObject message = choice["message"].toObject();
            result = message["content"].toString();
        }
        
        if (result.isEmpty()) {
            result = "Error: No response content received from OpenAI API.";
        }
    } else {
        result = QString("Error during OpenAI API call: %1").arg(reply->errorString());
    }
    
    reply->deleteLater();
    return result;
}

QByteArray AIWorker::pixmapToBase64(const QPixmap& pixmap) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return byteArray.toBase64();
}

void AIWorker::onNetworkReply() {
    // This slot is currently unused as we use synchronous requests with QEventLoop
    // But it could be used for asynchronous processing in the future
}

#include "AIWorker.moc"