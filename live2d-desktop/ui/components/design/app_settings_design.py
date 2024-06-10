from PySide6.QtCore import Qt
from PySide6.QtWidgets import QWidget, QVBoxLayout, QHBoxLayout
from qfluentwidgets import *

from config.configuration import Configuration
from ui.components.design.base_designs import ScrollDesign
from ui.components.setting_cards import *


class AppSettingsDesign(ScrollDesign):

    def __init__(self, config: Configuration):
        super().__init__()

        self.card_width = SpinSettingCard(config.width, FluentIcon.SETTING, "画布宽度")
        self.card_height = SpinSettingCard(config.height, FluentIcon.SETTING, "画布高度")
        self.card_fps = SpinSettingCard(config.fps, FluentIcon.SETTING, "FPS")

        self.card_group_drawCenter = ExpandGroupSettingCard(FluentIcon.SETTING, "绘制中心")
        self.card_drawX = GroupItemDoubleSpin(config.drawX, "X")
        self.card_drawY = GroupItemDoubleSpin(config.drawY, "Y")
        self.card_group_drawCenter.addGroupWidget(self.card_drawX)
        self.card_group_drawCenter.addGroupWidget(self.card_drawY)

        self.card_motion_interval = SpinSettingCard(config.motion_interval, FluentIcon.SETTING, "动作频率")
        self.card_lip_sync = DoubleSpinSettingCard(config.lip_sync, FluentIcon.SETTING, "口型同步幅度")
        self.card_scale = DoubleSpinSettingCard(config.scale, FluentIcon.SETTING, "缩放比例")
        self.card_volume = RangeSettingCard(config.volume, FluentIcon.MUSIC, "音量")
        # self.card_auto_repair = SwitchSettingCard(configItem=config.auto_repair, icon=FluentIcon.SETTING, title="自动修复")

        self.vBoxLayout.addWidget(self.card_width)
        self.vBoxLayout.addWidget(self.card_height)
        self.vBoxLayout.addWidget(self.card_fps)
        self.vBoxLayout.addWidget(self.card_group_drawCenter)
        self.vBoxLayout.addWidget(self.card_motion_interval)
        self.vBoxLayout.addWidget(self.card_lip_sync)
        self.vBoxLayout.addWidget(self.card_scale)
        self.vBoxLayout.addWidget(self.card_volume)
        # self.vBoxLayout.addWidget(self.card_auto_repair)



