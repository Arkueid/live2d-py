/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LAppPal.hpp"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <Model/CubismMoc.hpp>

#include <Log.hpp>

#include <filesystem>

using namespace Csm;

csmByte* LAppPal::LoadFileAsBytes(const std::string filePath, csmSizeInt* outSize)
{
    //filePath;//
    const char* pathStr = filePath.c_str();
    std::filesystem::path path = std::filesystem::u8path(filePath);

    size_t size = 0;
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

    if (outSize)
    {
        *outSize = static_cast<unsigned int>(size);
    }

    return reinterpret_cast<csmByte*>(buf);
}

void LAppPal::ReleaseBytes(csmByte* byteData)
{
    delete[] byteData;
}

void LAppPal::PrintLn(const Csm::csmChar *message)
{
    Info(message);
}

double LAppPal::GetCurrentTimePoint()
{
    return std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
}
