#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&server,SIGNAL(showMessage(QString)),this,SLOT(displayMessage(QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::displayMessage(QString msg)
{
    ui->textEditInfo->append(msg);
}


void MainWindow::on_pbStart_clicked()
{
    server.startServer(ui->spinBoxPort->value());
}

void MainWindow::on_pbClear_clicked()
{
    ui->textEditInfo->clear();
}

void MainWindow::on_pbStop_clicked()
{
    server.stopServer();
}
