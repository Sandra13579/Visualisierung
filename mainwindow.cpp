#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSqlQuery>
#include <QSqlIndex>
#include <QDateTime>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    database = new Datenbank(this);
    database->Connect();

//Auftragsverwaltung
    //Produktauswahl hinterlegen
    QSqlQuery query;
    query.prepare("SELECT `production_process_name` FROM `vpj`.`production_process`;");
    database->Exec(&query);
    int i = 0;
    while (query.next())
    {
        ui->comboBox->insertItem(i++, query.record().value(0).toString());
    }

//Werkstückverwaltung
    //Auswahl eingeliefert/ausgeliefert
    ui->comboBox_2->insertItems(0, { "angeliefert", "ausgeliefert"});
    ui->comboBox_4->insertItems(0, { "2", "3"});

//Wartungsverwaltung
    ui->comboBox_5->insertItems(0, { "Roboter", "Station"});

//Aufträge
    query.prepare("SELECT order_name FROM vpj.production_order;");
    database->Exec(&query);
    i = 0;
    while (query.next())
    {
        ui->production_order_name_comboBox->insertItem(i++, query.record().value(0).toString());
    }
}

MainWindow::~MainWindow()
{
    database->Disconnect();
    delete ui;
}

//Auftragsverwaltung
void MainWindow::on_pushButton_clicked()
{
    //Auftragsname auslesen
    QString production_order_name = ui->lineEdit->text();
    qDebug() << "Auftrag" << production_order_name;
    ui->lineEdit->clear();
    //Produktauswahl auslesen
    int production_process_id = ui->comboBox->currentIndex() + 1;
    qDebug() << "Pauftrags_id" << production_process_id;
    //Stückzahl auslesen
    int number_of_pieces = ui->spinBox->value();
    qDebug() << "Stückzahl" << number_of_pieces;
    ui->spinBox->setValue(0);
    if (production_order_name == "" || number_of_pieces == 0)
    {
        QMessageBox order_msgBox;
        order_msgBox.setText("Sie haben nicht alle Auftragseingaben getätigt.");
        order_msgBox.exec();
        qDebug() << "es wurde Eingabe vergessen";
    }
    else
    {
        QDateTime currentTime = QDateTime::currentDateTime(); //aktueller Zeitstempel

        QSqlQuery query;
        query.prepare("INSERT INTO vpj.production_order (order_name,  number_of_pieces, timestamp, workpiece_state_id, production_process_id) VALUES (:order_name, :pieces, :timestamp, :state_id, :process_id)");
        query.bindValue(":order_name", production_order_name);
        query.bindValue(":pieces", number_of_pieces);
        query.bindValue(":timestamp", currentTime);
        query.bindValue(":state_id", 5);
        query.bindValue(":process_id", production_process_id);
        database->Exec(&query);
        //Aufträge (siehe linke Seite Tab)
        ui->production_order_name_comboBox->clear();
        query.prepare("SELECT order_name FROM vpj.production_order;");
        database->Exec(&query);
        int i = 0;
        while (query.next())
        {
            ui->production_order_name_comboBox->insertItem(i++, query.record().value(0).toString());
        }
    }

}

//Werkstückverwaltung
void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    ui->comboBox_3->clear();
    if (ui->comboBox_2->currentIndex() == 0) //Wenn angeliefert gewählt wurde
    {
        ui->comboBox_3->insertItems(0, { "Rohstoffteillager 1", "Rohstoffteillager 2"});
    }
    else
    {
        ui->comboBox_3->insertItems(0, { "Fertigteillager 1", "Fertigteillager 2"});
    }
}

