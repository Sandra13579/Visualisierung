#ifndef DATENBANK_H
#define DATENBANK_H

#include <QSqlDatabase>
#include <QObject>

class Datenbank : public QObject
{
    Q_OBJECT
public:
    explicit Datenbank(QString connectionName);
    void Connect();  //Methode zur Herstellung der Datenbankverbindung
    void Disconnect();  //Methode zur Trennung der Datenbankverbindung
    void Exec(QSqlQuery *query);  //Methode zum Lesen und Schreiben auf eine Datenbank
    QSqlDatabase db() const { return _db; } //Übergabe an query!

signals:

private:
    QSqlDatabase _db;    //repräsentiert die tatsächliche Datenbankverbindung
};

#endif // DATENBANK_H
