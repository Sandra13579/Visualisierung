#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
#include <QLabel>
#include <datenbank.h>

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

    void pushButtonClicked();
    void WorkpiecePushButtonClicked();
    void pushButton3Clicked();
    void faultPushButtonClicked();

    void comboBox2CurrentIndexChanged(int index);
    void comboBox5CurrentIndexChanged(int index);
    void comboBox6CurrentIndexChanged(int index);
    void productionOrderNameComboBoxCurrentIndexChanged(int index);
    int workpiece_table(int index);

    void on_station_comboBox_currentIndexChanged();

    void on_product_lineEdit_textChanged();

    void fault();

    void showStationPanel(int stationId);

private:
    Ui::MainWindow *ui;
    Datenbank *database;
    QStandardItemModel *model;
    QTimer* updateTimer;
    QTimer* stationUpdateTimer;
    int selectedStation = 0;
    void updateRobotTab();
    void setLabelColorFromState(QLabel *label, int state);
};
#endif // MAINWINDOW_H
