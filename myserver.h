#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QByteArray>
#include <QMap>
#include "managerdb.h"

//типы передаваемых по сети сообщений
struct  TypeMessage
{
    static const quint16 Text =1;
    static const quint16 File=2;
    static const quint16 AuthRequest=3;
    static const quint16 AuthAnswer=4;
    static const quint16 RegistrationRequest=5;
    static const quint16 UsersList=6;
};

//структура для упаковки сообщения чата
struct Message{
    QString dateTime;
    QString nickname;
    QString message;
};

class MyServer : public QObject
{
    Q_OBJECT
public:
    MyServer(QObject* obj = nullptr): QObject(obj), tcpServer(nullptr), nextBlockSize(0){}
    void startServer(int port);
    void stopServer();

private:
    QTcpServer* tcpServer;
    quint64 nextBlockSize;					//размер блока принимаемых данных
    QMap<QString,QTcpSocket*> usersOnline;  //список пользователей, подключенных к серверу и их сокеты
    ManagerDB usersDB;						//менеджер для работы с базой данных

    void sendMessageToClient(QTcpSocket* socket, const Message& msg);
    void sendFileToClient(QTcpSocket *socket, const QString &fileName,const QString &from , const QByteArray &file);
    void sendAuthorizationState(QTcpSocket *socket,const QString& authMsg);
    void updateUsersList();
    void sendUsersList(QTcpSocket *socket);
    bool userIsConnected(const QString& login);

public slots:
    void slotNewConnection();
    void slotReadClient();

private slots:
    void slotDisconnected();

signals:
    void showMessage(QString msg); //отображение сообщения в главном окне сервера
};


#endif // MYSERVER_H
