#include "MainWindow.hpp"

#include <QFileDialog>
#include <QFileInfo>

#include "Live2DView.hpp"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    ui.setupUi(this);


    connect(ui.tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);

    connect(ui.actionOpen, &QAction::triggered, this, &MainWindow::onOpenModel);
    connect(ui.clbOpen, &QCommandLinkButton::clicked, this, &MainWindow::onOpenModel);

    ui.welcomeIcon->setPixmap(QPixmap("./moeroid.ico"));
}

MainWindow::~MainWindow()
{
}

void MainWindow::onTabCloseRequested(int index)
{
    QWidget *widget = ui.tabs->widget(index);

    ui.tabs->removeTab(index);

    delete widget;
}

void MainWindow::onOpenModel()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Model"), "../../Resources/v3", tr("Live2D Model (*.model3.json)"));

    if (fileName.isEmpty())
        return;

    Live2DView *scene = new Live2DView(fileName, this);

    QFileInfo fileInfo(fileName);
    ui.tabs->addTab(scene, fileInfo.baseName());
    ui.tabs->setCurrentWidget(scene);
}