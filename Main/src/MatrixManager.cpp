#include "MatrixManager.hpp"
#include "LAppModel.hpp"

MatrixManager::MatrixManager(): _offsetX(0.0f), _offsetY(0.0f), _scale(1.0f),
                                _ww(800),
                                _wh(600)
{
}

// call when scene is resized
void MatrixManager::UpdateScreenToScene(int width, int height)
{
    _ww = width;
    _wh = height;

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

void MatrixManager::ScreenToScene(float* x, float* y)
{
    *x = _screenToScene.TransformX(*x);
    *y = _screenToScene.TransformY(*y);
}

Csm::CubismMatrix44& MatrixManager::GetMvp(LAppModel* model)
{
    _mvp.LoadIdentity();

    if (model->GetModel()->GetCanvasWidth() > 1.0f && _ww < _wh)
    {
        // 横に長いモデルを縦長ウィンドウに表示する際モデルの横サイズでscaleを算出する
        model->GetModelMatrix()->SetWidth(2.0f);
        _mvp.Scale(1.0f, static_cast<float>(_ww) / static_cast<float>(_wh));
    }
    else
    {
        _mvp.Scale(static_cast<float>(_wh) / static_cast<float>(_ww), 1.0f);
    }

    Csm::CubismModelMatrix* m = model->GetModelMatrix();

    m->Scale(_scale, _scale);

    m->SetX(_offsetX);
    m->SetY(_offsetY);

    _mvp.MultiplyByMatrix(m);

    return _mvp;
}

void MatrixManager::SetOffset(float x, float y)
{
    _offsetX = x;
    _offsetY = y;
}

void MatrixManager::SetScale(float scale)
{
    _scale = scale;
}
