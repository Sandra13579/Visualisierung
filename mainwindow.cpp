#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSqlQuery>
#include <QSqlIndex>
#include <QDateTime>
#include <QMessageBox>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Slot connections for buttons etc.
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::pushButtonClicked);
    connect(ui->workpiece_pushButton, &QPushButton::clicked, this, &MainWindow::workpiecePushButtonClicked);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::pushButton3Clicked);
    connect(ui->fault_pushButton, &QPushButton::clicked, this, &MainWindow::faultPushButtonClicked);
    connect(ui->comboBox_2, &QComboBox::currentIndexChanged, this, &MainWindow::comboBox2CurrentIndexChanged);
    connect(ui->comboBox_5, &QComboBox::currentIndexChanged, this, &MainWindow::comboBox5CurrentIndexChanged);
    connect(ui->comboBox_6, &QComboBox::currentIndexChanged, this, &MainWindow::comboBox6CurrentIndexChanged);
    connect(ui->production_order_name_comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::productionOrderNameComboBoxCurrentIndexChanged);
    connect(ui->production_order_state_lineEdit, &QLineEdit::textChanged, this, &MainWindow::productLineEditTextChanged);
    connect(ui->station_comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::stationComboBoxCurrentIndexChanged);

    database = new Datenbank("Visualization");
    database->Connect();

//Auftragsverwaltung
    //Produktauswahl hinterlegen
    QSqlQuery query(database->db());
    query.prepare("SELECT `production_process_name` FROM `vpj`.`production_process`;");
    query.exec();
    while (query.next())
    {
        ui->comboBox->addItem(query.record().value(0).toString());
    }

//Werkstückverwaltung
    //Auswahl eingeliefert/ausgeliefert
    ui->comboBox_2->insertItems(0, { "angeliefert", "ausgeliefert"});
    ui->comboBox_4->insertItems(0, { "2", "3"});

//Wartungsverwaltung
    ui->comboBox_5->insertItems(0, { "Roboter", "Station"});

//Aufträge
    query.prepare("SELECT order_name FROM vpj.production_order;");
    query.exec();
    while (query.next())
    {
        ui->production_order_name_comboBox->addItem(query.record().value(0).toString());
    }

//Station
    query.prepare("SELECT station_name FROM vpj.station WHERE station_id < 10 ORDER BY station_id ASC;;");
    query.exec();
    while (query.next())
    {
        ui->station_comboBox->addItem(query.record().value(0).toString());
    }


//Station labels
    connect(ui->pushButton_2, &QPushButton::clicked, this, [=] () { showStationPanel(7); });
    connect(ui->pushButton_4, &QPushButton::clicked, this, [=] () { showStationPanel(5); });
    connect(ui->pushButton_5, &QPushButton::clicked, this, [=] () { showStationPanel(3); });
    connect(ui->pushButton_6, &QPushButton::clicked, this, [=] () { showStationPanel(1); });
    connect(ui->pushButton_7, &QPushButton::clicked, this, [=] () { showStationPanel(9); });

    ui->groupBox->setVisible(false);

    stationUpdateTimer = new QTimer(this);
    connect(stationUpdateTimer, &QTimer::timeout, this, &MainWindow::updateStationStatus);

//Robot labels
    connect(ui->pushButton_robot, &QPushButton::clicked, this, [=] () { showRobotPanel(1); });
    connect(ui->pushButton_robot_2, &QPushButton::clicked, this, [=] () { showRobotPanel(2); });
    connect(ui->pushButton_robot_3, &QPushButton::clicked, this, [=] () { showRobotPanel(3); });
    connect(ui->pushButton_robot_4, &QPushButton::clicked, this, [=] () { showRobotPanel(4); });

    ui->groupBox_2->setVisible(false);

    robotUpdateTimer = new QTimer(this);
    connect(robotUpdateTimer, &QTimer::timeout, this, &MainWindow::updateRobotStatus);

//Tab Aktualisierung
    tabUpdateTimer = new QTimer(this);
    connect(tabUpdateTimer, &QTimer::timeout, this, &MainWindow::updateTabs);
    tabUpdateTimer->start(1000);

//Roboter Positionen Aktualisierung
    robotsPositionsUpdateTimer = new QTimer(this);
    connect(robotsPositionsUpdateTimer, &QTimer::timeout, this, &MainWindow::updateRobotPosition);
    robotsPositionsUpdateTimer->start(100);
}

MainWindow::~MainWindow()
{
    database->Disconnect();
    delete ui;
}

void MainWindow::updateTabs()
{
    updateRobotTab(); //Roboter
    int indexorder = ui->production_order_name_comboBox->currentIndex();
    productionOrderNameComboBoxCurrentIndexChanged(indexorder); //Aufträge
    stationComboBoxCurrentIndexChanged(ui->station_comboBox->currentIndex()); //Station
    comboBox6CurrentIndexChanged(ui->comboBox_6->currentIndex()); //Wartungsverwaltung
    fault(); //Fehlermeldung
}

void MainWindow::updateStationStatus()
{
    switch (selectedStation) {
    case 0:
        break;
    case 9:
    {
        QSqlQuery query(database->db());
        query.prepare("SELECT station_id, state_id FROM vpj.station_place WHERE station_id = 9");
        query.exec();
        QList<int> states;
        while (query.next())
        {
            states.append(query.record().value(1).toInt());
        }
        if (states.count() == 2)
        {
            setLabelColorFromState(ui->label_stat11, states[0]);
            ui->label_stat11->setText("1");
            setLabelColorFromState(ui->label_stat12, -1);
            ui->label_stat12->clear();
            setLabelColorFromState(ui->label_stat13, -1);
            ui->label_stat13->clear();
            setLabelColorFromState(ui->label_stat21, states[1]);
            ui->label_stat21->setText("2");
            setLabelColorFromState(ui->label_stat22, -1);
            ui->label_stat22->clear();
            setLabelColorFromState(ui->label_stat23, -1);
            ui->label_stat23->clear();
        }
        break;
    }
    default:
    {
        QSqlQuery query(database->db());
        query.prepare("SELECT station_id, state_id FROM vpj.station_place WHERE station_id IN (:station_id_1,:station_id_2)");
        query.bindValue(":station_id_1", selectedStation);
        query.bindValue(":station_id_2", selectedStation + 1);
        query.exec();
        QList<int> states;
        while (query.next())
        {
            states.append(query.record().value(1).toInt());
        }
        setLabelColorFromState(ui->label_stat11, states[0]);
        ui->label_stat11->setText("1");
        setLabelColorFromState(ui->label_stat12, states[1]);
        ui->label_stat12->setText("2");
        setLabelColorFromState(ui->label_stat13, states[2]);
        ui->label_stat13->setText("3");
        setLabelColorFromState(ui->label_stat21, states[3]);
        ui->label_stat21->setText("1");
        setLabelColorFromState(ui->label_stat22, states[4]);
        ui->label_stat22->setText("2");
        setLabelColorFromState(ui->label_stat23, states[5]);
        ui->label_stat23->setText("3");
        break;
    }
    }
}

