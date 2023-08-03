#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
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
    void on_pushButton_clicked();

    void on_comboBox_2_currentIndexChanged(int index);

    void on_workpiece_pushButton_clicked();

    void on_comboBox_5_currentIndexChanged(int index);

    void on_comboBox_6_currentIndexChanged(int index);

    void on_pushButton_3_clicked();

    void on_production_order_name_comboBox_currentIndexChanged(int index);
    void workpiece_table(int index);


private:
    Ui::MainWindow *ui;
    Datenbank *database;
    QStandardItemModel *model;
};
#endif // MAINWINDOW_H
