#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtSql>
#include <QDebug>

#include "datenbank.h"

//erstellt eine neue ODBC Datenbankverbindung
Datenbank::Datenbank(QObject *parent)
    : QObject{parent}
{
    db = QSqlDatabase::addDatabase("QODBC");
}

//Verbindung zu Datenbank aufbauen
void Datenbank::Connect()
{
    QString connectString = QStringLiteral(
        "DRIVER={MySQL ODBC 8.0 Unicode Driver};"
        "SERVERNODE=127.0.0.1:3306;"
        "UID=root;"
        "PWD=vpj;");
    db.setDatabaseName(connectString);

    //konnte die Verbindung aufgebaut werden?
    if(db.open())
    {
        qDebug() << "ok";
    }
    else
    {
        qDebug() << db.lastError().text();
    }
}

//Verbindung schließen/trennen
void Datenbank::Disconnect()
{
    if(db.open())
    {
        db.close();
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
