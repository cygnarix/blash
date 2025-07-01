#pragma once

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

class DialogBox : public QDialog {
    Q_OBJECT

public:
    explicit DialogBox(QWidget* parent = nullptr);

signals:
    void screenshotRequested();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void setupUI();
    void applyCaptureProtection();

    QLabel* m_messageLabel;
    QPushButton* m_yesButton;
    QPushButton* m_noButton;
    QPushButton* m_screenshotButton;
};