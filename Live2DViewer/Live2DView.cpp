#include "Live2DView.hpp"

#include <QFileInfo>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QSlider>
#include <QTreeWidgetItem>

Live2DView::Live2DView(const QString &filePath, QWidget *parent) : QWidget(parent), hasCdi(false), syncTimer(this)
{
    ui.setupUi(this);

    ui.splitter->setStretchFactor(0, 0);
    ui.splitter->setStretchFactor(1, 1);

    ui.scene->LoadModel(filePath);

    paramValues = ui.scene->GetParamValues();

    Model *model = ui.scene->GetModel();
    initExpressions(model);
    initMotions(model);

    initCdi(model);

    initParameters(model);
    initParts(model);
    initDrawables(model);

    connect(ui.treeWidget, &QTreeWidget::itemDoubleClicked, this, &Live2DView::onTreeItemDoubleClicked);

    connect(ui.autoBreath, &QCheckBox::checkStateChanged, [&](Qt::CheckState state)
            { ui.scene->setAutoBreath(state == Qt::Checked); });
    connect(ui.autoBlink, &QCheckBox::checkStateChanged, [&](Qt::CheckState state)
            { ui.scene->setAutoBlink(state == Qt::Checked); });
    connect(ui.autoPhysics, &QCheckBox::checkStateChanged, [&](Qt::CheckState state)
            { ui.scene->setAutoPhysics(state == Qt::Checked); });

    syncTimer.setInterval(1000 / 40);
    syncTimer.setSingleShot(false);
    connect(&syncTimer, &QTimer::timeout, this, &Live2DView::onParamValuesUpdated);

    syncTimer.start();
}

Live2DView::~Live2DView()
{
}

void Live2DView::initExpressions(Model *model)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidget);
    item->setText(0, "Expressions");

    model->GetExpressions(item,
                          [](void *collector, const char *id, const char *file)
                          {
                              QTreeWidgetItem *topLevel = (QTreeWidgetItem *)collector;
                              QTreeWidgetItem *item = new QTreeWidgetItem(topLevel);
                              QFileInfo info(file);
                              item->setText(0, info.fileName());
                              item->setData(0, Qt::UserRole, QVariant(id));
                          });
}

void Live2DView::initMotions(Model *model)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidget);
    item->setText(0, "Motions");
    ui.treeWidget->addTopLevelItem(item);

    model->GetMotions(item,
                      [](void *collector, const char *group, int no, const char *file)
                      {
                          QTreeWidgetItem *topLevel = (QTreeWidgetItem *)collector;
                          QTreeWidgetItem *item = new QTreeWidgetItem(topLevel);
                          QFileInfo info(file);
                          item->setText(0, info.fileName());
                          item->setData(0, Qt::UserRole, QVariant(group));
                          item->setData(0, Qt::UserRole + 1, QVariant(no));
                      });
}

void Live2DView::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item->parent() == nullptr)
    {
        return;
    }

    if (item->parent()->text(0) == "Expressions")
    {
        Model *model = ui.scene->GetModel();
        model->SetExpression(item->data(0, Qt::UserRole).toString().toStdString().c_str());
    }
    else if (item->parent()->text(0) == "Motions")
    {
        Model *model = ui.scene->GetModel();
        model->StartMotion(item->data(0, Qt::UserRole).toString().toStdString().c_str(), item->data(0, Qt::UserRole + 1).toInt());
    }
}

void Live2DView::initCdi(Model *model)
{
    QDir dir(model->GetModelHomeDir());
    QStringList files = dir.entryList(QStringList() << "*.cdi3.json", QDir::Files);
    if (files.size() > 0)
    {
        hasCdi = true;
        QFile file(dir.absoluteFilePath(files[0]));
        if (file.open(QIODevice::ReadOnly))
        {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            cdi = doc.object();
            file.close();
        }
    }
}

void Live2DView::initParameters(Model *model)
{
    void *ptrs[4] = {ui.paramTable, nullptr, model, paramValues};
    QJsonArray parameters;
    if (hasCdi)
    {
        parameters = cdi["Parameters"].toArray();
        ptrs[1] = &parameters;
    }
    model->GetParameterIds(ptrs,
                           [](void *collector, const char *id)
                           {
                               QTableWidget *table = (QTableWidget *)(((void **)collector)[0]);
                               QJsonArray *cdiParams = (QJsonArray *)(((void **)collector)[1]);
                               Model *model = (Model *)(((void **)collector)[2]);
                               QVector<ParamValue> *paramValues = (QVector<ParamValue> *)(((void **)collector)[3]);

                               QString name = id;
                               const int index = table->rowCount();
                               if (cdiParams)
                               {
                                   name = cdiParams->at(index).toObject()["Name"].toString();
                               }
                               float maxValue = model->GetParameterMaximumValue(index);
                               float minValue = model->GetParameterMinimumValue(index);
                               float defaultValue = model->GetParameterDefaultValue(index);

                               QLabel *label = new QLabel(name, table);
                               table->insertRow(index);
                               QTableWidgetItem *idItem = new QTableWidgetItem(id);
                               QTableWidgetItem *nameItem = new QTableWidgetItem(name);
                               QTableWidgetItem *valueItem = new QTableWidgetItem(QString::number(defaultValue));
                               QSlider *slider = new QSlider(Qt::Horizontal, table);
                               slider->setRange(0, 100);
                               slider->setValue((defaultValue - minValue) * 100 / (maxValue - minValue));
                               QObject::connect(slider, &QSlider::valueChanged, [=](int value)
                                                {
                                   const float v = minValue + (maxValue - minValue) * value / 100;
                                   valueItem->setText(QString::number(v, 'f', 2));
                                   if (slider->isSliderDown())
                                   {
                                        paramValues->push_back({index, v});
                                   } });
                               table->setItem(index, 0, idItem);
                               table->setItem(index, 1, nameItem);
                               table->setCellWidget(index, 2, slider);
                               table->setItem(index, 3, valueItem);
                           });
}

void Live2DView::initParts(Model *model)
{
    void *ptrs[2] = {ui.partList, nullptr};
    QJsonArray parts;
    if (hasCdi)
    {
        parts = cdi["Parts"].toArray();
        ptrs[1] = &parts;
    }
    model->GetPartIds(ptrs,
                      [](void *collector, const char *id)
                      {
                          QListWidget *list = (QListWidget *)(((void **)collector)[0]);
                          QJsonArray *cdiParts = (QJsonArray *)(((void **)collector)[1]);
                          if (cdiParts)
                          {
                              QString idStr = cdiParts->at(list->count()).toObject()["Name"].toString();
                              list->addItem(idStr);
                          }
                          else
                          {
                              list->addItem(id);
                          }
                      });
}

void Live2DView::initDrawables(Model *model)
{
    model->GetDrawableIds(ui.drawableList,
                          [](void *collector, const char *id)
                          {
                              QListWidget *list = (QListWidget *)collector;
                              list->addItem(id);
                          });
}

void Live2DView::onParamValuesUpdated()
{
    Model *model = ui.scene->GetModel();
    for (int i = 0; i < ui.paramTable->rowCount(); i++)
    {
        QSlider *slider = (QSlider *)ui.paramTable->cellWidget(i, 2);
        if (slider->isSliderDown())
        {
            return;
        }
        slider->setValue(100 * (model->GetParameterValue(i) - model->GetParameterMinimumValue(i)) / (model->GetParameterMaximumValue(i) - model->GetParameterMinimumValue(i)));
    }
}