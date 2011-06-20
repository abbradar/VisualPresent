#include <QScrollBar>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    this->client = new VPClient(this);
    this->server = new VPServer(this);
    QObject::connect(this->client, SIGNAL(connectedToHost()), this,
                     SLOT(client_connected()));
    QObject::connect(this->client, SIGNAL(disconnectedFromHost()), this,
                     SLOT(client_disconnected()));
    QObject::connect(this->client, SIGNAL(textChanged()), this,
                     SLOT(client_text_changed()));
    QObject::connect(this->client, SIGNAL(highlightChanged()), this,
                     SLOT(client_highlight_changed()));
    this->timer = new QTimer(this);
    QObject::connect(this->timer, SIGNAL(timeout()), this,
                     SLOT(timer_timeout()));
    this->timer->setInterval(1000);
    this->oldText = new QString();
    QPalette textColors = this->ui->textEdit_main->palette();
    textColors.setColor(QPalette::Inactive, QPalette::Highlight, textColors.
                         color(QPalette::Active, QPalette::Highlight));
    textColors.setColor(QPalette::Inactive, QPalette::HighlightedText,
                         textColors.color(QPalette::Active,
                                           QPalette::HighlightedText));
    this->ui->textEdit_main->setPalette(textColors);
    this->ui->fontComboBox->setCurrentFont(this->ui->textEdit_main->font());
}

MainWindow::~MainWindow()
{
    delete this->timer;
    delete this->client;
    delete this->server;
    delete this->oldText;
    delete this->ui;
}

void MainWindow::on_pushButton_server_clicked()
{
    if (this->server->isListening()) {
        this->timer->stop();
        this->server->stop();
        this->ui->textEdit_main->clear();
        this->ui->textEdit_main->setReadOnly(true);
        this->ui->formatButtons->setEnabled(false);
        this->ui->fontComboBox->setCurrentFont(this->ui->
                                               textEdit_main->font());
        this->ui->groupBox_client->setEnabled(true);
        this->ui->pushButton_server->setText(QString::fromUtf8("Включить"));
    }
    else {
        if (!this->server->start(this->ui->spinBox_serverPort->value())) {
            return;
        }
        this->ui->groupBox_client->setEnabled(false);
        this->oldText->clear();
        this->oldHStart = 0;
        this->oldHEnd = 0;
        this->cursorChanging = false;
        this->timer->start();
        this->ui->textEdit_main->setReadOnly(false);
        this->ui->formatButtons->setEnabled(true);
        this->ui->pushButton_server->setText(QString::fromUtf8("Отключить"));
        this->ui->tabWidget->setCurrentWidget(this->ui->tab_text);
    }
}

void MainWindow::on_pushButton_client_clicked()
{
    if (this->client->isConnected()) {
        this->client->disconnectFromHost();
    }
    else {
        this->ui->groupBox_server->setEnabled(false);
        this->ui->pushButton_client->setEnabled(false);
        this->ui->lineEdit_clientHost->setReadOnly(true);
        this->client->connectToHost(
                this->ui->lineEdit_clientHost->text(),
                this->ui->spinBox_clientPort->value());
    }
}

void MainWindow::client_connected()
{
    this->ui->tabWidget->setCurrentWidget(this->ui->tab_text);
    this->ui->pushButton_client->setEnabled(true);
    this->ui->pushButton_client->setText(QString::fromUtf8("Отключиться"));
}

void MainWindow::client_disconnected()
{
    this->ui->tabWidget->setCurrentWidget(this->ui->tab_settings);
    this->ui->textEdit_main->clear();
    this->ui->pushButton_client->setText(QString::fromUtf8("Подключиться"));
    this->ui->pushButton_client->setEnabled(true);
    this->ui->lineEdit_clientHost->setReadOnly(false);
    this->ui->groupBox_server->setEnabled(true);
}

void MainWindow::client_text_changed()
{
    int hPos = this->ui->textEdit_main->horizontalScrollBar()->value();
    int vPos = this->ui->textEdit_main->verticalScrollBar()->value();
    this->ui->textEdit_main->setHtml(this->client->getText());
    QTextCursor current = this->ui->textEdit_main->textCursor();
    current.setPosition(this->client->getHighlightStart(),
                        QTextCursor::MoveAnchor);
    current.setPosition(this->client->getHighlightEnd(),
                        QTextCursor::KeepAnchor);
    this->ui->textEdit_main->setTextCursor(current);
    if (this->ui->checkBox_followCursor->checkState() != Qt::Checked) {
        this->ui->textEdit_main->horizontalScrollBar()->setValue(hPos);
        this->ui->textEdit_main->verticalScrollBar()->setValue(vPos);
    }
}
\
void MainWindow::client_highlight_changed()
{
    int hPos = this->ui->textEdit_main->horizontalScrollBar()->value();
    int vPos = this->ui->textEdit_main->verticalScrollBar()->value();
    QTextCursor current = this->ui->textEdit_main->textCursor();
    current.setPosition(this->client->getHighlightStart(),
                        QTextCursor::MoveAnchor);
    current.setPosition(this->client->getHighlightEnd(),
                        QTextCursor::KeepAnchor);
    this->ui->textEdit_main->setTextCursor(current);
    if (this->ui->checkBox_followCursor->checkState() != Qt::Checked) {
        this->ui->textEdit_main->horizontalScrollBar()->setValue(hPos);
        this->ui->textEdit_main->verticalScrollBar()->setValue(vPos);
    }
}

void MainWindow::timer_timeout()
{
    this->timer->stop();
    QString text = this->ui->textEdit_main->toHtml();
    if (text != *(oldText)) {
        this->server->setText(this->ui->textEdit_main->toHtml());
        *(oldText) = text;
    }
    QTextCursor cursor = this->ui->textEdit_main->textCursor();
    int hStart = cursor.selectionStart();
    int hEnd = cursor.selectionEnd();
    if (hStart != oldHStart || hEnd != oldHEnd) {
        this->server->setHighlight(hStart, hEnd);
        oldHStart = hStart;
        oldHEnd = hEnd;
    }
    this->timer->start();
}

void MainWindow::on_toolButton_Bold_clicked()
{
    QFont font = this->ui->textEdit_main->currentFont();
    font.setBold(!font.bold());
    this->ui->textEdit_main->setCurrentFont(font);
}

void MainWindow::on_toolButton_Italic_clicked()
{
    this->ui->textEdit_main->setFontItalic(!this->ui->textEdit_main->
                                           fontItalic());
}

void MainWindow::on_toolButton_Underline_clicked()
{
    this->ui->textEdit_main->setFontUnderline(!this->ui->textEdit_main->
                                           fontUnderline());
}

void MainWindow::on_toolButton_Overline_clicked()
{
    QFont font = this->ui->textEdit_main->currentFont();
    font.setOverline(!font.overline());
    this->ui->textEdit_main->setCurrentFont(font);
}

void MainWindow::on_fontComboBox_currentFontChanged(QFont f)
{
    if (!this->cursorChanging) {
        this->ui->textEdit_main->setFontFamily(f.family());
    }
}

void MainWindow::on_textEdit_main_cursorPositionChanged()
{
    QFont font = this->ui->textEdit_main->currentFont();
    this->ui->toolButton_Bold->setChecked(font.bold());
    this->ui->toolButton_Italic->setChecked(font.italic());
    this->ui->toolButton_Underline->setChecked(font.underline());
    this->ui->toolButton_Overline->setChecked(font.overline());
    this->cursorChanging = true;
    this->ui->fontComboBox->setCurrentFont(this->ui->textEdit_main->
                                           fontFamily());
    this->cursorChanging = false;
}
