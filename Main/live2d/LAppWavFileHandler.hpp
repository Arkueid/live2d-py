/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include <CubismFramework.hpp>
#include <Type/csmVector.hpp>

 /**
  * @brief wavファイルハンドラ
  * @attention 16bit wav ファイル読み込みのみ実装済み
  */
class LAppWavFileHandler
{
public:
    /**
     * @brief 読み込んだwavfileの情報
     */
    struct WavFileInfo
    {
        /**
         * @brief コンストラクタ
         */
        WavFileInfo() : _fileName(""), _numberOfChannels(0),
            _bitsPerSample(0), _samplingRate(0), _samplesPerChannel(0),
            _avgBytesPerSec(0), _blockAlign(0)
        { }

        Csm::csmString _fileName; ///< ファイル名
        Csm::csmUint32 _numberOfChannels; ///< チャンネル数
        Csm::csmUint32 _bitsPerSample; ///< サンプルあたりビット数
        Csm::csmUint32 _samplingRate; ///< サンプリングレート
        Csm::csmUint32 _samplesPerChannel; ///< 1チャンネルあたり総サンプル数
        Csm::csmUint32 _avgBytesPerSec; ///< 平均データ速度
        Csm::csmUint32 _blockAlign; ///< ブロックサイズ
    } _wavFileInfo;

    /**
     * @brief コンストラクタ
     */
    LAppWavFileHandler();

    /**
     * @brief デストラクタ
     */
    ~LAppWavFileHandler();

    /**
     * @brief wavファイルハンドラの内部状態更新
     *
     * @param[in]   deltaTimeSeconds    デルタ時間[秒]
     * @retval  true    更新されている
     * @retval  false   更新されていない
     */
    Csm::csmBool Update(Csm::csmFloat32 deltaTimeSeconds);

    /**
     * @brief 引数で指定したwavファイルの読み込みを開始する
     *
     * @param[in] filePath wavファイルのパス
     */
    void Start(const Csm::csmString& filePath);

    /**
     * @brief 現在のRMS値取得
     *
     * @retval  csmFloat32 RMS値
     */
    Csm::csmFloat32 GetRms() const;

    /**
     * @brief ファイル情報を取得
     *
     * @retval  ファイル情報
     */
    const WavFileInfo& GetWavFileInfo() const;

    /**
     * @brief 正規化前のデータを取得
     *
     * @retval  正規化前のデータ
     */
    const Csm::csmByte* GetRawData() const;

    /**
     * @brief 正規化前のデータの大きさを取得
     *
     * @retval  正規化前のデータの大きさ
     */
    Csm::csmUint64 GetRawDataSize() const;

    /**
     * @brief 正規化データを取得する
     *
     * @retval 正規化データ
     */
    Csm::csmVector<Csm::csmFloat32> GetPcmData() const;

    /**
     * @brief 引数で指定したチャンネルの正規化データを取得する
     *
     * @param[in] dst 格納先
     * @param[in] useChannel 使用するチャンネル
     */
    void GetPcmDataChannel(Csm::csmFloat32* dst, Csm::csmUint32 useChannel) const;

    /**
     * @brief -1～1の範囲の1サンプル取得
     *
     * @param[in] bitsPerSample ビット深度
     * @param[in] data 正規化したいデータ
     * @param[in] dataSize 正規化したいデータの大きさ
     *
     * @retval    csmFloat32    正規化されたサンプル
     */
    static Csm::csmFloat32 NormalizePcmSample(Csm::csmUint32 bitsPerSample, Csm::csmByte* data, Csm::csmUint32 dataSize);

private:
    /**
     * @brief wavファイルのロード
     *
     * @param[in] filePath wavファイルのパス
     * @retval  true    読み込み成功
     * @retval  false   読み込み失敗
     */
    Csm::csmBool LoadWavFile(const Csm::csmString& filePath);

    /**
     * @brief PCMデータの解放
     */
    void ReleasePcmData();

    /**
     * @brief -1～1の範囲の1サンプル取得
     * @retval    csmFloat32    正規化されたサンプル
     */
    Csm::csmFloat32 GetPcmSample();

    /**
     * @brief バイトリーダ
     */
    struct ByteReader {
        /**
         * @brief コンストラクタ
         */
        ByteReader() : _fileByte(NULL), _fileSize(0), _readOffset(0)
        { }

        /**
         * @brief 8ビット読み込み
         * @return Csm::csmUint8 読み取った8ビット値
         */
        Csm::csmUint8 Get8()
        {
            const Csm::csmUint8 ret = _fileByte[_readOffset];
            _readOffset++;
            return ret;
        }

        /**
         * @brief 16ビット読み込み（リトルエンディアン）
         * @return Csm::csmUint16 読み取った16ビット値
         */
        Csm::csmUint16 Get16LittleEndian()
        {
            const Csm::csmUint16 ret = (_fileByte[_readOffset + 1] << 8) | _fileByte[_readOffset];
            _readOffset += 2;
            return ret;
        }

        /**
         * @brief 24ビット読み込み（リトルエンディアン）
         * @return Csm::csmUint32 読み取った24ビット値（下位24ビットに設定）
         */
        Csm::csmUint32 Get24LittleEndian()
        {
            const Csm::csmUint32 ret =
                (_fileByte[_readOffset + 2] << 16) | (_fileByte[_readOffset + 1] << 8)
                | _fileByte[_readOffset];
            _readOffset += 3;
            return ret;
        }

        /**
         * @brief 32ビット読み込み（リトルエンディアン）
         * @return Csm::csmUint32 読み取った32ビット値
         */
        Csm::csmUint32 Get32LittleEndian()
        {
            const Csm::csmUint32 ret =
                (_fileByte[_readOffset + 3] << 24) | (_fileByte[_readOffset + 2] << 16)
                | (_fileByte[_readOffset + 1] << 8) | _fileByte[_readOffset];
            _readOffset += 4;
            return ret;
        }

        /**
         * @brief シグネチャの取得と参照文字列との一致チェック
         * @param[in] reference 検査対象のシグネチャ文字列
         * @retval  true    一致している
         * @retval  false   一致していない
         */
        Csm::csmBool GetCheckSignature(const Csm::csmString& reference)
        {
            Csm::csmChar getSignature[4] = { 0, 0, 0, 0 };
            const Csm::csmChar* referenceString = reference.GetRawString();
            if (reference.GetLength() != 4)
            {
                return false;
            }
            for (Csm::csmUint32 signatureOffset = 0; signatureOffset < 4; signatureOffset++)
            {
                getSignature[signatureOffset] = static_cast<Csm::csmChar>(Get8());
            }
            return (getSignature[0] == referenceString[0]) && (getSignature[1] == referenceString[1])
                && (getSignature[2] == referenceString[2]) && (getSignature[3] == referenceString[3]);
        }

        Csm::csmByte* _fileByte; ///< ロードしたファイルのバイト列
        Csm::csmSizeInt _fileSize; ///< ファイルサイズ
        Csm::csmUint32 _readOffset; ///< ファイル参照位置
    } _byteReader;

    Csm::csmByte* _rawData; ///< 正規化される前のバイト列
    Csm::csmUint64 _rawDataSize; ///< 正規化される前のバイト列の大きさ
    Csm::csmFloat32** _pcmData; ///< -1から1の範囲で表現された音声データ配列
    Csm::csmUint32 _sampleOffset; ///< サンプル参照位置
    Csm::csmFloat32 _lastRms; ///< 最後に計測したRMS値
    Csm::csmFloat32 _userTimeSeconds; ///< デルタ時間の積算値[秒]
 };