void MainWindow::updateRobotStatus()
{
    QSqlQuery query(database->db());
    //query.prepare("SELECT state_id, battery_level FROM vpj.robot WHERE robot_id = :robot_id");
    query.prepare("SELECT robot.battery_level, state.state_name, jobtype.jobtype_name FROM vpj.robot INNER JOIN vpj.state ON robot.state_id = state.state_id INNER JOIN vpj.jobtype ON robot.jobtype_id = jobtype.jobtype_id WHERE robot.robot_id = :robot_id");
    query.bindValue(":robot_id", selectedRobot);
    query.exec();
    if (query.next())
    {
        int batteryLevel = query.record().value(0).toInt();
        QString state = query.record().value(1).toString();
        QString jobType = query.record().value(2).toString();
        ui->label_23->setText("Roboter " + QString::number(selectedRobot) + ":");
        ui->label_24->setText("Akkustand: " + QString::number(batteryLevel) + "%");
        ui->label_25->setText("Status: " + state);
        ui->label_26->setText("Auftrag: " + jobType);
    }

    query.prepare("SELECT rfid FROM vpj.workpiece WHERE robot_id = :robot_id");
    query.bindValue(":robot_id", selectedRobot);
    query.exec();
    if (query.next())
    {
        int rfid = query.record().value(0).toInt();
        ui->label_27->setText("Werkstück RFID: " + QString::number(rfid));
    }
    else
    {
        ui->label_27->setText("Werkstück RFID: -");
    }
}

//Auftragsverwaltung
void MainWindow::pushButtonClicked()
{
    //Auftragsname auslesen
    QString productionOrderName = ui->lineEdit->text();
    qDebug() << "Auftrag" << productionOrderName;
    ui->lineEdit->clear();
    //Produktauswahl auslesen
    int productionProcessId = ui->comboBox->currentIndex() + 1;
    qDebug() << "Pauftrags_id" << productionProcessId;
    //Stückzahl auslesen
    int numberOfPieces = ui->spinBox->value();
    qDebug() << "Stückzahl" << numberOfPieces;
    ui->spinBox->setValue(0);
    if (productionOrderName == "" || numberOfPieces == 0)
    {
        QMessageBox orderMsgBox;
        orderMsgBox.setWindowTitle("Meldung");
        orderMsgBox.setText("Sie haben nicht alle Auftragseingaben getätigt.");
        orderMsgBox.exec();
        qDebug() << "es wurde Eingabe vergessen";
    }
    else
    {
        QDateTime currentTime = QDateTime::currentDateTime(); //aktueller Zeitstempel

        QSqlQuery query(database->db());
        query.prepare("INSERT INTO vpj.production_order (order_name,  number_of_pieces, timestamp, workpiece_state_id, production_process_id) VALUES (:order_name, :pieces, :timestamp, :state_id, :process_id)");
        query.bindValue(":order_name", productionOrderName);
        query.bindValue(":pieces", numberOfPieces);
        query.bindValue(":timestamp", currentTime);
        query.bindValue(":state_id", 5);
        query.bindValue(":process_id", productionProcessId);
        query.exec();
        //Aufträge (siehe linke Seite Tab)
        query.prepare("SELECT order_name FROM vpj.production_order;");
        query.exec();
        while (query.next())
        {
            QString orderName = query.record().value(0).toString();
            int existingIndex = ui->production_order_name_comboBox->findText(orderName); // Überprüfe, ob der orderName bereits in der QComboBox vorhanden ist
            if (existingIndex == -1) // Wenn der orderName nicht vorhanden ist, füge ihn zur QComboBox hinzu
            {
                ui->production_order_name_comboBox->addItem(orderName);
            }
        }
    }
}

//Werkstückverwaltung
void MainWindow::comboBox2CurrentIndexChanged(int index)
{
    ui->comboBox_3->clear();
    switch (index)
    {
    case 0:
        ui->comboBox_3->insertItems(0, { "Rohstoffteillager 1", "Rohstoffteillager 2"});
        break;
    case 1:
        ui->comboBox_3->insertItems(0, { "Fertigteillager 1", "Fertigteillager 2"});
        break;
    }
}

