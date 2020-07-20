#include "myserver.h"
#include <QDataStream>
#include <QDateTime>

using namespace std;

//перегружаем операторы выввода и ввода в поток, для записи туда структуры сообщения
QDataStream& operator <<(QDataStream& out, const Message& msg)
{
    out << msg.dateTime;
    out << msg.nickname;
    out << msg.message;
    return out;
}

QDataStream& operator >>(QDataStream& in, Message& msg)
{
    in >> msg.dateTime;
    in >> msg.nickname;
    in >> msg.message;
    return in;
}

//запуск сервера
void MyServer::startServer(int port)
{
    tcpServer = new QTcpServer(this);

    //запускаем сервер на прослушку заданного порта
    if(!tcpServer->listen(QHostAddress::Any,port))
    {
        emit showMessage("Server Error! Unable to start the server: " + tcpServer->errorString());
        tcpServer->close();
        return;
    }

    //соединяем сигнал и слот сервера для обработки новых подключений
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));

    //если не удается подключиться к базе данных то останавливаем сервер
    if(!usersDB.connectToDataBase())
    {
        stopServer();
        emit showMessage("<font color=\"red\">Error connect to Data BASE!</font>");
        return;
    }
    else
    {
        emit showMessage("<font color=\"green\">Server is started!</font>");

        //получаем список зарегистрированных пользователей
        QStringList usersList = usersDB.getUsersList();

        if(usersList.size()>0)
        {
            emit showMessage("Список пользователей:");

            for(int i=0; i<usersList.size(); ++i)
            {
                emit showMessage(QString::number(i+1)+". "+usersList[i]+"\n");
            }
        }
    }
}


//остановка сервера
void MyServer::stopServer()
{
    tcpServer->close();

    //отключаем все подключенные соекты и очищаем список сокетов
    for(auto socket : usersOnline.values())
    {
      socket->close();
    }

    usersOnline.clear();

    emit showMessage("<font color=\"red\">The server is stopped!</font>");
}

//отправка файла клиенту
void MyServer::sendFileToClient(QTcpSocket *socket, const QString &fileName,const QString &from , const QByteArray &file)
{
    QByteArray block;
    QDataStream stream(&block, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_3);

    //записываем в поток размер данных, тип сообщения, отправителя файла, имя файла и его содержимое
    stream << quint64(0);
    stream << TypeMessage::File;
    stream << from;
    stream <<fileName;
    stream << file;

    //сдвигаемся к началу потока, для того что бы указать размер передаваемых данных
    stream.device()->seek(0);
    stream << quint64(block.size()-sizeof(quint64));

    //отправляем данные
    socket->write(block);
    socket->flush();
}

//отправка клиенту состояния авторизации
void MyServer::sendAuthorizationState(QTcpSocket *socket,const QString& authMsg)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    //отправляем клиенту статус авторизации и сервисное сообщение
    out << quint64(0)<<TypeMessage::AuthAnswer<<authMsg;

    //сдвигаемся к началу потока, для того что бы указать размер передаваемых данных
    out.device()->seek(0);
    out << quint64(arrBlock.size()-sizeof(quint64));

    //записываем данные в сокет
    socket->write(arrBlock);
    socket->flush();
}

//отправка клиенту списка подключенных пользователей
void MyServer::sendUsersList(QTcpSocket *socket)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    out << quint64(0)<<TypeMessage::UsersList;

    QStringList users;

    //получаем из ключей мэпа список пользователей
    for(const auto& user : usersOnline.keys())
        users.push_back(user);

    //записываем список пользователей в поток
    out << users;

    //сдвигаемся к началу потока, для того что бы указать размер передаваемых данных
    out.device()->seek(0);
    out << quint64(arrBlock.size()-sizeof(quint64));

    //записываем данные в сокет
    socket->write(arrBlock);
    socket->flush();
}

