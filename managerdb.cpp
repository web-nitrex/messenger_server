#include "managerdb.h"

ManagerDB::~ManagerDB()
{
    if(db.open())
        db.close();
}


//подключение к базе данных
bool ManagerDB::connectToDataBase()
{
    db = QSqlDatabase::addDatabase(TYPE_DB);
    db.setDatabaseName(DATA_BASE_NAME);

    if(!db.open())
        return false;
    else
        return true;
}

//добавление нового пользователя в базу данных
bool ManagerDB::addUser(const QString &login, const QString &password)
{
    QSqlQuery query;

    QString strF = "INSERT INTO users(login,password)"
                          "VALUES('%1','%2');";

    QString str = strF.arg(login)
              .arg(password);

    //поле "логин" в БД создано как уникальное, поэтому при попытке добавить
    //пользователя с уже имеющимся в БД логином, вернется false
    if(!query.exec(str))
        return false;
    else
        return true;
}

//поиск пользователя в базе данных
bool ManagerDB::findUser(const QString &login, const QString &password)
{
    QSqlQuery query;

    QString str = "SELECT * FROM users WHERE login='"+login+"'"+" AND password='"+password+"'";

    if(!query.exec(str))
    {
        return false;
    }

    QSqlRecord rec = query.record();

    while(query.next())
    {
       QString loginFromDB=query.value(rec.indexOf("login")).toString();
       QString passwordFromDB=query.value(rec.indexOf("password")).toString();

       if(loginFromDB!="" && passwordFromDB!="")
           return true;
    }

    return false;
}

//получение списка пользователей, зарегистрированных на сервере
QStringList ManagerDB::getUsersList()
{
    QSqlQuery query;
    QStringList users;

    QString str = "SELECT * FROM users";

    if(!query.exec(str))
    {
        return users;
    }

    QSqlRecord rec = query.record();

    while(query.next())
    {
       users.push_back(query.value(rec.indexOf("login")).toString());
    }

    return users;
}