void MainWindow::workpiecePushButtonClicked()
{
    QSqlQuery query(database->db());

    //an-/ ausgeliefert lesen
    int checkInOut = ui->comboBox_2->currentIndex();
    qDebug() << "an/ausgeliefert" << checkInOut;
    //RFID auslesen
    QString rfid = ui->lineEdit_2->text();
    qDebug() << "RFID" << rfid;
    //RFID Eingaben nur Zahlen?
    bool isConvertibleToInt = false;
    int rfidInt = rfid.toInt(&isConvertibleToInt);
    ui->lineEdit_2->clear();
    //Stationsauswahl auslesen
    int station = ui->comboBox_3->currentIndex() + 1;
    qDebug() << "Station" << station;
    //Platzauswahl auslesen
    int place = ui->comboBox_4->currentIndex() + 2;
    qDebug() << "Platz" << place;

    if (rfid == "")
    {
        QMessageBox orderMsgBox;
        orderMsgBox.setWindowTitle("Meldung");
        orderMsgBox.setText("Sie haben keine RFID eingegeben.");
        orderMsgBox.exec();
    }
    else if (!isConvertibleToInt) //keine reine Zahlenfolge
    {
        QMessageBox orderMsgBox;
        orderMsgBox.setWindowTitle("Meldung");
        orderMsgBox.setText("Ihre RFID Eingabe ist keine reine Zahlenfolge.");
        orderMsgBox.exec();
    }
    else
    {
        if (checkInOut == 1) //ausgeliefert
        {
            station += 6;
        }
        rfid = tr("%1").arg(rfid);
        QDateTime currentTime = QDateTime::currentDateTime(); //aktueller Zeitstempel
        query.prepare("SELECT station_place_id, state_id FROM vpj.station_place WHERE station_id = :station_id AND place_id = :place_id");
        query.bindValue(":station_id", station);
        query.bindValue(":place_id", place);
        query.exec();
        int stationPlaceId = 0;
        query.next();
        stationPlaceId = query.record().value(0).toInt();

        if (checkInOut == 0) //angeliefert
        {
            if (query.record().value(1).toInt() == 0) //Stationsplatz frei?
            {
                query.prepare("INSERT INTO vpj.workpiece (rfid,  checked_in, timestamp, workpiece_state_id, station_place_id) VALUES (:rfid, :checked_in, :timestamp, :state_id, :place_id)");
                query.bindValue(":rfid", rfid);
                query.bindValue(":checked_in", 1);
                query.bindValue(":timestamp", currentTime);
                query.bindValue(":state_id", 1);
                query.bindValue(":place_id", stationPlaceId);
                query.exec();

                query.prepare("UPDATE vpj.station_place SET state_id = :state_id WHERE station_place_id = :place_id");
                query.bindValue(":state_id", 1);
                query.bindValue(":place_id", stationPlaceId);
                query.exec();
            }
            else
            {
                QMessageBox orderMsgBox;
                orderMsgBox.setWindowTitle("Meldung");
                orderMsgBox.setText("Der gewählte Stationsplatz ist bereits belegt.");
                orderMsgBox.exec();
            }
        }

        else //ausgeliefert
        {
            query.prepare("SELECT rfid FROM vpj.workpiece INNER JOIN vpj.station_place ON station_place.station_place_id = workpiece.station_place_id WHERE workpiece.rfid = :rfid AND workpiece.workpiece_state_id = 0 AND workpiece.station_place_id = :station_place_id;");
            query.bindValue(":rfid", rfid);
            query.bindValue(":station_place_id", stationPlaceId);
            query.exec();
            if (query.next())
            {
                query.prepare("UPDATE vpj.station_place SET state_id = 0 WHERE station_place_id = :place_id");
                query.bindValue(":place_id", stationPlaceId);
                query.exec();
                query.prepare("UPDATE vpj.workpiece SET checked_in = 0, workpiece_state_id = 4, station_place_id = 0, timestamp = :timestamp WHERE station_place_id = :place_id");
                query.bindValue(":timestamp", currentTime);
                query.bindValue(":place_id", stationPlaceId);
                query.exec();
            }
            else
            {
                QMessageBox orderMsgBox;
                orderMsgBox.setWindowTitle("Meldung");
                orderMsgBox.setText("Die gewünschte RFID befindet sich nicht an dem gewählten Stationsplatz.");
                orderMsgBox.exec();
            }
        }
    }
}

//Wartungsverwaltung
void MainWindow::comboBox5CurrentIndexChanged(int index) //Auswahl Roboter oder Station
{
    ui->comboBox_6->clear();
    QSqlQuery query(database->db());
    switch (index)
    {
    case 0: //if robot is selected
        //Welche Roboter gibt es in DB?
        query.prepare("SELECT robot_id FROM vpj.robot ORDER BY robot_id ASC;");
        query.exec();
        while (query.next())
        {
            ui->comboBox_6->addItem(query.record().value(0).toString());    //Roboter_id in Auswahlfeld schreiben
        }
        break;
    case 1: //if station is selected
        query.prepare("SELECT station_name FROM vpj.station WHERE station_id < 9 ORDER BY station_id ASC;");
        query.exec();
        while (query.next())
        {
            ui->comboBox_6->addItem(query.record().value(0).toString());    //Stationsnamen in Auswahlfeld schreiben
        }
        ui->comboBox_6->addItems({ "Ladestation 1", "Ladestation 2" });
        break;
    }
}


void MainWindow::comboBox6CurrentIndexChanged(int index)
{
    index++;    //index count starts at "0" but the robot id starts at "1"
    switch (ui->comboBox_5->currentIndex()) {
    case 0:
    {
        ui->pushButton_3->setText("Roboter in Wartung schicken");
        QSqlQuery query(database->db());
        query.prepare("SELECT robot_id FROM vpj.robot WHERE state_id = 3;");
        query.exec();
        int robotIdState3;
        while (query.next())
        {
            robotIdState3 = query.record().value(0).toInt();
            if (index == robotIdState3) //wenn ausgewählter Roboter bereits in Wartung ist
            {
                ui->pushButton_3->setText("Roboter in Betrieb nehmen");
            }
        }
        break;
    }
    case 1:
        ui->pushButton_3->setText("Station in Wartung schicken");
        //qDebug() << "station_id_selected = " << index;
        if (index > 8)
        {
            //Charging station selected
            QSqlQuery query(database->db());
            query.prepare("SELECT maintenance FROM vpj.station_place WHERE station_id =9 AND place_id = :place_id_selected ");
            query.bindValue(":place_id_selected", index - 8);
            query.exec();
            query.next();
            //qDebug() << "Ladestation " << query.record().value(0).toInt();
            if (query.record().value(0).toInt() == 1) //wenn ausgewählte Station bereits in Wartung ist
            {
                ui->pushButton_3->setText("Station in Betrieb nehmen");
            }
        }
        else
        {
            //Bearbeitungsstationen (1-8)
            QSqlQuery query(database->db());
            query.prepare("SELECT maintenance FROM vpj.station WHERE station_id = :station_id_selected ;");
            query.bindValue(":station_id_selected", index);
            query.exec();
            query.next();
            //qDebug() << "in Maintenance = " << query.record().value(0).toInt();
            if (query.record().value(0).toInt() == 1) //wenn ausgewählte Station bereits in Wartung ist
            {
                ui->pushButton_3->setText("Station in Betrieb nehmen");
            }
        }
        break;
    }
}


