#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "myserver.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    MyServer server;

private slots:
    void displayMessage(QString msg);
    void on_pbStart_clicked();

    void on_pbClear_clicked();
    void on_pbStop_clicked();
};
#endif // MAINWINDOW_H
