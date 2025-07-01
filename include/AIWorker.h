#pragma once

#include <QObject>
#include <QPixmap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QThread>

class AIWorker : public QObject {
    Q_OBJECT

public:
    explicit AIWorker(const QList<QPixmap>& pixmaps, const QString& modelChoice, QObject* parent = nullptr);

signals:
    void finished(const QString& result);

public slots:
    void run();

private slots:
    void onNetworkReply();

private:
    QString processWithGemini();
    QString processWithOpenAI();
    QByteArray pixmapToBase64(const QPixmap& pixmap);
    QString createGeminiRequest();
    QString createOpenAIRequest();

    QList<QPixmap> m_pixmaps;
    QString m_modelChoice;
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply = nullptr;
};