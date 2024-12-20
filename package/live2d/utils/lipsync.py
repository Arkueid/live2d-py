import wave
import numpy as np
import time


class WavHandler:
    def __init__(self):
        # 每个通道的采样帧数
        self.numFrames: int = 0
        # 采样率，帧/秒
        self.sampleRate: int = 0
        self.sampleWidth: int = 0
        # 通道数
        self.numChannels: int = 0
        # 数据
        self.pcmData: np.ndarray = None
        # 已经读取的帧数
        self.lastOffset: int = 0
        # 当前rms值
        self.currentRms: float = 0
        # 开始读取的时间
        self.startTime: float = -1

    def Start(self, filePath: str) -> None:
        self.ReleasePcmData()
        try:
            with wave.open(filePath, "r") as wav:
                self.numFrames = wav.getnframes()
                self.sampleRate = wav.getframerate()
                self.sampleWidth = wav.getsampwidth()
                self.numChannels = wav.getnchannels()
                # 双声道 / 单声道
                self.pcmData = np.frombuffer(wav.readframes(self.numFrames), dtype=np.int16)
                # 标准化
                self.pcmData = self.pcmData / np.max(np.abs(self.pcmData))
                # 拆分通道
                self.pcmData = self.pcmData.reshape(-1, self.numChannels).T

                self.startTime = time.time()
                self.lastOffset = 0

        except Exception as e:
            self.ReleasePcmData()

    def ReleasePcmData(self):
        if self.pcmData is not None:
            del self.pcmData
            self.pcmData = None

    def GetRms(self) -> float:
        """
        获取当前音频响度
        """
        return self.currentRms

    def Update(self) -> bool:
        """
        更新位置
        """
        # 数据未加载或者数据已经读取完毕
        if self.pcmData is None or self.lastOffset >= self.numFrames:
            return False

        currentTime = time.time() - self.startTime
        currentOffset = int(currentTime * self.sampleRate)

        # 时间太短
        if currentOffset == self.lastOffset:
            return True

        currentOffset = min(currentOffset, self.numFrames)

        dataFragment = self.pcmData[:, self.lastOffset:currentOffset].astype(np.float32)

        self.currentRms = np.sqrt(np.mean(np.square(dataFragment)))

        self.lastOffset = currentOffset
        return True