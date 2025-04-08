#pragma once

#include "ui_Live2DView.h"

#include <QJsonObject>
#include <QTimer>


class Live2DView : public QWidget
{
    Q_OBJECT

    void initExpressions(Model *model);
    void initMotions(Model *model);
    
    void initCdi(Model *model);

    void initParameters(Model *model);
    void initParts(Model *model);
    void initDrawables(Model *model);

private slots:
    void onTreeItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onParamValuesUpdated();

public:
    Live2DView(const QString& filePath, QWidget *parent = nullptr);
    ~Live2DView() override;

private:
    Ui::Live2DView ui;

    bool hasCdi;
    QJsonObject cdi;

    QVector<ParamValue>* paramValues;

    QTimer syncTimer;
};