void MainWindow::pushButton3Clicked() //in Wartung schicken, aus Wartung rausholen
{
    QDateTime currentTime = QDateTime::currentDateTime(); //aktueller Zeitstempel

    if ( ui->pushButton_3->text() == "Roboter in Wartung schicken")
    {
        QSqlQuery query(database->db());
        query.prepare("UPDATE vpj.robot SET maintenance = 1 WHERE robot_id = :robot_id");
        query.bindValue(":robot_id", ui->comboBox_6->currentIndex()+1);
        query.exec();
    }
    else if ( ui->pushButton_3->text() == "Roboter in Betrieb nehmen")
    {
        QSqlQuery query(database->db());
        query.prepare("UPDATE vpj.robot SET maintenance = 0, timestamp = :timestamp, state_id = 0, jobtype_id = 3 WHERE robot_id = :robot_id");
        query.bindValue(":robot_id", ui->comboBox_6->currentIndex()+1);
        query.bindValue(":timestamp", currentTime);
        query.exec();
    }
    else if ( ui->pushButton_3->text() == "Station in Wartung schicken")
    {
        int stationId = ui->comboBox_6->currentIndex()+1;

        if (stationId > 8) //Ladestationen
        {
            QSqlQuery query(database->db());
            query.prepare("SELECT station_place_id FROM `vpj`.`station_place` WHERE station_id = 9 AND place_id = :place_id ");
            query.bindValue(":place_id", stationId - 8);
            query.exec();
            query.next();
            int station_place_id = query.record().value(0).toInt();
            query.prepare("UPDATE vpj.station_place SET maintenance = 1, state_id = 3 WHERE station_place_id = :station_place_id AND state_id = 0;");
            query.bindValue(":station_place_id", station_place_id);
            query.exec();
        }

        if (stationId < 8) //Bearbeitungsstation gewählt
        {
            QSqlQuery query(database->db());
            query.prepare("UPDATE vpj.station SET maintenance = 1 WHERE station_id = :station_id");
            query.bindValue(":station_id", stationId);
            query.exec();

            query.prepare("UPDATE vpj.station_place SET state_id = 3 WHERE station_id = :station_id AND state_id = 0");
            query.bindValue(":station_id", stationId);
            query.exec();
        }
    }
    else if ( ui->pushButton_3->text() == "Station in Betrieb nehmen")
    {
        int station_id = ui->comboBox_6->currentIndex()+1;

        if (station_id > 8) //Ladestationen
        {
            QSqlQuery query(database->db());
            query.prepare("SELECT station_place_id FROM `vpj`.`station_place` WHERE station_id = 9 AND place_id = :place_id ");
            query.bindValue(":place_id", station_id - 8);
            query.exec();
            query.next();
            int stationPlaceId = query.record().value(0).toInt();
            query.prepare("UPDATE vpj.station_place SET maintenance = 0, state_id = 0 WHERE station_place_id = :station_place_id AND state_id = 3;");
            query.bindValue(":station_place_id", stationPlaceId);
            query.exec();
        }

        if (station_id < 8) //Bearbeitungsstation gewählt
        {
            QSqlQuery query(database->db());
            query.prepare("UPDATE vpj.station SET maintenance = 0 WHERE station_id = :station_id");
            query.bindValue(":station_id", station_id);
            query.exec();

            query.prepare("UPDATE vpj.station_place SET state_id = 0 WHERE station_id = :station_id AND state_id = 3");
            query.bindValue(":station_id", station_id);
            query.exec();
        }
    }
}

//Aufträge
void MainWindow::productionOrderNameComboBoxCurrentIndexChanged(int index)
{
    //Wekstückeübersicht
    int totalProcessed = workpiece_table(index +1);

    //Produkt
    QString orderName = ui->production_order_name_comboBox->currentText();
    QSqlQuery query(database->db());
    query.prepare("SELECT production_process_name FROM vpj.production_process INNER JOIN vpj.production_order ON production_order.production_process_id = production_process.production_process_id WHERE production_order.order_name = :order_name;");
    query.bindValue(":order_name", orderName);
    query.exec();
    query.next();
    ui->product_lineEdit->setText(query.record().value(0).toString());

    //Auftragsstatus
    query.prepare("SELECT workpiece_state_name FROM vpj.workpiece_state INNER JOIN vpj.production_order ON production_order.workpiece_state_id = workpiece_state.workpiece_state_id WHERE production_order.order_name = :order_name;");
    query.bindValue(":order_name", orderName);
    query.exec();
    query.next();
    ui->production_order_state_lineEdit->setText(query.record().value(0).toString());

    //Fertigungsstand
    query.prepare("SELECT number_of_pieces FROM vpj.production_order WHERE production_order.order_name = :order_name;");
    query.bindValue(":order_name", orderName);
    query.exec();
    if (query.next())
    {
        int numberOfPieces = query.record().value(0).toInt();

        ui->manufacturing_progressBar->setValue(100/numberOfPieces * totalProcessed);
    }
    else
    {
        ui->manufacturing_progressBar->setValue(0);
    }
}

