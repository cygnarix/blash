#pragma once

#include <QWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QThread>
#include <memory>

class StealthToolbar;
class AIWorker;
class ResponseDialog;

class ScreenshotViewer : public QWidget {
    Q_OBJECT

public:
    explicit ScreenshotViewer(const QList<QPixmap>& pixmaps, StealthToolbar* parent = nullptr, const QString& modelChoice = "gpt");

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onConfirm();
    void onRestart();
    void onResult(const QString& text);
    void onResponseClosed();

private:
    void setupUI();
    void setupButtons();
    void applyCaptureProtection();

    StealthToolbar* m_toolbar;
    QList<QPixmap> m_pixmaps;
    QString m_modelSelection;
    
    QScrollArea* m_scrollArea;
    QLabel* m_loadingLabel;
    QPushButton* m_closeButton;
    QPushButton* m_confirmButton;
    QPushButton* m_restartButton;
    
    std::unique_ptr<QThread> m_thread;
    std::unique_ptr<AIWorker> m_worker;
    std::unique_ptr<ResponseDialog> m_responseDialog;
};