#include "MatrixManager.hpp"
#include <cmath>

void MatrixManager::Initialize()
{
    _offset.LoadIdentity();
    _scale = 1.0f;
}

// call when scene is resized
void MatrixManager::UpdateScreenToScene(int width, int height)
{

    float ratio = static_cast<float>(width) / static_cast<float>(height);
    float left = -ratio;
    float right = ratio;
    float bottom = -1.0f;
    float top = 1.0f;

    _screenToScene.LoadIdentity(); // サイズが変わった際などリセット必須
    if (width > height)
    {
        float screenW = fabsf(right - left);
        _screenToScene.ScaleRelative(screenW / width, -screenW / width);
    }
    else
    {
        float screenH = fabsf(top - bottom);
        _screenToScene.ScaleRelative(screenH / height, -screenH / height);
    }
    _screenToScene.TranslateRelative(-width * 0.5f, -height * 0.5f);
    
}

void MatrixManager::UpdateProjection(LAppModel *model, int ww, int wh)
{
    _projection.LoadIdentity();

    if (model->GetModel()->GetCanvasWidth() > 1.0f && ww < wh)
    {
        // 横に長いモデルを縦長ウィンドウに表示する際モデルの横サイズでscaleを算出する
        model->GetModelMatrix()->SetWidth(2.0f);
        _projection.Scale(1.0f, static_cast<float>(ww) / static_cast<float>(wh));
    }
    else
    {
        _projection.Scale(static_cast<float>(wh) / static_cast<float>(ww), 1.0f);
    }
}

void MatrixManager::ScreenToScene(float *x, float *y)
{
    *x = _screenToScene.TransformX(*x);
    *y = _screenToScene.TransformY(*y);
}

Csm::CubismMatrix44 &MatrixManager::GetProjection()
{
    _projection.ScaleRelative(_scale, _scale);
    _projection.MultiplyByMatrix(&_offset);
    return _projection;
}

void MatrixManager::SetOffset(float dx, float dy)
{
    _offset.LoadIdentity();
    _offset.Translate(dx, dy);
}

void MatrixManager::SetScale(float scale)
{
    _scale = scale;
}