void MainWindow::on_workpiece_pushButton_clicked()
{
    //an-/ ausgeliefert lesen
    int check_in_out = ui->comboBox_2->currentIndex();
    qDebug() << "an/ausgeliefert" << check_in_out;
    //RFID auslesen
    QString rfid = ui->lineEdit_2->text();
    qDebug() << "RFID" << rfid;
    ui->lineEdit_2->clear();
    //Stationsauswahl auslesen
    int station = ui->comboBox_3->currentIndex() + 1;
    qDebug() << "Station" << station;
    //Platzauswahl auslesen
    int place = ui->comboBox_4->currentIndex() + 2;
    qDebug() << "Platz" << place;

    if (rfid == "")
    {
        QMessageBox order_msgBox;
        order_msgBox.setText("Sie haben nicht alle Eingaben getätigt.");
            order_msgBox.exec();
    }
    else
    {
        if (check_in_out == 1) //ausgeliefert
        {
            station += 6;
        }
        rfid = tr("%1").arg(rfid);
        QDateTime currentTime = QDateTime::currentDateTime(); //aktueller Zeitstempel
        QSqlQuery query;
        query.prepare("SELECT station_place_id FROM vpj.station_place WHERE station_id = :station_id AND place_id = :place_id");
        query.bindValue(":station_id", station);
        query.bindValue(":place_id", place);
        database->Exec(&query);
        int station_place_id = 0;
        query.next();
        station_place_id = query.record().value(0).toInt();
        if (check_in_out == 0) //angeliefert
        {
        query.prepare("INSERT INTO vpj.workpiece (rfid,  checked_in, timestamp, workpiece_state_id, station_place_id) VALUES (:rfid, :checked_in, :timestamp, :state_id, :place_id)");
        query.bindValue(":rfid", rfid);
        query.bindValue(":checked_in", 1);
        query.bindValue(":timestamp", currentTime);
        query.bindValue(":state_id", 1);
        query.bindValue(":place_id", station_place_id);
        database->Exec(&query);

        query.prepare("UPDATE vpj.station_place SET state_id = :state_id WHERE station_place_id = :place_id");
        query.bindValue(":state_id", 1);
        query.bindValue(":place_id", station_place_id);
        database->Exec(&query);
        }
        else if (check_in_out == 1) //ausgeliefert
        {
        QSqlQuery query;
        query.prepare("SELECT rfid FROM vpj.workpiece INNER JOIN vpj.station_place ON station_place.station_place_id = workpiece.station_place_id WHERE workpiece.rfid = :rfid AND workpiece.workpiece_state_id = 0 AND workpiece.station_place_id = :station_place_id;");
        query.bindValue(":rfid", rfid);
        query.bindValue(":station_place_id", station_place_id);
        database->Exec(&query);
        if (query.next())
        {
            query.prepare("UPDATE vpj.station_place SET state_id = 0 WHERE station_place_id = :place_id");
            query.bindValue(":place_id", station_place_id);
            database->Exec(&query);
            query.prepare("UPDATE vpj.workpiece SET checked_in = 0, workpiece_state_id = 4, station_place_id = 0, timestamp = :timestamp WHERE station_place_id = :place_id");
            query.bindValue(":timestamp", currentTime);
            query.bindValue(":place_id", station_place_id);
            database->Exec(&query);
        }
        }
    }
}

//Wartungsverwaltung
void MainWindow::on_comboBox_5_currentIndexChanged(int index) //Auswahl Roboter oder Station
{
    QSqlQuery query;
    if (ui->comboBox_5->currentIndex() == 0) //Wenn Roboter gewählt wurde
    {
        ui->comboBox_6->clear();    //vorherige Items löschen, damit die nächsten nicht angehängt werden
        //Welche Roboter gibt es in DB?
        query.prepare("SELECT robot_id FROM vpj.robot ORDER BY robot_id ASC;");
        database->Exec(&query);
        int i = 0;
        while (query.next())
        {
            ui->comboBox_6->insertItem(i++, query.record().value(0).toString());  //Roboter_id in Auswahlfeld schreiben
        }
    }
    else //Wenn Station gewählt wurde
    {
        ui->comboBox_6->clear();
        query.prepare("SELECT station_name FROM vpj.station WHERE station_id < 9 ORDER BY station_id ASC;;");
        database->Exec(&query);
        int i = 0;
        while (query.next())
        {
            ui->comboBox_6->insertItem(i++, query.record().value(0).toString());  //Roboter_id in Auswahlfeld schreiben
        }
        ui->comboBox_6->insertItem(i++, "Ladestation 1");
        ui->comboBox_6->insertItem(i++, "Ladestation 2");
    }
}