//проверка пользователя на то что он сейчас подключен к серверу (онлайн)
bool MyServer::userIsConnected(const QString &login)
{
    if(usersOnline.find(login)==usersOnline.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

//обновление списка пользователей у клиентов
void MyServer::updateUsersList()
{
    //отсылаем всем подключенным пользователям списки подключенных к серверу пользователей
    for(auto clientSocket : usersOnline.values())
    {
       sendUsersList(clientSocket);
    }
}

//отправление текстового сообщения клиенту
void MyServer::sendMessageToClient(QTcpSocket *socket, const Message &msg)
{
    QByteArray arrBlock;
    QDataStream out(&arrBlock,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_3);

    //записываем в поток размер передаваемых данных, тип сообщения и само сообщение
    out << quint64(0)<<TypeMessage::Text<<msg;

    //сдвигаемся к началу потока, для того что бы указать размер передаваемых данных
    out.device()->seek(0);
    out << quint64(arrBlock.size()-sizeof(quint64));

    //записываем данные в сокет
    socket->write(arrBlock);
    socket->flush();
}

//обработчик нового соединения
void MyServer::slotNewConnection()
{
    //получаем сокет нового соединения
    QTcpSocket* clientSocket = tcpServer->nextPendingConnection();

    //соединяем сигналы и слоты отключения сокета и готовности к чтению данных
    connect(clientSocket,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(slotReadClient()));

    emit showMessage("Connected"+clientSocket->peerAddress().toString()+":"+QString::number(clientSocket->peerPort()));
}

//чтение данных от клиента
void MyServer::slotReadClient()
{
    //получаем сокет, от которого принимаем данные
    QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());
    QDataStream in(clientSocket);

    in.setVersion(QDataStream::Qt_5_3);

    for(;;)
    {
        //проверяем что это первый прием данных и до этого ничего не принимали
        if(!nextBlockSize)
        {
            //если количество входящих байт, которые ожидают считывания меньше чем размер sizeof переменной
            //в которой мы указываем размер блока данных то выходим из цикла
            if(clientSocket->bytesAvailable() < sizeof (quint64))
            {
                break;
            }

            //если все в порядке, то считываем общее количество принимаемых данных
            in >> nextBlockSize;
        }

        //если нет больше данных то прекращаем чтение
        if(clientSocket->bytesAvailable() < nextBlockSize)
        {
            break;
        }

        quint16 typeMsg;//тип принятого сообщения

        //считываем тип принятого сообщения
        in >> typeMsg;

        if(typeMsg == TypeMessage::Text) //принимаем текстовое сообщение
        {
            Message msg;
            QString recipient;

            in >> recipient;

            //проверяем что пользователь в сети
            if(usersOnline.find(recipient)==usersOnline.end())
                return;

            //записываем принятые данные в строку
            in >> msg;

            //пересылаем сообщение адресату
            sendMessageToClient(usersOnline[recipient], msg);

        }
        else if (typeMsg == TypeMessage::File) //принимаем файл
        {
            QString fileName;
            QByteArray arrFile;

            QString recipient,from;
            in >> recipient;
            in >> from;

            //проверяем что пользователь в сети
            if(usersOnline.find(recipient)==usersOnline.end())
                return;

            //считываем имя файла и его содержимое из потока
            in >> fileName;
            in >> arrFile;

            //отправляем файл адресату
            sendFileToClient(usersOnline[recipient], fileName,from, arrFile);

        }
        else if (typeMsg == TypeMessage::AuthRequest) //принимаем запрос авторизации
        {
            QString login, password;

            //считываем имя пользователя и пароль
            in >> login;
            in >> password;

            //проверяем что такой пользователь есть в списке зарегистрированных и он в данный момент не подключен к серверу
            if(!usersDB.findUser(login,password) || userIsConnected(login))
            {
                //авторизация не прошла
                //тут мы сначала должны отправить сообщение о том что авторизация не удалась
                sendAuthorizationState(clientSocket,"Authorization faild!");

                //закрываем сокет
                clientSocket->close();
            }
            else
            {
                //авторизация прошла успешно
                //отправляем сообщение об успешной авторизации
                sendAuthorizationState(clientSocket,"Authorization was succesful!");

                //связываем логин с сокетом
                usersOnline[login]=clientSocket;

                emit showMessage("Connected clients: "+QString::number(usersOnline.size()));

                //обновляем список пользователей у клиентов
                updateUsersList();

                emit showMessage("Connected client: "+login);

            }
        }
        else if (typeMsg == TypeMessage::RegistrationRequest) //принимаем запрос на регистрацию
        {
            QString login, password;

            //считываем имя пользователя и пароль
            in >> login;
            in >> password;

            //пытаемся добавить пользователя в базу данных
            if(!usersDB.addUser(login,password))
            {
                //регистрация не прошла
                //тут мы сначала должны отправить сообщение о том что регистрация не удалась
                sendAuthorizationState(clientSocket,"Registration faild!");

                //закрываем сокет
                clientSocket->close();
            }
            else
            {
                //регистрация прошла успешно
                //отправляем сообщение об успешной регистрации
                sendAuthorizationState(clientSocket,"Registration was succesful!");

                //связываем логин с сокетом
                usersOnline[login]=clientSocket;

                emit showMessage("Connected clients: "+QString::number(usersOnline.size()));

                //обновляем список пользователей у клиентов
                updateUsersList();

                emit showMessage("Registred client: "+login);

            }
        }

        //показываем что данных больше нет, что бы выйти из слота чтения данных
        nextBlockSize=0;

    }
}

//отключения клиента от сервера
void MyServer::slotDisconnected()
{
    //получаем сокет, котрый отключается от сервера
    QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());

    //удаляем пользователя из списка online
    for(auto& user : usersOnline.toStdMap())
    {
        if(user.second==clientSocket)
        {
           usersOnline.remove(user.first);
        }
    }

    //обновляем список пользователей у клиентов
    updateUsersList();

    emit showMessage("Disconnected from"+clientSocket->peerAddress().toString()+":"+QString::number(clientSocket->peerPort()));
    emit showMessage("Connected clients: "+QString::number(usersOnline.size()));

    //удаляем сокет
    clientSocket->deleteLater();
}