int MainWindow::workpiece_table(int index) //Werkstückübersicht
{
    //qDebug() << "index = " <<index;
    int totalProcessed = 0;

    //Sicherstellen, dass die Zeigerobjekte auch wieder gelöscht werden!
    //Sonst ist der Speicher irgendwann voll...
    model = new QStandardItemModel();

    model->setHorizontalHeaderLabels({"Fertigungsschritt", "       Dauer     ", "       Stückzahl      "}); // Setze die Spaltenüberschriften

    QList<QString> fertigungsschritte = {"unbearbeitet", "sägen", "schleifen", "lackieren", "polstern", "fertig produziert", "ausgeliefert"};
    int rowCount = fertigungsschritte.size();
    model->setRowCount(rowCount);
    for (int row = 0; row < rowCount; ++row)
    {
        QString fertigungsschritt = fertigungsschritte.at(row);
        QString duration, counts;

        QStandardItem *fertigungsschrittItem = new QStandardItem(fertigungsschritt); // Füge den Namen des Fertigungsschritte in die erste Spalte ein
        model->setItem(row, 0, fertigungsschrittItem);

        //Schrittdauer
        QSqlQuery query(database->db());
        query.prepare("SELECT step_duration FROM vpj.production_step INNER JOIN vpj.production_order ON production_order.production_process_id = production_step.production_process_id WHERE production_order.production_order_id = :order_id AND production_step.step_id = :step_id;");
        query.bindValue(":order_id", index);
        query.bindValue(":step_id", row);
        query.exec();
        if (query.next() && row < 5)
        {
            duration = tr("%1").arg(query.record().value(0).toInt());
            //qDebug() << "in Duration = " << query.record().value(0).toInt();
            QStandardItem *durationItem = new QStandardItem(duration);
            model->setItem(row, 1, durationItem);

            //Stückzahl
            query.prepare("SELECT COUNT(*) FROM vpj.workpiece WHERE production_order_id = :order_id AND step_id = :step_id;");
            query.bindValue(":order_id", index);
            query.bindValue(":step_id", row);
            query.exec();
            if (query.next())
            {
                counts = tr("%1").arg(query.record().value(0).toInt());
                //qDebug() << "in counts = " << query.record().value(0).toInt();
                QStandardItem *countsItem = new QStandardItem(counts);
                model->setItem(row, 2, countsItem);
            }
        }
        else
        {
            QStandardItem *emptyItem = new QStandardItem();
            model->setItem(row, 1, emptyItem);
            if (row < 5)
            {
                model->setItem(row, 2, emptyItem);
            }
            else if (row == 5) //Stückzahl, fertig produziert
            {
                query.prepare("SELECT COUNT(*) FROM vpj.workpiece WHERE production_order_id = :order_id AND workpiece_state_id = 0;");
                query.bindValue(":order_id", index);
                query.exec();
                query.next();
                totalProcessed = query.record().value(0).toInt();
                counts = tr("%1").arg(query.record().value(0).toInt());
                //qDebug() << "in counts = " << query.record().value(0).toInt();
                QStandardItem *countsItem = new QStandardItem(counts);
                model->setItem(row, 2, countsItem);
            }
            else if (row == 6) //Stückzahl, ausgeliefert
            {
                query.prepare("SELECT COUNT(*) FROM vpj.workpiece WHERE production_order_id = :order_id AND workpiece_state_id = 4;");
                query.bindValue(":order_id", index);
                query.exec();
                query.next();
                totalProcessed += query.record().value(0).toInt();
                counts = tr("%1").arg(query.record().value(0).toInt());
                //qDebug() << "in counts = " << query.record().value(0).toInt();
                QStandardItem *countsItem = new QStandardItem(counts);
                model->setItem(row, 2, countsItem);
            }
        }
    }
    ui->workpiece_tableView->setModel(model); // erstellt die Tabelle
    return totalProcessed;
    // Optional: Größe der Spalten automatisch an den Inhalt anpassen
    //ui->workpiece_tableView->resizeColumnsToContents();
}

void MainWindow::productLineEditTextChanged() //Bilder ändern
{
    ui->picture_label->clear();
    if (ui->product_lineEdit->text() == "Tisch")
    {
        QPixmap pic(":/images/Tisch.png");
        QSize labelSize = ui->picture_label->size();
        ui->picture_label->setPixmap(pic.scaled(labelSize, Qt::KeepAspectRatio));
    }
    if (ui->product_lineEdit->text() == "Stuhl")
    {
        QPixmap pic(":/images/Stuhl.png");
        QSize labelSize = ui->picture_label->size();
        ui->picture_label->setPixmap(pic.scaled(labelSize, Qt::KeepAspectRatio));
    }
    if (ui->product_lineEdit->text() == "Bett")
    {
        QPixmap pic(":/images/Bett.png");
        QSize labelSize = ui->picture_label->size();
        ui->picture_label->setPixmap(pic.scaled(labelSize, Qt::KeepAspectRatio));
    }
    if (ui->product_lineEdit->text() == "Regal")
    {
        QPixmap pic(":/images/Regal.png");
        QSize labelSize = ui->picture_label->size();
        ui->picture_label->setPixmap(pic.scaled(labelSize, Qt::KeepAspectRatio));
    }
    if (ui->product_lineEdit->text() == "Schrank")
    {
        QPixmap pic(":/images/Schrank.png");
        QSize labelSize = ui->picture_label->size();
        ui->picture_label->setPixmap(pic.scaled(labelSize, Qt::KeepAspectRatio));
    }
    if (ui->product_lineEdit->text() == "Hocker")
    {
        QPixmap pic(":/images/Hocker.png");
        QSize labelSize = ui->picture_label->size();
        ui->picture_label->setPixmap(pic.scaled(labelSize, Qt::KeepAspectRatio));
    }
}

