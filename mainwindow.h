#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include "vpclient.h"
#include "vpserver.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer* timer;
    QString* oldText;
    bool cursorChanging;
    int oldHStart, oldHEnd;
    VPClient* client;
    VPServer* server;

private slots:
    void on_textEdit_main_cursorPositionChanged();
    void on_fontComboBox_currentFontChanged(QFont f);
    void on_toolButton_Overline_clicked();
    void on_toolButton_Underline_clicked();
    void on_toolButton_Italic_clicked();
    void on_toolButton_Bold_clicked();
    void on_pushButton_client_clicked();
    void on_pushButton_server_clicked();
    void client_connected();
    void client_disconnected();
    void client_text_changed();
    void client_highlight_changed();
    void timer_timeout();
};

#endif // MAINWINDOW_H
