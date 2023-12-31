#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <datenbank.h>
#include "interface.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateTabs();
    void updateStationStatus();
    void updateRobotStatus();
    void updateRobotPosition();

    void pushButtonClicked();
    void workpiecePushButtonClicked();
    void pushButton3Clicked();
    void faultPushButtonClicked();

    void comboBox2CurrentIndexChanged(int index);
    void comboBox5CurrentIndexChanged(int index);
    void comboBox6CurrentIndexChanged(int index);
    void productionOrderNameComboBoxCurrentIndexChanged(int index);
    void cancel_loading1_pushButton();
    void cancel_loading2_pushButton();
    void cancel_loading3_pushButton();
    void cancel_loading4_pushButton();
    int workpiece_table(int index);
    
    void stationComboBoxCurrentIndexChanged(int index);
    
    void productLineEditTextChanged();

    void fault();

    void showStationPanel(int stationId);
    void showRobotPanel(int robotId);

private:
    Ui::MainWindow *ui;
    Datenbank *database;
    QStandardItemModel *model;
    QTimer* tabUpdateTimer;
    QTimer* stationUpdateTimer;
    QTimer* robotUpdateTimer;
    QTimer* robotsPositionsUpdateTimer;
    Interface* interface;
    int selectedStation = 0;
    int selectedRobot = 0;
    void updateRobotTab();
    void setLabelColorFromState(QLabel *label, int state, int rfid = -1);
    void setRobotPosition(QPushButton *button, int state, int x, int y, int batteryLevel);
};
#endif // MAINWINDOW_H
