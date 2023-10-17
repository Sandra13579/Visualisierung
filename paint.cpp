#include <QSqlQuery>
#include <QSqlIndex>
#include "paint.h"
#include "datenbank.h"

Paint::Paint(QWidget* parent) : QTextBrowser(parent)
{
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &Paint::updateRobotPosition);
    updateTimer->start(15);

    robotDB = new Datenbank("Painter");
    robotDB->Connect();
}

Paint::~Paint()
{
    updateTimer->stop();
    robotDB->Disconnect();
}

void Paint::updateRobotPosition()
{
    QSqlQuery query(robotDB->db());
    query.prepare("SELECT robot_id, robot_position_x, robot_position_y, state_id, battery_level FROM vpj.robot;");
    query.exec();

    int id;
    if (!query.exec())
    {
        qWarning() << "Failed to execute query updateRobotPosition";
    }
    while (query.next())
    {
        QSqlRecord record = query.record();
        id = record.value(0).toInt();
        if ( id <= 4)
        {
            robotX[id] = record.value(1).toInt()/10;
            robotY[id] = 400-record.value(2).toInt()/10; //Umrechnung der Kameraposition in Bildposition
            robotState[id] = record.value(3).toInt();
            robotBattery[id] = record.value(4).toInt();
            //qDebug() << "RobID " << id << " PosX = " << record.value(1).toInt() << " PosY = " << record.value(2).toInt() << " state "<< record.value(3).toInt();
        }
    }
    update();
}

void Paint::paintEvent(QPaintEvent* e)
{
    QTextBrowser::paintEvent(e);
//    QPainter finished_parts_warehouse(this->viewport());
//    finished_parts_warehouse.fillRect(105, 97, 13, 32, Qt::gray);
//    QPainter machine34(this->viewport());
//    machine34.fillRect(225, 97, 13, 32, Qt::gray);
//    QPainter machine12(this->viewport());
//    machine12.fillRect(345, 97, 13, 32, Qt::gray);
//    QPainter raw_material_parts_warehouse(this->viewport());
//    raw_material_parts_warehouse.fillRect(465, 97, 13, 32, Qt::gray);
//    QPainter charging_station(this->viewport());
//    charging_station.fillRect(613, 0, 60, 28, Qt::gray);
    QPainter robot(this->viewport());
    for (int i = 1; i < 5; i++)
    {
        if (robotState[i] != 3)
        { // nur wenn der Roboter nicht in Wartung
            if (robotBattery[i] <= 30)
            {
                robot.setBrush(Qt::red);
            }
            else if (robotBattery[i] <= 70)
            {
                robot.setBrush(QColor(245,154,17,255)); //Farbe=Orange (RGB) und dann Transparenz (255 höchster Wert)
            }
            else
            {
                robot.setBrush(Qt::green);
            }
            robot.drawEllipse(robotX[i]-15,robotY[i]-15,30,30); //x,y,breite,höhe

            robot.drawText(robotX[i]+12-15,robotY[i]+20-15, tr("%1").arg(i)); //tr("%1").arg(i) konvertiert QString zu int
            //qDebug() << "Zeichne Rob " << i;
        }
    }
}
