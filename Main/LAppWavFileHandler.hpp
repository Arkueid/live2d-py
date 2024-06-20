/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once
#include <string>
#include <vector>

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

        std::string _fileName; ///< ファイル名
        unsigned int _numberOfChannels; ///< チャンネル数
        unsigned int _bitsPerSample; ///< サンプルあたりビット数
        unsigned int _samplingRate; ///< サンプリングレート
        unsigned int _samplesPerChannel; ///< 1チャンネルあたり総サンプル数
        unsigned int _avgBytesPerSec; ///< 平均データ速度
        unsigned int _blockAlign; ///< ブロックサイズ
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
    bool Update(float deltaTimeSeconds);

    /**
     * @brief 引数で指定したwavファイルの読み込みを開始する
     *
     * @param[in] filePath wavファイルのパス
     */
    void Start(const std::string& filePath, float startUserTime);

    /**
     * @brief 現在のRMS値取得
     *
     * @retval  csmFloat32 RMS値
     */
    float GetRms() const;

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
    const unsigned char* GetRawData() const;

    /**
     * @brief 正規化前のデータの大きさを取得
     *
     * @retval  正規化前のデータの大きさ
     */
    unsigned long GetRawDataSize() const;

    /**
     * @brief 正規化データを取得する
     *
     * @retval 正規化データ
     */
    std::vector<float> GetPcmData() const;

    /**
     * @brief 引数で指定したチャンネルの正規化データを取得する
     *
     * @param[in] dst 格納先
     * @param[in] useChannel 使用するチャンネル
     */
    void GetPcmDataChannel(float* dst, unsigned int useChannel) const;

    /**
     * @brief -1～1の範囲の1サンプル取得
     *
     * @param[in] bitsPerSample ビット深度
     * @param[in] data 正規化したいデータ
     * @param[in] dataSize 正規化したいデータの大きさ
     *
     * @retval    csmFloat32    正規化されたサンプル
     */
    static float NormalizePcmSample(unsigned int bitsPerSample, unsigned char* data, unsigned int dataSize);

private:
    /**
     * @brief wavファイルのロード
     *
     * @param[in] filePath wavファイルのパス
     * @retval  true    読み込み成功
     * @retval  false   読み込み失敗
     */
    bool LoadWavFile(const std::string& filePath);

    /**
     * @brief PCMデータの解放
     */
    void ReleasePcmData();

    /**
     * @brief -1～1の範囲の1サンプル取得
     * @retval    csmFloat32    正規化されたサンプル
     */
    float GetPcmSample();

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
         * @return unsigned char 読み取った8ビット値
         */
        unsigned char Get8()
        {
            const unsigned char ret = _fileByte[_readOffset];
            _readOffset++;
            return ret;
        }

        /**
         * @brief 16ビット読み込み（リトルエンディアン）
         * @return unsigned short 読み取った16ビット値
         */
        unsigned short Get16LittleEndian()
        {
            const unsigned short ret = (_fileByte[_readOffset + 1] << 8) | _fileByte[_readOffset];
            _readOffset += 2;
            return ret;
        }

        /**
         * @brief 24ビット読み込み（リトルエンディアン）
         * @return unsigned int 読み取った24ビット値（下位24ビットに設定）
         */
        unsigned int Get24LittleEndian()
        {
            const unsigned int ret =
                (_fileByte[_readOffset + 2] << 16) | (_fileByte[_readOffset + 1] << 8)
                | _fileByte[_readOffset];
            _readOffset += 3;
            return ret;
        }

        /**
         * @brief 32ビット読み込み（リトルエンディアン）
         * @return unsigned int 読み取った32ビット値
         */
        unsigned int Get32LittleEndian()
        {
            const unsigned int ret =
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
        bool GetCheckSignature(const std::string& reference)
        {
            char getSignature[4] = { 0, 0, 0, 0 };
            const char* referenceString = reference.c_str();
            if (reference.length() != 4)
            {
                return false;
            }
            for (unsigned int signatureOffset = 0; signatureOffset < 4; signatureOffset++)
            {
                getSignature[signatureOffset] = static_cast<char>(Get8());
            }
            return (getSignature[0] == referenceString[0]) && (getSignature[1] == referenceString[1])
                && (getSignature[2] == referenceString[2]) && (getSignature[3] == referenceString[3]);
        }

        unsigned char* _fileByte; ///< ロードしたファイルのバイト列
        int _fileSize; ///< ファイルサイズ
        unsigned int _readOffset; ///< ファイル参照位置
    } _byteReader;

    unsigned char* _rawData; ///< 正規化される前のバイト列
    unsigned long _rawDataSize; ///< 正規化される前のバイト列の大きさ
    float** _pcmData; ///< -1から1の範囲で表現された音声データ配列
    unsigned int _sampleOffset; ///< サンプル参照位置
    float _lastRms; ///< 最後に計測したRMS値
    float _startUserTime; ///< デルタ時間の積算値[秒]
 };