void MainWindow::on_comboBox_6_currentIndexChanged(int index)
{
    QSqlQuery query;
    if (ui->comboBox_5->currentIndex() == 0) //Wenn Roboter gewählt wurde
    {
        ui->pushButton_3->setText("Roboter in Wartung schicken");
        query.prepare("SELECT `robot_id` FROM `vpj`.`robot` WHERE state_id = 3;");
        database->Exec(&query);
        int robot_id_state3;
        while (query.next())
        {
            robot_id_state3=query.record().value(0).toInt();
            if (ui->comboBox_6->currentIndex()+1 == robot_id_state3) //wenn ausgewählter Roboter bereits in Wartung ist
            {
                ui->pushButton_3->setText("Roboter in Betrieb nehmen");
            }
        }
    }
    else // wenn Station gewählt wurde
    {
        ui->pushButton_3->setText("Station in Wartung schicken");
        int station_id_selected = ui->comboBox_6->currentIndex()+1;
        qDebug() << "station_id_selected = " << station_id_selected;
        //Ladestation
        if (station_id_selected > 8)
        {
            query.prepare("SELECT maintenance FROM `vpj`.`station_place` WHERE station_id =9 AND place_id = :place_id_selected ");
            query.bindValue(":place_id_selected", station_id_selected - 8);
            database->Exec(&query);
            query.next();
            qDebug() << "Ladestation " << query.record().value(0).toInt();
            if (query.record().value(0).toInt() == 1) //wenn ausgewählte Station bereits in Wartung ist
            {
                ui->pushButton_3->setText("Station in Betrieb nehmen");
            }
        }
        else
        {
        //Bearbeitungsstationen (1-8)
        query.prepare("SELECT maintenance FROM `vpj`.`station` WHERE station_id = :station_id_selected ;");
        query.bindValue(":station_id_selected", station_id_selected);
        database->Exec(&query);
        query.next();
        qDebug() << "in Maintenance = " << query.record().value(0).toInt();
        if (query.record().value(0).toInt() == 1) //wenn ausgewählte Station bereits in Wartung ist
        {
            ui->pushButton_3->setText("Station in Betrieb nehmen");
        }
        }
    }
}


void MainWindow::on_pushButton_3_clicked() //in Wartung schicken, aus Wartung rausholen
{
    QDateTime currentTime = QDateTime::currentDateTime(); //aktueller Zeitstempel
    QSqlQuery query;

    if ( ui->pushButton_3->text() == "Roboter in Wartung schicken")
    {
        query.prepare("UPDATE vpj.robot SET maintenance = 1 WHERE robot_id = :robot_id");
        query.bindValue(":robot_id", ui->comboBox_6->currentIndex()+1);
        database->Exec(&query);
    }
    if ( ui->pushButton_3->text() == "Roboter in Betrieb nehmen")
    {
        query.prepare("UPDATE vpj.robot SET maintenance = 0, timestamp = :timestamp, state_id = 0, jobtype_id = 3 WHERE robot_id = :robot_id");
        query.bindValue(":robot_id", ui->comboBox_6->currentIndex()+1);
        query.bindValue(":timestamp", currentTime);
        database->Exec(&query);
    }

    if ( ui->pushButton_3->text() == "Station in Wartung schicken")
    {
        int station_id = ui->comboBox_6->currentIndex()+1;

        if (station_id > 8) //Ladestationen
        {
        query.prepare("SELECT station_place_id FROM `vpj`.`station_place` WHERE station_id = 9 AND place_id = :place_id ");
        query.bindValue(":place_id", station_id - 8);
        database->Exec(&query);
        query.next();
        int station_place_id = query.record().value(0).toInt();
        query.prepare("UPDATE vpj.station_place SET maintenance = 1, state_id = 3 WHERE station_place_id = :station_place_id AND state_id = 0;");
        query.bindValue(":station_place_id", station_place_id);
        database->Exec(&query);
        }

        if (station_id < 8) //Bearbeitungsstation gewählt
        {
        query.prepare("UPDATE vpj.station SET maintenance = 1 WHERE station_id = :station_id");
        query.bindValue(":station_id", station_id);
        database->Exec(&query);

        query.prepare("UPDATE vpj.station_place SET state_id = 3 WHERE station_id = :station_id AND state_id = 0");
        query.bindValue(":station_id", station_id);
        database->Exec(&query);
        }
    }
    if ( ui->pushButton_3->text() == "Station in Betrieb nehmen")
    {
        int station_id = ui->comboBox_6->currentIndex()+1;

        if (station_id > 8) //Ladestationen
        {
        query.prepare("SELECT station_place_id FROM `vpj`.`station_place` WHERE station_id = 9 AND place_id = :place_id ");
        query.bindValue(":place_id", station_id - 8);
        database->Exec(&query);
        query.next();
        int station_place_id = query.record().value(0).toInt();
        query.prepare("UPDATE vpj.station_place SET maintenance = 0, state_id = 0 WHERE station_place_id = :station_place_id AND state_id = 3;");
        query.bindValue(":station_place_id", station_place_id);
        database->Exec(&query);
        }

        if (station_id < 8) //Bearbeitungsstation gewählt
        {
        query.prepare("UPDATE vpj.station SET maintenance = 0 WHERE station_id = :station_id");
        query.bindValue(":station_id", station_id);
        database->Exec(&query);

        query.prepare("UPDATE vpj.station_place SET state_id = 0 WHERE station_id = :station_id AND state_id = 3");
        query.bindValue(":station_id", station_id);
        database->Exec(&query);
        }
    }
}

