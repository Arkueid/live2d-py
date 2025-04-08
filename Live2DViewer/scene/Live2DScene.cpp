#include "Live2DScene.hpp"

#include <QDateTime>
#include <QMouseEvent>

static bool gladLoaded = false;
static int fps = 40;

Live2DScene::Live2DScene(QWidget *parent) : QOpenGLWidget(parent), lastUpdateTime(-1), model(nullptr), paramValues(), autoBlink(true), autoBreath(true)
{
}

Live2DScene::~Live2DScene()
{
    delete model;
}

void Live2DScene::LoadModel(const QString &filePath)
{
    model = new Model();
    model->LoadModelJson(filePath.toStdString().c_str());
}

Model *Live2DScene::GetModel()
{
    return model;
}

QVector<ParamValue> *Live2DScene::GetParamValues()
{
    return &paramValues;
}

void Live2DScene::setAutoBlink(bool value)
{
    autoBlink = value;
}

void Live2DScene::setAutoBreath(bool value)
{
    autoBreath = value;
}

void Live2DScene::setAutoPhysics(bool value)
{
    autoPhysics = value;
}

void Live2DScene::timerEvent(QTimerEvent *event)
{
    update();
}

void Live2DScene::initializeGL()
{
    initializeOpenGLFunctions();

    if (!gladLoaded)
    {
        gladLoadGL();
        gladLoaded = true;
    }

    model->CreateRenderer();

    lastUpdateTime = QDateTime::currentMSecsSinceEpoch();

    startTimer(1000 / fps);
}

void Live2DScene::paintGL()
{
    if (lastUpdateTime < 0)
    {
        return;
    }

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    long long currentTime = QDateTime::currentMSecsSinceEpoch();
    float deltaTime = float(((double)QDateTime::currentMSecsSinceEpoch() - (double)lastUpdateTime) / 1000.0);
    lastUpdateTime = currentTime;

    bool motionUpdated = false;
    model->LoadParameters();
    if (!model->IsMotionFinished())
    {
        motionUpdated = model->UpdateMotion(deltaTime);
    }

    for (auto &param : paramValues)
    {
        model->SetParameterValue(param.index, param.value);
    }
    
    model->SaveParameters();

    if (!motionUpdated && autoBlink)
    {
        model->UpdateBlink(deltaTime);
    }
    model->UpdateExpression(deltaTime);

    model->UpdateDrag(deltaTime);

    if (autoBreath)
    {
        model->UpdateBreath(deltaTime);
    }

    paramValues.clear();   

    if (autoPhysics)
    {
        model->UpdatePhysics(deltaTime);    
    }

    model->UpdatePose(deltaTime);
    model->Draw();

    emit paramValuesUpdated();
}

void Live2DScene::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    model->Resize(w, h);
}

void Live2DScene::mouseMoveEvent(QMouseEvent *event)
{
    model->Drag((float)event->x(), (float)event->y());
}

void Live2DScene::mousePressEvent(QMouseEvent *event)
{
}

void Live2DScene::mouseReleaseEvent(QMouseEvent *event)
{
    // 复位
    model->Drag(width() / 2, height() / 2);
}
