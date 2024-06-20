/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LAppWavFileHandler.hpp"
#include <cmath>
#include <cstdint>
#include "FileManager.h"

LAppWavFileHandler::LAppWavFileHandler()
    : _rawData(NULL)
    , _pcmData(NULL)
    , _startUserTime(0.0f)
    , _lastRms(0.0f)
    , _sampleOffset(0)
{
}

LAppWavFileHandler::~LAppWavFileHandler()
{
    if (_rawData != NULL)
    {
        free(_rawData);
    }

    if (_pcmData != NULL)
    {
        ReleasePcmData();
    }
}

bool LAppWavFileHandler::Update(float deltaTimeSeconds)
{
    unsigned int goalOffset;
    float rms;

    // データロード前/ファイル末尾に達した場合は更新しない
    if ((_pcmData == NULL)
        || (_sampleOffset >= _wavFileInfo._samplesPerChannel))
    {
        _lastRms = 0.0f;
        return false;
    }

    // 経過時間後の状態を保持
    float timeSlip = deltaTimeSeconds - _startUserTime;
    goalOffset = static_cast<unsigned int>(timeSlip * _wavFileInfo._samplingRate);
    if (goalOffset > _wavFileInfo._samplesPerChannel)
    {
        goalOffset = _wavFileInfo._samplesPerChannel;
    }

    // RMS計測
    rms = 0.0f;
    for (unsigned int channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
    {
        for (unsigned int sampleCount = _sampleOffset; sampleCount < goalOffset; sampleCount++)
        {
            float pcm = _pcmData[channelCount][sampleCount];
            rms += pcm * pcm;
        }
    }
    rms = sqrt(rms / (_wavFileInfo._numberOfChannels * (goalOffset - _sampleOffset)));

    _lastRms = rms;
    _sampleOffset = goalOffset;
    return true;
}

void LAppWavFileHandler::Start(const std::string& filePath, float startUserTime)
{
    // WAVファイルのロード
    if (!LoadWavFile(filePath))
    {
        return;
    }

    // サンプル参照位置を初期化
    _sampleOffset = 0;
    _startUserTime = startUserTime;

    // RMS値をリセット
    _lastRms = 0.0f;
}

float LAppWavFileHandler::GetRms() const
{
    return _lastRms;
}

const LAppWavFileHandler::WavFileInfo& LAppWavFileHandler::GetWavFileInfo() const
{
    return _wavFileInfo;
}

const unsigned char* LAppWavFileHandler::GetRawData() const
{
    return _rawData;
}

unsigned long LAppWavFileHandler::GetRawDataSize() const
{
    return _rawDataSize;
}

std::vector<float> LAppWavFileHandler::GetPcmData() const
{
    std::vector<float> buffer;

    for (unsigned int sampleCount = 0; sampleCount < _wavFileInfo._samplesPerChannel; sampleCount++)
    {
        for (unsigned int channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
        {
            buffer.push_back(_pcmData[channelCount][sampleCount]);
        }
    }

    return buffer;
}

void LAppWavFileHandler::GetPcmDataChannel(float* dst, unsigned int useChannel) const
{
    for (unsigned int sampleCount = 0; sampleCount < _wavFileInfo._samplesPerChannel; sampleCount++)
    {
        dst[sampleCount] = _pcmData[useChannel][sampleCount];
    }
}

float LAppWavFileHandler::NormalizePcmSample(unsigned int bitsPerSample, unsigned char* data, unsigned int dataSize)
{
    int pcm32;

    // 32ビット幅に拡張してから-1～1の範囲に丸める
    switch (bitsPerSample)
    {
    case 8:
        if (1 <= dataSize)
        {
            const unsigned char ret = data[0];
            pcm32 = static_cast<int>(ret) - 128;
            pcm32 <<= 24;
        }
        else
        {
            pcm32 = 0;
        }
        break;
    case 16:
        if (2 <= dataSize)
        {
            const unsigned short ret = (data[1] << 8) | data[0];
            pcm32 = ret << 16;
        }
        else
        {
            pcm32 = 0;
        }
        break;
    case 24:
        if (3 <= dataSize)
        {
            const unsigned int ret = (data[2] << 16) | (data[1] << 8) | data[0];
            pcm32 = ret << 8;
        }
        else
        {
            pcm32 = 0;
        }
        break;
    case 32:
        if (4 <= dataSize)
        {
            const unsigned int ret = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
            pcm32 = ret << 0;
        }
        else
        {
            pcm32 = 0;
        }
        break;
    default:
        // 対応していないビット幅
        pcm32 = 0;
        break;
    }

    return static_cast<float>(pcm32) / INT32_MAX;
}

bool LAppWavFileHandler::LoadWavFile(const std::string& filePath)
{
    bool ret;

    // 既にwavファイルロード済みならば領域開放
    if (_rawData != NULL)
    {
        free(_rawData);
        _rawDataSize = 0;
    }
    if (_pcmData != NULL)
    {
        ReleasePcmData();
    }

    // ファイルロード
    _byteReader._fileByte = FileManager::loadFile(filePath.c_str(), &(_byteReader._fileSize));
    _byteReader._readOffset = 0;

    // ファイルロードに失敗しているか、先頭のシグネチャ"RIFF"を入れるサイズもない場合は失敗
    if ((_byteReader._fileByte == NULL) || (_byteReader._fileSize < 4))
    {
        return false;
    }

    // ファイル名
    _wavFileInfo._fileName = filePath;

    do {
        // シグネチャ "RIFF"
        if (!_byteReader.GetCheckSignature("RIFF"))
        {
            ret = false;
            break;
        }
        // ファイルサイズ-8（読み飛ばし）
        _byteReader.Get32LittleEndian();
        // シグネチャ "WAVE"
        if (!_byteReader.GetCheckSignature("WAVE"))
        {
            ret = false;
            break;
        }
        // シグネチャ "fmt "
        if (!_byteReader.GetCheckSignature("fmt "))
        {
            ret = false;
            break;
        }
        // fmtチャンクサイズ
        const unsigned int fmtChunkSize = _byteReader.Get32LittleEndian();
        // フォーマットIDは1（リニアPCM）以外受け付けない
        if (_byteReader.Get16LittleEndian() != 1)
        {
            ret = false;
            break;
        }
        // チャンネル数
        _wavFileInfo._numberOfChannels = _byteReader.Get16LittleEndian();
        // サンプリングレート
        _wavFileInfo._samplingRate = _byteReader.Get32LittleEndian();
        // 平均データ速度
        _wavFileInfo._avgBytesPerSec = _byteReader.Get32LittleEndian();
        // ブロックサイズ
        _wavFileInfo._blockAlign = _byteReader.Get16LittleEndian();
        // 量子化ビット数
        _wavFileInfo._bitsPerSample = _byteReader.Get16LittleEndian();
        // fmtチャンクの拡張部分の読み飛ばし
        if (fmtChunkSize > 16)
        {
            _byteReader._readOffset += (fmtChunkSize - 16);
        }
        // "data"チャンクが出現するまで読み飛ばし
        while (!(_byteReader.GetCheckSignature("data"))
            && (_byteReader._readOffset < _byteReader._fileSize))
        {
            _byteReader._readOffset += _byteReader.Get32LittleEndian();
        }
        // ファイル内に"data"チャンクが出現しなかった
        if (_byteReader._readOffset >= _byteReader._fileSize)
        {
            ret = false;
            break;
        }
        // サンプル数
        {
            const unsigned int dataChunkSize = _byteReader.Get32LittleEndian();
            _wavFileInfo._samplesPerChannel = (dataChunkSize * 8) / (_wavFileInfo._bitsPerSample * _wavFileInfo._numberOfChannels);
        }
        // 領域確保
        _rawDataSize = static_cast<unsigned long>(_wavFileInfo._blockAlign) * static_cast<unsigned long>(_wavFileInfo._samplesPerChannel);
        _rawData = static_cast<unsigned char*>(malloc(sizeof(unsigned char) * _rawDataSize));
        _pcmData = static_cast<float**>(malloc(sizeof(float*) * _wavFileInfo._numberOfChannels));
        for (unsigned int channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
        {
            _pcmData[channelCount] = static_cast<float*>(malloc(sizeof(float) * _wavFileInfo._samplesPerChannel));
        }
        // 波形データ取得
        unsigned long rawPos = 0;
        for (unsigned int sampleCount = 0; sampleCount < _wavFileInfo._samplesPerChannel; sampleCount++)
        {
            for (unsigned int channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
            {
                // 正規化前
                for (unsigned int byteCount = 0; byteCount < _wavFileInfo._bitsPerSample / 8; byteCount++)
                {
                    _rawData[rawPos++] = _byteReader._fileByte[_byteReader._readOffset + byteCount];
                }
                // 正規化後
                _pcmData[channelCount][sampleCount] = GetPcmSample();
            }
        }

        ret = true;

    }  while (false);

    // ファイル開放
    FileManager::releaseBuffer(_byteReader._fileByte);
    _byteReader._fileByte = NULL;
    _byteReader._fileSize = 0;

    return ret;
}

float LAppWavFileHandler::GetPcmSample()
{
    int pcm32;

    // 32ビット幅に拡張してから-1～1の範囲に丸める
    switch (_wavFileInfo._bitsPerSample)
    {
    case 8:
        pcm32 = static_cast<int>(_byteReader.Get8()) - 128;
        pcm32 <<= 24;
        break;
    case 16:
        pcm32 = _byteReader.Get16LittleEndian() << 16;
        break;
    case 24:
        pcm32 = _byteReader.Get24LittleEndian() << 8;
        break;
    default:
        // 対応していないビット幅
        pcm32 = 0;
        break;
    }

    return static_cast<float>(pcm32) / INT32_MAX;
}

void LAppWavFileHandler::ReleasePcmData()
{
    for (unsigned int channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
    {
        free(_pcmData[channelCount]);
    }
    free(_pcmData);
    _pcmData = NULL;
}
