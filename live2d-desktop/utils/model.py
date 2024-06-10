import os
import time

from PySide6.QtMultimedia import QMediaPlayer, QAudioOutput
from PySide6.QtCore import QUrl, QCoreApplication, Signal

from app import live2d, define
from config.configuration import Configuration
from ui.view.flyout_text import FlyoutText
from ui.view.scene import Scene


def find_model_dir(path: str) -> list[str]:
    ls: list[str] = list()
    dirs = os.listdir(path)
    for i in dirs:

        if i == '.' or i == '..':
            continue

        dirName = os.path.join(path, i)
        if not os.path.isdir(dirName):
            continue

        modelJson = os.path.join(dirName, i + define.MODEL_JSON_SUFFIX)
        if os.path.exists(modelJson):
            ls.append(i)
    return ls


class Model(Scene.CallBackSet):

    def onInitialize(self):
        self.initialize = True
        self.load_model()

    def onUpdate(self, ww: int, wh: int):
        self.model.SetScale(self.config.scale.value)
        self.model.SetOffset(self.config.drawX.value, self.config.drawY.value)
        live2d.ClearBuffer()
        self.model.Update(ww, wh)

    def onResize(self, ww: int, wh: int):
        self.model.Resize(ww, wh)

    def onTouch(self, rx: int, ry: int):
        self.model.Touch(rx, ry, self.onStartMotionHandler, self.set_motion_finished)

    def onMouseMoved(self, mx: int, my: int):
        self.model.Drag(mx, my)

    def onIntervalReached(self):
        self.start_random_motion(live2d.MotionGroup.IDLE.value, live2d.MotionPriority.IDLE.value)

    def IsFinished(self):
        return self.motionFinished and self.soundFinished

    config: Configuration
    model: live2d.LAppModel | None
    motionFinished: bool
    soundFinished: bool
    initialize: bool
    audioPlayer: QMediaPlayer
    audioOutput: QAudioOutput
    flyoutText: FlyoutText

    def __init__(self):
        self.model = None
        self.motionFinished = True
        self.soundFinished = True
        self.initialize = False
        self.audioPlayer = QMediaPlayer()
        self.audioOutput = QAudioOutput()
        self.audioPlayer.setAudioOutput(self.audioOutput)
        self.audioPlayer.playbackStateChanged.connect(self.set_sound_finished)

    def setup(self, config: Configuration, flyoutText: FlyoutText):
        self.config = config
        self.audioOutput.setVolume(self.config.volume.value / 100)
        self.flyoutText = flyoutText
        self.config.volume.valueChanged.connect(lambda: self.audioOutput.setVolume(self.config.volume.value / 100))

    def load_model(self):
        if not self.initialize:
            return

        if self.model is not None:
            del self.model

        self.model = live2d.LAppModel()
        self.model.LoadAssets(
            os.path.join(self.config.resource_dir.value, self.config.model_name.value),
            self.config.model_name.value + define.MODEL_JSON_SUFFIX)

        self.motionFinished = True

    def start_motion(self, group, no, priority):
        self.model.StartMotion(group, no, priority,
                               self.onStartMotionHandler, self.set_motion_finished)

    def start_random_motion(self, group, priority):
        self.model.StartRandomMotion(group, priority, self.onStartMotionHandler, self.set_motion_finished)

    def set_motion_finished(self):
        self.motionFinished = True
        info = time.strftime("[INFO  %Y-%m-%d %H:%M:%S] motion finished", time.localtime(time.time()))
        print(info)

        self.set_text_finished()

    def set_sound_finished(self, state):
        if state == QMediaPlayer.PlaybackState.StoppedState:
            self.soundFinished = True
            info = time.strftime("[INFO  %Y-%m-%d %H:%M:%S] sound finished", time.localtime(time.time()))
            print(info)
            self.set_text_finished()
   

    def set_text_finished(self):
        if self.motionFinished and self.soundFinished:
            self.flyoutText.fadeOut()

    def onStartMotionHandler(self, group, no):
        self.motionFinished = False
        self.playAudio(group, no)
        self.showText(group, no)

    def showText(self, group, no):
        self.flyoutText.hide()
        text = self.config.model3Json.motion_groups().group(group).motion(no).text()
        if text:
            self.flyoutText.showText(text)

    def playAudio(self, group, no):
        if self.audioPlayer.isPlaying():
            self.audioPlayer.stop()
            QCoreApplication.processEvents()
        file = self.config.model3Json.motion_groups().group(group).motion(no).sound()
        if not file:
            return
        path = os.path.join(self.config.model3Json.src_dir(), file)
        if not os.path.exists(path):
            return
        self.soundFinished = False
        self.audioPlayer.setSource(QUrl.fromLocalFile(path))
        self.audioPlayer.play()
        print(f"[INFO  {time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))}] play audio: {path}")
