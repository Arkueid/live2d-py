#pragma once


#include "ui_MainWindow.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void onTabCloseRequested(int index);

    void onOpenModel();

    void showHelp();

    void showAbout();
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow ui;
};