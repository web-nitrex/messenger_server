#ifndef MANAGERDB_H
#define MANAGERDB_H

#include <QObject>
#include <QtSql>

class ManagerDB : public QObject
{
    Q_OBJECT
public:
    ManagerDB(QObject* obj = nullptr) : QObject(obj){}
    ~ManagerDB();
    bool connectToDataBase();
    bool addUser(const QString& login,const QString& password);
    bool deleteUser(const QString& login);
    bool findUser(const QString& login,const QString& password);
    QStringList getUsersList();
private:
    QSqlDatabase db;
    const QString TYPE_DB = "QSQLITE";
    const QString DATA_BASE_NAME = "database.db";
};

#endif // MANAGERDB_H
