import os
from qfluentwidgets import *

from config.configuration import Configuration
from ui.components.design.base_designs import ScrollDesign
from ui.components.setting_cards import *


class AppSettingsDesign(ScrollDesign):

    def __init__(self, config: Configuration):
        super().__init__()
        self.resource_dir = config.resource_dir.value

        self.card_width = SpinSettingCard(config.width, self.icon("width.svg"), "画布宽度")
        self.card_height = SpinSettingCard(config.height, self.icon("height.svg"), "画布高度")
        self.card_fps = SpinSettingCard(config.fps, self.icon("fps.svg"), "FPS")

        self.card_group_drawCenter = ExpandGroupSettingCard(self.icon("draw_center.svg"), "绘制中心")
        self.card_group_drawCenter.setContentsMargins(20, 10, 20, 10)
        self.card_drawX = GroupItemDoubleSpin(config.drawX, "X")
        self.card_drawY = GroupItemDoubleSpin(config.drawY, "Y")
        self.card_group_drawCenter.addGroupWidget(self.card_drawX)
        self.card_group_drawCenter.addGroupWidget(self.card_drawY)

        self.card_motion_interval = SpinSettingCard(config.motion_interval, self.icon("motion_interval.svg"), "动作频率")
        self.card_lip_sync = DoubleSpinSettingCard(config.lip_sync, FluentIcon.SETTING, "口型同步幅度")
        self.card_scale = DoubleSpinSettingCard(config.scale, self.icon("scale.svg"), "缩放比例")
        self.card_volume = RangeSettingCard(config.volume, self.icon("volume.svg"), "音量")
        self.card_volume.setContentsMargins(10, 10, 20, 10)
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

    def icon(self, path):
        return QIcon(os.path.join(self.resource_dir, path))