//Station
void MainWindow::stationComboBoxCurrentIndexChanged(int index)
{
    int stationId = index+1;
    int placeId = 2;
    ui->label_14->setText("Platz 2");
    ui->label_15->setText("Platz 3");
    ui->label_18->setVisible(true);
    ui->label_19->setVisible(true);
    ui->station_history_lineEdit_2->setVisible(true);
    ui->station_history_lineEdit_3->setVisible(true);
    if (stationId == 9) //Ladestationen
    {
        placeId = 1;
        ui->label_14->setText("Platz 1");
        ui->label_15->setText("Platz 2");
        ui->label_18->setVisible(false);
        ui->label_19->setVisible(false);
        ui->station_history_lineEdit_2->setVisible(false);
        ui->station_history_lineEdit_3->setVisible(false);
    }
    QSqlQuery query(database->db());
    //Status
    //Platz 2
    query.prepare("SELECT state_name FROM vpj.state INNER JOIN vpj.station_place ON state.state_id = station_place.state_id WHERE station_place.station_id = :station_id AND place_id = :place_id;");
    query.bindValue(":station_id", stationId);
    query.bindValue(":place_id", placeId);
    query.exec();
    if (query.next())
    {
        ui->station_state_lineEdit_2->setText(query.record().value(0).toString());
    }
    //Platz 3
    query.prepare("SELECT state_name FROM vpj.state INNER JOIN vpj.station_place ON state.state_id = station_place.state_id WHERE station_place.station_id = :station_id AND place_id = :place_id;");
    query.bindValue(":station_id", stationId);
    query.bindValue(":place_id", placeId +1);
    query.exec();
    if (query.next())
    {
        ui->station_state_lineEdit_3->setText(query.record().value(0).toString());
    }

    //Historie
    //Platz 2
    query.prepare("SELECT COUNT(*) FROM vpj.workpiece_history INNER JOIN vpj.station_place ON workpiece_history.station_place_id = station_place.station_place_id WHERE station_place.station_id = :station_id AND station_place.place_id = :place_id AND workpiece_history.workpiece_state_id = 1;");
    query.bindValue(":station_id", stationId);
    query.bindValue(":place_id", placeId);
    query.exec();
    query.next();
    ui->station_history_lineEdit_2->setText(query.record().value(0).toString());
    //Platz 3
    query.prepare("SELECT COUNT(*) FROM vpj.workpiece_history INNER JOIN vpj.station_place ON workpiece_history.station_place_id = station_place.station_place_id WHERE station_place.station_id = :station_id AND station_place.place_id = :place_id AND workpiece_history.workpiece_state_id = 1;");
    query.bindValue(":station_id", stationId);
    query.bindValue(":place_id", placeId +1);
    query.exec();
    query.next();
    ui->station_history_lineEdit_3->setText(query.record().value(0).toString());
}

//Roboter
void MainWindow::updateRobotTab()
{
    QSqlQuery test(database->db());
    test.prepare("SELECT robot.robot_id, robot.battery_level, state.state_name, jobtype.jobtype_name FROM vpj.robot INNER JOIN vpj.state ON robot.state_id = state.state_id INNER JOIN vpj.jobtype ON robot.jobtype_id = jobtype.jobtype_id");
    test.exec();
    while (test.next())
    {
        int robotId = test.record().value(0).toInt();
        int batteryLevel = test.record().value(1).toInt();
        QString state = test.record().value(2).toString();
        QString jobType = test.record().value(3).toString();

        QSqlQuery query2(database->db());
        query2.prepare("SELECT COUNT(DISTINCT workpiece_history.workpiece_id, station_place.station_id) FROM vpj.workpiece_history INNER JOIN vpj.station_place ON station_place.station_place_id = workpiece_history.station_place_id WHERE workpiece_history.workpiece_state_id = 3 AND workpiece_history.robot_id = :robot_id;");
        query2.bindValue(":robot_id", robotId);
        query2.exec();
        query2.next();
        QString workpieceCount = query2.record().value(0).toString();

        switch (robotId)
        {
        case 1:
            ui->robot1_workpieces_lineEdit->setText(workpieceCount);
            ui->robot1_state_lineEdit->setText(state);
            if (state != "inaktiv")
            {
                ui->robot1_progressBar->setValue(batteryLevel);
                ui->robot1_jobtype_lineEdit->setText(jobType);
            }
            else
            {
                ui->robot1_progressBar->setValue(0);
            }
            break;
        case 2:
            ui->robot2_workpieces_lineEdit->setText(workpieceCount);
            ui->robot2_state_lineEdit->setText(state);
            if (state != "inaktiv")
            {
                ui->robot2_progressBar->setValue(batteryLevel);
                ui->robot2_jobtype_lineEdit->setText(jobType);
            }
            else
            {
                ui->robot2_progressBar->setValue(0);
            }
            break;
        case 3:
            ui->robot3_workpieces_lineEdit->setText(workpieceCount);
            ui->robot3_state_lineEdit->setText(state);
            if (state != "inaktiv")
            {
                ui->robot3_progressBar->setValue(batteryLevel);
                ui->robot3_jobtype_lineEdit->setText(jobType);
            }
            else
            {
                ui->robot3_progressBar->setValue(0);
            }
            break;
        case 4:
            ui->robot4_workpieces_lineEdit->setText(workpieceCount);
            ui->robot4_state_lineEdit->setText(state);
            if (state != "inaktiv")
            {
                ui->robot4_progressBar->setValue(batteryLevel);
                ui->robot4_jobtype_lineEdit->setText(jobType);
            }
            else
            {
                ui->robot4_progressBar->setValue(0);
            }
            break;
        default:
            break;
        }
    }
    //update();
}

//Fehler
void MainWindow::fault()
{
    ui->label_21->setText("");

    //Ladestation wurde nicht richtig verbunden
    QSqlQuery query(database->db());
    query.prepare("SELECT robot_id, station_place_id FROM vpj.robot WHERE state_id = 4 AND (station_place_id = 25 OR station_place_id = 26);");
    query.exec();
    if (query.next())
    {
        QString faultText = "Roboter " + query.record().value(0).toString() + " wurde nicht richtig mit der Ladestation ";
        if (query.record().value(1).toInt() == 25)
        {
            faultText += "1";
        }
        else
        {
            faultText += "2";
        }
        faultText += " verbunden!";
        ui->label_21->setStyleSheet("color: red;");
        ui->label_21->setText(faultText);
        ui->label_21->setToolTip("Hinweis: Verbinde den Roboter richtig mit der Ladestation, danach darf quittiert werden");
    }
    else
    {
        //es wurde eine falsche RFID eingelesen
        query.prepare("SELECT station_id FROM vpj.station WHERE state_id = 4;");
        query.exec();
        if (query.next())
        {
            QString faultText = "Es liegt ein falsches Werkstück unter dem RFID-Reader " + query.record().value(0).toString();
            ui->label_21->setStyleSheet("color: red;");
            ui->label_21->setText(faultText);
            ui->label_21->setToolTip("Hinweis: Lege das Werkstück zurück an den Startplatz, danach darf quittiert werden");
        }
        else
        {
            //ein werkstück wurde verloren
            query.prepare("SELECT robot_id FROM vpj.robot WHERE state_id = 4 AND (station_place_id != 25 OR station_place_id != 26);");
            query.exec();
            if (query.next())
            {
                QString faultText = "Der Roboter " + query.record().value(0).toString() + " hat ein Werkstück verloren";
                ui->label_21->setStyleSheet("color: red;");
                ui->label_21->setText(faultText);
                ui->label_21->setToolTip("Hinweis: Entferne das heruntergefallene Werkstück aus dem Produktionsprozess\n(dieses ist fehlerhaft und somit Ausschuss), danach darf quittiert werden");
            }
        }
    }
}

