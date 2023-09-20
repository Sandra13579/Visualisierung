#ifndef PAINT_H
#define PAINT_H

#include <QTextBrowser>
#include <QPainter>
#include <QTimer>
#include <datenbank.h>

class Paint : public QTextBrowser
{
public:
    Paint(QWidget* parent = nullptr);
    ~Paint();
private:
    QTimer* updateTimer;
    void updateRobotPosition();
    int robotX[5]; //Element 0 wird nicht genutzt
    int robotY[5];
    int robotState[5];
    int robotBattery[5];
    Datenbank *robotDB;

protected:
    void paintEvent(QPaintEvent* e) override;


};

#endif // PAINT_H
