#ifndef DATENBANK_H
#define DATENBANK_H

#include <QSqlDatabase>
#include <QObject>

class Datenbank : public QObject
{
    Q_OBJECT
public:
    explicit Datenbank(QObject *parent = nullptr);
    void Connect();  //Methode zur Herstellung der Datenbankverbindung
    void Disconnect();  //Methode zur Trennung der Datenbankverbindung
    void Exec(QSqlQuery *query);  //Methode zum Lesen und Schreiben auf eine Datenbank

signals:

private:
    QSqlDatabase db;    //repräsentiert die tatsächliche Datenbankverbindung
};

#endif // DATENBANK_H
