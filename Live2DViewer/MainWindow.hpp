#pragma once


#include "ui_MainWindow.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void onTabCloseRequested(int index);

    void onOpenModel();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow ui;
};