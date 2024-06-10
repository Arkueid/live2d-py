import sys

from PySide6.QtCore import Qt, Signal, QPropertyAnimation
from PySide6.QtGui import QColor
from PySide6.QtWidgets import QWidget, QHBoxLayout, QApplication, QGraphicsDropShadowEffect
from qfluentwidgets import PushButton, FlyoutView, isDarkTheme

from config import Configuration


class FlyoutText(QWidget):
    pressed = Signal()
    config: Configuration

    def __init__(self, config: Configuration, parent):
        super().__init__(parent)
        self.view = FlyoutView("", "d")
        self.config = config

        self.hBoxLayout = QHBoxLayout(self)

        self.hBoxLayout.setContentsMargins(15, 8, 15, 20)
        self.hBoxLayout.addWidget(self.view)
        self.setShadowEffect()

        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setWindowFlags(Qt.WindowType.Tool | Qt.WindowType.FramelessWindowHint |
                            Qt.WindowType.NoDropShadowWindowHint)

        self.pressed.connect(self.close)

    def setShadowEffect(self, blurRadius=35, offset=(0, 8)):
        color = QColor(0, 0, 0, 80 if isDarkTheme() else 30)
        self.shadowEffect = QGraphicsDropShadowEffect(self.view)
        self.shadowEffect.setBlurRadius(blurRadius)
        self.shadowEffect.setOffset(*offset)
        self.shadowEffect.setColor(color)
        self.view.setGraphicsEffect(None)
        self.view.setGraphicsEffect(self.shadowEffect)

    def closeEvent(self, e):
        if self.fadeOutAni:
            self.fadeOutAni.deleteLater()
            self.fadeOutAni = None

        super().closeEvent(e)

    def mouseReleaseEvent(self, event):
        super().mouseReleaseEvent(event)
        self.pressed.emit()

    def showText(self, content):
        self.hide()
        self.setWindowOpacity(1)
        self.view.content = content
        self.view._adjustText()
        self.show()
        self.move(self.config.lastX.value + self.config.width.value // 2 - self.width() // 2,
            self.config.lastY.value - self.height() - 10)
        self.activateWindow()

    def fadeOut(self):
        self.fadeOutAni = QPropertyAnimation(self, b'windowOpacity', self)
        self.fadeOutAni.finished.connect(self.close)
        self.fadeOutAni.setStartValue(1)
        self.fadeOutAni.setEndValue(0)
        self.fadeOutAni.setDuration(120)
        self.fadeOutAni.start()


class Demo(QWidget):

    def __init__(self):
        super().__init__()
        self.button = PushButton("Click Me", self)
        self.button.clicked.connect(self.showFlyout)
        self.ft = FlyoutText(self)

        self.hBoxLayout = QHBoxLayout(self)
        self.hBoxLayout.addWidget(self.button, 0, Qt.AlignCenter)
        self.resize(600, 500)

    def showFlyout(self):
        self.ft.showText("33333333333333333333333333333333333")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = Demo()
    win.show()
    app.exec()
