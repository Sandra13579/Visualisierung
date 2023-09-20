#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtSql>
#include <QDebug>

#include "datenbank.h"

//erstellt eine neue ODBC Datenbankverbindung
Datenbank::Datenbank(QString connectionName)
{
    _db = QSqlDatabase::addDatabase("QODBC", connectionName);
}

//Verbindung zu Datenbank aufbauen
void Datenbank::Connect()
{
    QString connectString = QStringLiteral(
        "DRIVER={MySQL ODBC 8.0 Unicode Driver};"
        "SERVERNODE=127.0.0.1:3306;"
        "UID=root;"
        "PWD=vpj;");
    _db.setDatabaseName(connectString);

    //konnte die Verbindung aufgebaut werden?
    if(_db.open())
    {
        qDebug() << _db.connectionName() << "connected to database!";
    }
    else
    {
        qDebug() << _db.lastError().text();
    }
}

//Verbindung schließen/trennen
void Datenbank::Disconnect()
{
    if(_db.open())
    {
        _db.close();
        qDebug() << _db.connectionName() << "disconnected from database!";
    }
}


//Datenbank Kommando ausführen = schreibend und lesend auf eine Datenbank zugreifen
void Datenbank::Exec(QSqlQuery *query)
{
    if (!query->exec())
    {
        qWarning() << "Failed to execute query";  //falls ein Fehler aufgetreten ist
    }
}
