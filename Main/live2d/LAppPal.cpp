/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LAppPal.hpp"
#include <cstdio>
#include <stdarg.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <Model/CubismMoc.hpp>
#include "LAppDefine.hpp"

#include <chrono>
#include <Log.hpp>

#include <filesystem>

using std::endl;
using namespace Csm;
using namespace std;
using namespace LAppDefine;

double LAppPal::s_currentFrame = 0.0;
double LAppPal::s_lastFrame = 0.0;
double LAppPal::s_deltaTime = 0.0;


csmByte* LAppPal::LoadFileAsBytes(const string filePath, csmSizeInt* outSize)
{
    //filePath;//
    const char* pathStr = filePath.c_str();
    std::filesystem::path path = std::filesystem::u8path(filePath);

    int size = 0;
    if (std::filesystem::exists(path))
    {
        size = std::filesystem::file_size(path);
        if (size == 0)
        {
            Info("Stat succeeded but file size is zero. path:%s", pathStr);
            return NULL;
        }
    }
    else
    {
        Info("Stat failed. errno:%d path:%s", errno, pathStr);
        return NULL;
    }

    std::fstream file;
    file.open(path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        Info("File open failed. path:%s", pathStr);
        return NULL;
    }

    char* buf = new char[size];
    file.read(buf, size);
    file.close();

    if(outSize) *outSize = size;
    
    return reinterpret_cast<csmByte*>(buf);
}

void LAppPal::ReleaseBytes(csmByte* byteData)
{
    delete[] byteData;
}

csmFloat32  LAppPal::GetDeltaTime()
{
    return static_cast<csmFloat32>(s_deltaTime);
}

void LAppPal::UpdateTime()
{
    s_currentFrame = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
    s_deltaTime = s_currentFrame - s_lastFrame;
    s_lastFrame = s_currentFrame;
}

void LAppPal::PrintLn(const Csm::csmChar *message)
{
    Info(message);
}
