from abc import abstractmethod, ABC

from PySide6.QtGui import QAction, QIcon
from PySide6.QtWidgets import QSystemTrayIcon
from qfluentwidgets import CheckableMenu

from config import Configuration


class Systray(QSystemTrayIcon):
    class CallbackSet(ABC):

        @abstractmethod
        def toggleCharacterVisibility(self):
            pass

        @abstractmethod
        def toggleEyeTracking(self):
            pass

        @abstractmethod
        def toggleClickTransparent(self):
            pass

        @abstractmethod
        def lockWindow(self):
            pass

        @abstractmethod
        def stickWindowToTop(self):
            pass

        @abstractmethod
        def openSettings(self):
            pass

        @abstractmethod
        def exitApplication(self):
            pass

    callbackSet: CallbackSet

    def __init__(self):
        super().__init__()
        self.action_ls = [
            QAction("角色显示"),
            QAction("视线追踪"),
            QAction("透明穿透"),
            QAction("锁定窗口"),
            QAction("窗口置顶"),
            QAction("打开设置"),
            QAction("退出")
        ]

        menu = CheckableMenu()
        menu.addActions(self.action_ls[:-1])
        menu.addSeparator()
        menu.addAction(self.action_ls[-1])
        self.setContextMenu(menu)

        self.activated.connect(self.on_activated)

    def on_activated(self, reason):
        if reason == self.ActivationReason.Context:
            self.contextMenu().show()
        elif reason == self.ActivationReason.DoubleClick:
            self.callbackSet.toggleCharacterVisibility()
            self.action_ls[0].setChecked(not self.action_ls[0].isChecked())

    def setup(self, config: Configuration, callbackSet: CallbackSet):
        self.setIcon(QIcon(config.icon_path.value))

        self.callbackSet = callbackSet

        callbacks = [
            self.callbackSet.toggleCharacterVisibility,
            self.callbackSet.toggleEyeTracking,
            self.callbackSet.toggleClickTransparent,
            self.callbackSet.lockWindow,
            self.callbackSet.stickWindowToTop,
            self.callbackSet.openSettings,
            self.callbackSet.exitApplication,
        ]

        values = [
            config.visible.value,
            config.track_enable.value,
            config.click_transparent.value,
            not config.enable.value,
            config.stay_on_top.value,
            None,
            None,
        ]

        for idx, action in enumerate(self.action_ls):
            action: QAction
            if values[idx] is not None:
                action.setCheckable(True)
                action.setChecked(values[idx])
            action.triggered.connect(callbacks[idx])

    def start(self):
        self.show()
