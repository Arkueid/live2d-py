/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "LAppWavFileHandler.hpp"
#include <cmath>
#include <cstdint>
#include "LAppPal.hpp"

LAppWavFileHandler::LAppWavFileHandler()
    : _rawData(NULL)
    , _pcmData(NULL)
    , _userTimeSeconds(0.0f)
    , _lastRms(0.0f)
    , _sampleOffset(0)
{
}

LAppWavFileHandler::~LAppWavFileHandler()
{
    if (_rawData != NULL)
    {
        CSM_FREE(_rawData);
    }

    if (_pcmData != NULL)
    {
        ReleasePcmData();
    }
}

Csm::csmBool LAppWavFileHandler::Update(Csm::csmFloat32 deltaTimeSeconds)
{
    Csm::csmUint32 goalOffset;
    Csm::csmFloat32 rms;

    // データロード前/ファイル末尾に達した場合は更新しない
    if ((_pcmData == NULL)
        || (_sampleOffset >= _wavFileInfo._samplesPerChannel))
    {
        _lastRms = 0.0f;
        return false;
    }

    // 経過時間後の状態を保持
    _userTimeSeconds += deltaTimeSeconds;
    goalOffset = static_cast<Csm::csmUint32>(_userTimeSeconds * _wavFileInfo._samplingRate);
    if (goalOffset > _wavFileInfo._samplesPerChannel)
    {
        goalOffset = _wavFileInfo._samplesPerChannel;
    }

    // RMS計測
    rms = 0.0f;
    for (Csm::csmUint32 channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
    {
        for (Csm::csmUint32 sampleCount = _sampleOffset; sampleCount < goalOffset; sampleCount++)
        {
            Csm::csmFloat32 pcm = _pcmData[channelCount][sampleCount];
            rms += pcm * pcm;
        }
    }
    rms = sqrt(rms / (_wavFileInfo._numberOfChannels * (goalOffset - _sampleOffset)));

    _lastRms = rms;
    _sampleOffset = goalOffset;
    return true;
}

void LAppWavFileHandler::Start(const Csm::csmString& filePath)
{
    // WAVファイルのロード
    if (!LoadWavFile(filePath))
    {
        return;
    }

    // サンプル参照位置を初期化
    _sampleOffset = 0;
    _userTimeSeconds = 0.0f;

    // RMS値をリセット
    _lastRms = 0.0f;
}

Csm::csmFloat32 LAppWavFileHandler::GetRms() const
{
    return _lastRms;
}

const LAppWavFileHandler::WavFileInfo& LAppWavFileHandler::GetWavFileInfo() const
{
    return _wavFileInfo;
}

const Csm::csmByte* LAppWavFileHandler::GetRawData() const
{
    return _rawData;
}

Csm::csmUint64 LAppWavFileHandler::GetRawDataSize() const
{
    return _rawDataSize;
}

Csm::csmVector<Csm::csmFloat32> LAppWavFileHandler::GetPcmData() const
{
    Csm::csmVector<Csm::csmFloat32> buffer;

    for (Csm::csmUint32 sampleCount = 0; sampleCount < _wavFileInfo._samplesPerChannel; sampleCount++)
    {
        for (Csm::csmUint32 channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
        {
            buffer.PushBack(_pcmData[channelCount][sampleCount]);
        }
    }

    return buffer;
}

void LAppWavFileHandler::GetPcmDataChannel(Csm::csmFloat32* dst, Csm::csmUint32 useChannel) const
{
    for (Csm::csmUint32 sampleCount = 0; sampleCount < _wavFileInfo._samplesPerChannel; sampleCount++)
    {
        dst[sampleCount] = _pcmData[useChannel][sampleCount];
    }
}

Csm::csmFloat32 LAppWavFileHandler::NormalizePcmSample(Csm::csmUint32 bitsPerSample, Csm::csmByte* data, Csm::csmUint32 dataSize)
{
    Csm::csmInt32 pcm32;

    // 32ビット幅に拡張してから-1～1の範囲に丸める
    switch (bitsPerSample)
    {
    case 8:
        if (1 <= dataSize)
        {
            const Csm::csmUint8 ret = data[0];
            pcm32 = static_cast<Csm::csmInt32>(ret) - 128;
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
            const Csm::csmUint16 ret = (data[1] << 8) | data[0];
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
            const Csm::csmUint32 ret = (data[2] << 16) | (data[1] << 8) | data[0];
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
            const Csm::csmUint32 ret = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
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

    return static_cast<Csm::csmFloat32>(pcm32) / INT32_MAX;
}

Csm::csmBool LAppWavFileHandler::LoadWavFile(const Csm::csmString& filePath)
{
    Csm::csmBool ret;

    // 既にwavファイルロード済みならば領域開放
    if (_rawData != NULL)
    {
        CSM_FREE(_rawData);
        _rawDataSize = 0;
    }
    if (_pcmData != NULL)
    {
        ReleasePcmData();
    }

    // ファイルロード
    _byteReader._fileByte = LAppPal::LoadFileAsBytes(filePath.GetRawString(), &(_byteReader._fileSize));
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
        const Csm::csmUint32 fmtChunkSize = _byteReader.Get32LittleEndian();
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
            const Csm::csmUint32 dataChunkSize = _byteReader.Get32LittleEndian();
            _wavFileInfo._samplesPerChannel = (dataChunkSize * 8) / (_wavFileInfo._bitsPerSample * _wavFileInfo._numberOfChannels);
        }
        // 領域確保
        _rawDataSize = static_cast<Csm::csmUint64>(_wavFileInfo._blockAlign) * static_cast<Csm::csmUint64>(_wavFileInfo._samplesPerChannel);
        _rawData = static_cast<Csm::csmByte*>(CSM_MALLOC(sizeof(Csm::csmByte) * _rawDataSize));
        _pcmData = static_cast<Csm::csmFloat32**>(CSM_MALLOC(sizeof(Csm::csmFloat32*) * _wavFileInfo._numberOfChannels));
        for (Csm::csmUint32 channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
        {
            _pcmData[channelCount] = static_cast<Csm::csmFloat32*>(CSM_MALLOC(sizeof(Csm::csmFloat32) * _wavFileInfo._samplesPerChannel));
        }
        // 波形データ取得
        Csm::csmUint64 rawPos = 0;
        for (Csm::csmUint32 sampleCount = 0; sampleCount < _wavFileInfo._samplesPerChannel; sampleCount++)
        {
            for (Csm::csmUint32 channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
            {
                // 正規化前
                for (Csm::csmUint32 byteCount = 0; byteCount < _wavFileInfo._bitsPerSample / 8; byteCount++)
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
    LAppPal::ReleaseBytes(_byteReader._fileByte);
    _byteReader._fileByte = NULL;
    _byteReader._fileSize = 0;

    return ret;
}

Csm::csmFloat32 LAppWavFileHandler::GetPcmSample()
{
    Csm::csmInt32 pcm32;

    // 32ビット幅に拡張してから-1～1の範囲に丸める
    switch (_wavFileInfo._bitsPerSample)
    {
    case 8:
        pcm32 = static_cast<Csm::csmInt32>(_byteReader.Get8()) - 128;
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

    return static_cast<Csm::csmFloat32>(pcm32) / INT32_MAX;
}

void LAppWavFileHandler::ReleasePcmData()
{
    for (Csm::csmUint32 channelCount = 0; channelCount < _wavFileInfo._numberOfChannels; channelCount++)
    {
        CSM_FREE(_pcmData[channelCount]);
    }
    CSM_FREE(_pcmData);
    _pcmData = NULL;
}
