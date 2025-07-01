#pragma once

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class ResponseDialog : public QDialog {
    Q_OBJECT

public:
    explicit ResponseDialog(QWidget* parent = nullptr);

    void setResponseText(const QString& text);

signals:
    void closed();

protected:
    void paintEvent(QPaintEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onCopyClicked();

private:
    void setupUI();
    void applyCaptureProtection();

    QTextEdit* m_textEdit;
    QPushButton* m_copyButton;
    QPushButton* m_closeButton;
    QLabel* m_titleLabel;
};