void MainWindow::faultPushButtonClicked()
{
    QSqlQuery query(database->db());
    QSqlQuery query2(database->db());
    QSqlQuery query3(database->db());
    QSqlQuery query4(database->db());
    QSqlQuery query5(database->db());
    query.prepare("SELECT robot_id FROM vpj.robot WHERE state_id = 4 AND (station_place_id = 25 OR station_place_id = 26);");
    query.exec();
    if (query.next()) //der Roboter hat sich nicht richtig mit der Ladestation verbunden
    {
        query2.prepare("UPDATE vpj.robot SET state_id = 5, timestamp = NOW() WHERE robot_id = :robot_id;");
        query2.bindValue(":robot_id", query.record().value(0).toString());
        query2.exec();
    }
    else
    {
        query.prepare("SELECT station_id FROM vpj.station WHERE state_id = 4;");
        query.exec();
        if (query.next()) // es wurde eine falsche RFID gelesen
        {
            int start_station_id = query.record().value(0).toInt();
            query2.prepare("SELECT robot_id FROM vpj.robot r INNER JOIN vpj.station_place sp ON r.station_place_id = sp.station_place_id WHERE sp.station_id = :station_id;");
            query2.bindValue(":station_id", start_station_id);
            query2.exec();
            query2.next();
            int robot_id = query2.record().value(0).toInt();
            query3.prepare("UPDATE vpj.robot SET state_id = 0 WHERE robot_id = :robot_id;"); //Roboter frei geben
            query3.bindValue(":robot_id", robot_id);
            query3.exec();
            qDebug() << "fehler" << "robot_id" << query2.record().value(0).toInt() << "station_id" << query.record().value(0).toInt();

            //Zielstationsplatz freigeben
            query3.prepare("SELECT destination_station_place_id, start_station_place_id FROM vpj.workpiece wp INNER JOIN vpj.robot r ON r.robot_id = wp.robot_id WHERE r.robot_id = :robot_id;");
            query3.bindValue(":robot_id", query2.record().value(0).toString());
            query3.exec();
            query3.next();
            int destination_station_place_id = query3.record().value(0).toInt();
            int start_station_place_id = query3.record().value(1).toInt();
            query4.prepare("UPDATE vpj.station_place SET state_id = 0 WHERE station_place_id = :station_place_id;"); //Roboter frei geben
            query4.bindValue(":station_place_id", destination_station_place_id);
            query4.exec();
            qDebug() << "fehler" << "station_place_id" << query3.record().value(0).toInt();


            //Start und Zielstation vorbereiten für Freigabe
            query4.prepare("SELECT station_id FROM vpj.station_place WHERE  station_place_id = :destination_station_place_id;");
            query4.bindValue(":destination_station_place_id",destination_station_place_id);
            query4.exec();
            query4.next();
            int destination_station_id = query4.record().value(0).toInt();
            query5.prepare("UPDATE vpj.station SET state_id = 0 WHERE station_id = :station_id");
            query5.bindValue(":station_id", destination_station_id);
            query5.exec();
            query5.prepare("UPDATE vpj.station SET state_id = 1 WHERE station_id = :station_id");
            query5.bindValue(":station_id", start_station_id);
            query5.exec();

            //Transportauftrag zurücknehmen
            query4.prepare("UPDATE vpj.workpiece SET workpiece_state_id = 1, robot_id = NULL, step_id = step_id -1 WHERE station_place_id = :start_station_place_id;");
            query4.bindValue(":start_station_place_id", start_station_place_id);
            query4.exec();
        }
        else // ein Werkstück ist verloren gegangen
        {
            query.prepare("SELECT robot_id FROM vpj.robot WHERE state_id = 4 AND (station_place_id != 25 OR station_place_id != 26);");
            query.exec();
            if (query.next())
            {
                int robot_id = query.record().value(0).toInt();
                query2.prepare("SELECT workpiece_id, checked_in, start_station_place_id, destination_station_place_id, station_place_id, production_order_id FROM vpj.workpiece WHERE robot_id = :robot_id;");
                query2.bindValue(":robot_id", robot_id);
                query2.exec();
                if (query2.next())
                {
                    int workpiece_id = query2.record().value(0).toInt();
                    int checked_in = query2.record().value(1).toInt();
                    int start_station_place_id = query2.record().value(2).toInt();
                    int destination_station_place_id = query2.record().value(3).toInt();
                    int station_place_id = query2.record().value(4).toInt();
                    int production_order_id = query2.record().value(5).toInt();
                    query3.prepare("SELECT station_id FROM vpj.station_place WHERE station_place_id = :station_place_id;");
                    query3.bindValue(":station_place_id", start_station_place_id);
                    query3.exec();
                    query3.next();
                    int start_station_id = query3.record().value(0).toInt();
                    query3.prepare("SELECT station_id FROM vpj.station_place WHERE station_place_id = :station_place_id;");
                    query3.bindValue(":station_place_id", destination_station_place_id);
                    query3.exec();
                    query3.next();
                    int destination_station_id = query3.record().value(0).toInt();

                    if (checked_in == 1 && start_station_place_id == station_place_id)
                    {
                        query3.prepare("UPDATE vpj.station_place SET state_id = 0 WHERE station_place_id = :start_station_place_id OR station_place_id = :destination_station_place_id;");
                        query3.bindValue(":start_station_place_id", start_station_place_id);
                        query3.bindValue(":destination_station_place_id", destination_station_place_id);
                        query3.exec();

                        query4.prepare("UPDATE vpj.station SET state_id = 0 WHERE station_id = :station_id;");
                        query4.bindValue(":station_id", destination_station_id);
                        query4.exec();                                              
                        query4.prepare("UPDATE vpj.station SET state_id = 1 WHERE station_id = :station_id;");
                        query4.bindValue(":station_id", start_station_id);
                        query4.exec();
                    }
                    else
                    {
                        query3.prepare("UPDATE vpj.robot SET station_place_id = :station_place_id WHERE robot_id = :robot_id;");
                        query3.bindValue(":station_place_id", destination_station_place_id);
                        query3.bindValue(":robot_id", robot_id);
                        query3.exec();

                        query3.prepare("UPDATE vpj.station_place SET state_id = 0 WHERE station_place_id = :destination_station_place_id;");
                        query3.bindValue(":destination_station_place_id", destination_station_place_id);
                        query3.exec();
                        query4.prepare("UPDATE vpj.station SET state_id = 1 WHERE station_id = :station_id;");
                        query4.bindValue(":station_id", destination_station_id);
                        query4.exec();
                    }
                    query3.prepare("UPDATE vpj.robot SET state_id = 0 WHERE robot_id = :robot_id;");
                    query3.bindValue(":robot_id", robot_id);
                    query3.exec();
                    query3.prepare("UPDATE vpj.production_order SET assigned_workpieces = assigned_workpieces -1 WHERE production_order_id = :production_order_id;");
                    query3.bindValue(":production_order_id", production_order_id);
                    query3.exec();
                    query3.prepare("UPDATE vpj.workpiece SET production_order_id = NULL, workpiece_state_id = 6, station_place_id = NULL, robot_id = NULL, checked_in = 0, TIMESTAMP = NOW() WHERE workpiece_id = :workpiece_id;");
                    query3.bindValue(":workpiece_id", workpiece_id);
                    query3.exec();
                }
            }
        }
    }
}