//Aufträge
void MainWindow::on_production_order_name_comboBox_currentIndexChanged(int index)
{
    workpiece_table(index +1);
}

void MainWindow::workpiece_table(int index)
{
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Fertigungsschritt", "       Dauer     ", "       Stückzahl      "}); // Setze die Spaltenüberschriften

    // Füge die Zeilen mit den Namen hinzu
    QList<QString> fertigungsschritte = {"unbearbeitet", "sägen", "schleifen", "lackieren", "polstern", "fertig produziert", "ausgeliefert"};
    int rowCount = fertigungsschritte.size();
    model->setRowCount(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        QString fertigungsschritt = fertigungsschritte.at(row);
        QString duration, counts;

        // Füge den Namen des Fertigungsschritts in die erste Spalte ein
        QStandardItem *fertigungsschrittItem = new QStandardItem(fertigungsschritt);
        model->setItem(row, 0, fertigungsschrittItem);

        //Schrittdauer
        QSqlQuery query;
        query.prepare("SELECT step_duration FROM vpj.production_step INNER JOIN vpj.production_order ON production_order.production_process_id = production_step.production_process_id WHERE production_order.production_order_id = :order_id AND production_step.step_id = :step_id;");
        query.bindValue(":order_id", index);
        query.bindValue(":step_id", row);
        database->Exec(&query);
        if (query.next() && row != 5)
        {
            duration = tr("%1").arg(query.record().value(0).toInt());
            qDebug() << "in Duration = " << query.record().value(0).toInt();
            QStandardItem *durationItem = new QStandardItem(duration);
            model->setItem(row, 1, durationItem);

            //Stückzahl
            query.prepare("SELECT COUNT(*) FROM vpj.workpiece WHERE production_order_id = :order_id AND step_id = :step_id;");
            query.bindValue(":order_id", index);
            query.bindValue(":step_id", row);
            database->Exec(&query);
            if (query.next())
            {
            counts = tr("%1").arg(query.record().value(0).toInt());
            qDebug() << "in counts = " << query.record().value(0).toInt();
            QStandardItem *countsItem = new QStandardItem(counts);
            model->setItem(row, 2, countsItem);
            }
        }
        else
        {
            QStandardItem *emptyItem = new QStandardItem();
            model->setItem(row, 1, emptyItem);
            model->setItem(row, 2, emptyItem);
        }
        if (row == 5) //Stückzahl, fertig produziert
        {
            query.prepare("SELECT COUNT(*) FROM vpj.workpiece WHERE production_order_id = :order_id AND workpiece_state_id = 0;");
            query.bindValue(":order_id", index);
            database->Exec(&query);
            query.next();
            counts = tr("%1").arg(query.record().value(0).toInt());
            qDebug() << "in counts = " << query.record().value(0).toInt();
            QStandardItem *countsItem = new QStandardItem(counts);
            model->setItem(row, 2, countsItem);
        }
        if (row == 6) //Stückzahl, fertig produziert
        {
            query.prepare("SELECT COUNT(*) FROM vpj.workpiece WHERE production_order_id = :order_id AND workpiece_state_id = 4;");
            query.bindValue(":order_id", index);
            database->Exec(&query);
            query.next();
            counts = tr("%1").arg(query.record().value(0).toInt());
            qDebug() << "in counts = " << query.record().value(0).toInt();
            QStandardItem *countsItem = new QStandardItem(counts);
            model->setItem(row, 2, countsItem);
        }

    }
    ui->workpiece_tableView->setModel(model); // erstellt die Tabelle

    // Optional: Größe der Spalten automatisch an den Inhalt anpassen
    //ui->workpiece_tableView->resizeColumnsToContents();
}
