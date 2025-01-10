#pragma once

#include <CubismFramework.hpp>

#include "Math/CubismMatrix44.hpp"

class LAppModel;

class MatrixManager
{
public:
    MatrixManager();
    void UpdateScreenToScene(int ww, int wh);
    void ScreenToScene(float *x, float *y);
    Csm::CubismMatrix44 &GetMvp(LAppModel *model);
    void SetOffset(float x, float y);
    void SetScale(float scale);
    void InvertTransform(float* x, float* y);
private:
    Csm::CubismMatrix44 _screenToScene;
    Csm::CubismMatrix44 _mvp;
    float _offsetX;
    float _offsetY;
    float _scale;
    int _ww;
    int _wh;
    float _baseScaleX;
    float _baseScaleY;
};