//Stationsplatzvisualisierung
void MainWindow::showStationPanel(int stationId)
{
    if (!ui->groupBox->isVisible() || stationId != selectedStation)
    {
        ui->groupBox->setVisible(true);

        if (stationId == 9)     //charging station
        {
            ui->groupBox->move(650, 300);
            ui->groupBox->resize(104, 86);
            ui->label_stat1_cap->clear();
            ui->label_stat2_cap->setText(QString::number(stationId));
            ui->label_stat2_cap->move(ui->groupBox->size().width() / 2 - 10, 20);
        }
        else    //working station
        {
            ui->groupBox->move(650, 130);
            ui->groupBox->resize(104, 256);
            ui->label_stat1_cap->setText(QString::number(stationId));
            ui->label_stat2_cap->setText(QString::number(stationId + 1));
            ui->label_stat2_cap->move(20, 20);
        }
        selectedStation = stationId;
        stationUpdateTimer->start(100);
    }
    else
    {
        ui->groupBox->setVisible(false);
        selectedStation = 0;
        stationUpdateTimer->stop();
    }
}

void MainWindow::showRobotPanel(int robotId)
{
    if (!ui->groupBox_2->isVisible() || robotId != selectedRobot)
    {
        ui->groupBox_2->setVisible(true);
        selectedRobot = robotId;
        robotUpdateTimer->start(100);
    }
    else
    {
        ui->groupBox_2->setVisible(false);
        selectedRobot = 0;
        robotUpdateTimer->stop();
    }
}

void MainWindow::updateRobotPosition()
{
    QSqlQuery query(database->db());
    query.prepare("SELECT robot_id, robot_position_x, robot_position_y, state_id, battery_level FROM vpj.robot;");
    if (!query.exec())
    {
        qWarning() << "Failed to execute query updateRobotPosition";
        return;
    }
    while (query.next())
    {
        int robotId = query.record().value(0).toInt();
        int x = query.record().value(1).toInt() / 10;
        int y = 400 - query.record().value(2).toInt() / 10;
        int state = query.record().value(3).toInt();
        int batteryLevel = query.record().value(4).toInt();

        switch (robotId) {
        case 1:
            setRobotPosition(ui->pushButton_robot, state, x, y, batteryLevel);
            break;
        case 2:
            setRobotPosition(ui->pushButton_robot_2, state, x, y, batteryLevel);
            break;
        case 3:
            setRobotPosition(ui->pushButton_robot_3, state, x, y, batteryLevel);
            break;
        case 4:
            setRobotPosition(ui->pushButton_robot_4, state, x, y, batteryLevel);
            break;
        default:
            break;
        }
    }
}

void MainWindow::setRobotPosition(QPushButton *button, int state, int x, int y, int batteryLevel)
{
    if (state == 3)
    {
        button->setVisible(false);
    }
    else
    {
        button->setGeometry(x - 15, y - 15, 30, 30);
        if (batteryLevel <= 30)
        {
            button->setStyleSheet("background-color: red; border-radius: 15px; border: 1px solid gray;");
        }
        else if (batteryLevel <=70)
        {
            button->setStyleSheet("background-color: orange; border-radius: 15px; border: 1px solid gray;");
        }
        else
        {
            button->setStyleSheet("background-color: rgb(52, 235, 52); border-radius: 15px; border: 1px solid gray;");
        }
        button->setVisible(true);
    }
}

void MainWindow::setLabelColorFromState(QLabel *label, int state)
{
    switch (state)
    {
    case 0: //State: available
        label->setStyleSheet("background-color: green; border-radius: 10px; color: white;");
        label->setToolTip("Frei");
        break;
    case 1: //State: assigned
        label->setStyleSheet("background-color: orange; border-radius: 10px;");
        label->setToolTip("Belegt");
        break;
    case 2: //State: reserved
        label->setStyleSheet("background-color: yellow; border-radius: 10px;");
        label->setToolTip("Reserviert");
        break;
    case 3: //State: inactive
        label->setStyleSheet("background-color: grey; border-radius: 10px; color: white;");
        label->setToolTip("Inaktiv");
        break;
    case 4: //State: fault
        label->setStyleSheet("background-color: red; border-radius: 10px; color: white;");
        label->setToolTip("Fehler");
        break;
    default: //default state make the label invisible
        label->setStyleSheet("background-color: white;");
        label->setToolTip("");
        break;
    }
}
