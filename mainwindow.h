#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QTimer>
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

    void on_pushButton_clicked();

    void on_comboBox_2_currentIndexChanged();

    void on_workpiece_pushButton_clicked();

    void on_comboBox_5_currentIndexChanged();

    void on_comboBox_6_currentIndexChanged();

    void on_pushButton_3_clicked();

    void on_production_order_name_comboBox_currentIndexChanged(int index);
    int workpiece_table(int index);

    void on_station_comboBox_currentIndexChanged();

    void on_product_lineEdit_textChanged();

private:
    Ui::MainWindow *ui;
    Datenbank *database;
    QStandardItemModel *model;
    QTimer* updateTimer;
    void updateRobotTab();
};
#endif // MAINWINDOW_H
