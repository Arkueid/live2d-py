from typing import Union
from PySide6.QtWidgets import QWidget, QHBoxLayout
from PySide6.QtGui import QIcon
from qfluentwidgets import *


class StyledSettingCard(SettingCard):

    def __init__(self, icon: Union[str, QIcon, FluentIconBase], title):
        super().__init__(icon, title)
        self.setContentsMargins(10, 10, 10, 10)


class SpinSettingCard(StyledSettingCard):

    def __init__(self, configItem: RangeConfigItem, icon: Union[str, QIcon, FluentIconBase], title):
        super().__init__(icon, title)
        self.configItem = configItem
        sb = SpinBox()
        sb.setMinimumWidth(150)
        sb.setSingleStep(10)
        sb.setRange(*configItem.range)
        sb.setValue(configItem.value)
        self.hBoxLayout.addWidget(sb)

        sb.valueChanged.connect(self.setValue)

    def setValue(self, value):
        self.configItem.value = value


class DoubleSpinSettingCard(StyledSettingCard):
    configItem: RangeConfigItem

    def __init__(self, configItem: RangeConfigItem, icon: Union[str, QIcon, FluentIconBase], title):
        super().__init__(icon, title)
        self.configItem = configItem
        dsb = DoubleSpinBox()
        dsb.setSingleStep(0.01)
        dsb.setRange(*configItem.range)
        dsb.setValue(configItem.value)
        self.hBoxLayout.addWidget(dsb)
        dsb.setMinimumWidth(150)

        dsb.valueChanged.connect(self.setValue)

    def setValue(self, value):
        self.configItem.value = value


class GroupItemDoubleSpin(QWidget):
    configItem: RangeConfigItem

    def __init__(self, configItem: RangeConfigItem, title):
        super().__init__()
        self.configItem = configItem

        hBoxLayout = QHBoxLayout()
        # hBoxLayout.setContentsMargins(48, 12, 48, 12)

        dsb = DoubleSpinBox()
        dsb.setMinimumWidth(150)
        dsb.setSingleStep(0.01)
        dsb.setRange(*configItem.range)
        dsb.setValue(configItem.value)

        hBoxLayout.addWidget(BodyLabel(title))
        hBoxLayout.addStretch(1)
        hBoxLayout.addWidget(dsb)

        self.setLayout(hBoxLayout)

        dsb.valueChanged.connect(self.setValue)

    def setValue(self, value):
        self.configItem.value = value


class ChangeModelSettingCard(StyledSettingCard):
    configItem: ConfigItem
    modelList: list[str]

    def __init__(self, configItem: ConfigItem, modelList: list[str], icon: Union[str, QIcon, FluentIconBase], title):
        super().__init__(icon, title)
        self.configItem = configItem
        self.modelList = modelList

        self.cb = ComboBox()
        self.cb.addItems(modelList)
        self.cb.setCurrentText(configItem.value)
        self.btn_change = PrimaryPushButton("切换")
        self.btn_change.released.connect(self.changeModel)
        self.hBoxLayout.addWidget(self.cb)
        self.hBoxLayout.addSpacing(10)
        self.hBoxLayout.addWidget(self.btn_change)

    def changeModel(self):
        self.configItem.value = self.cb.currentText()
