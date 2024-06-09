from PySide6.QtWidgets import (QWidget,
                               QGridLayout, QFileDialog, QVBoxLayout)
from PySide6.QtGui import QColor, QIntValidator
from PySide6.QtCore import Qt
from qfluentwidgets import LineEdit, PrimaryPushButton, Slider, BodyLabel, ToggleToolButton, \
    FluentIcon, CheckBox


class ApiSettingsDesign(QWidget):
    def __init__(self):
        super().__init__()

        # Initialize fields
        self.savePath = LineEdit()

        self.apiKey = LineEdit()

        self.apiSecret = LineEdit()

        self.hostPort = LineEdit()

        self.readTimeOut = LineEdit()
        self.readTimeOut.setValidator(QIntValidator(0, 9999, self))
        self.readTimeOut.setFixedWidth(100)

        self.route = LineEdit()

        self.voiceRoute = LineEdit()

        # Labels
        self.lbl_apiKey = BodyLabel("ApiKey")
        self.lbl_apiSecret = BodyLabel("ApiSecret")
        self.lbl_hostPort = BodyLabel("服务器地址")
        self.lbl_route = BodyLabel("文本处理路径")
        self.lbl_readTimeOut = BodyLabel("最长响应时间")
        self.lbl_savePath = BodyLabel("聊天保存位置")
        self.lbl_voiceRoute = BodyLabel("语音处理路径")

        # Buttons
        self.apply = PrimaryPushButton("保存")

        self.reset = PrimaryPushButton("重置")

        self.chooseDir = PrimaryPushButton("打开")

        # Checkboxes
        self.MlyAI = CheckBox("茉莉云API")

        self.CustomChatServer = CheckBox("自定义聊天服务器")

        self.CustomVoiceChat = CheckBox("自定义语音处理路径")

        # Layout
        grid = QGridLayout()

        grid.addWidget(self.lbl_savePath, 0, 0, 1, 1)
        grid.addWidget(self.savePath, 0, 1, 1, 4)
        grid.addWidget(self.chooseDir, 0, 5, 1, 1)
        grid.addWidget(self.MlyAI, 1, 0, 1, 1)
        grid.addWidget(self.lbl_apiKey, 2, 0, 1, 1)
        grid.addWidget(self.apiKey, 2, 1, 1, 4)
        grid.addWidget(self.lbl_apiSecret, 3, 0, 1, 1)
        grid.addWidget(self.apiSecret, 3, 1, 1, 4)
        grid.addWidget(self.CustomChatServer, 4, 0, 1, 1)
        grid.addWidget(self.lbl_hostPort, 5, 0, 1, 1)
        grid.addWidget(self.hostPort, 5, 1, 1, 4)
        grid.addWidget(self.lbl_route, 6, 0, 1, 1)
        grid.addWidget(self.route, 6, 1, 1, 4)
        grid.addWidget(self.CustomVoiceChat, 7, 0, 1, 1)
        grid.addWidget(self.lbl_voiceRoute, 8, 0, 1, 1)
        grid.addWidget(self.voiceRoute, 8, 1, 1, 1)
        grid.addWidget(self.lbl_readTimeOut, 9, 0, 1, 1)
        grid.addWidget(self.readTimeOut, 9, 1, 1, 4)
        grid.addWidget(self.apply, 10, 4, 2, 1)
        grid.addWidget(self.reset, 10, 5, 2, 1)

        # Set placeholder texts
        self.hostPort.setPlaceholderText("example: http://127.0.0.1:8080")
        self.voiceRoute.setPlaceholderText("example: /voiceRoute")
        self.route.setPlaceholderText("example: /chat")
        self.readTimeOut.setPlaceholderText("eg: 10")

        # Set layout
        self.setLayout(grid)
