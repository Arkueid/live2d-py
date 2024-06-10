import os.path
import sys

from PySide6.QtWidgets import (
    QApplication, QWidget, QTreeWidgetItem, QFormLayout, QSplitter, QVBoxLayout, QFileDialog
)
from PySide6.QtCore import Qt
from ui.components.dialogs import InputDialog
from utils.model3json import MotionGroups, Model3Json, Motion, MotionGroup

from qfluentwidgets import TreeWidget, TextEdit, BodyLabel, RoundMenu, Dialog, \
    FluentIcon, Action, SplitPushButton


class MotionEditorDesign(QWidget):
    data: MotionGroups
    model_dir: str
    playMotionFunc: callable

    def __init__(self, model3Json: Model3Json):
        super().__init__()
        self.data = model3Json.motion_groups()
        self.model_dir = model3Json.src_dir()

        self.setMinimumHeight(250)

        splitter = QSplitter(self)
        splitter.setOrientation(Qt.Orientation.Horizontal)

        self.tree = TreeWidget()
        self.tree.setHeaderHidden(True)
        self.populate_tree()
        self.tree.itemClicked.connect(self.on_item_clicked)
        self.tree.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.tree.customContextMenuRequested.connect(self.open_menu)
        splitter.addWidget(self.tree)

        self.detail_widget = QWidget()
        self.detail_layout = QFormLayout()
        self.detail_widget.setLayout(self.detail_layout)
        splitter.addWidget(self.detail_widget)

        vbox = QVBoxLayout()
        self.setLayout(vbox)
        vbox.addWidget(splitter)

        self.playMotionFunc = lambda group, idx: print(group, idx)

        splitter.setSizes([300, 600])

    def populate_tree(self):
        self.tree.clear()
        for category, motions in self.data:
            category_item = QTreeWidgetItem(self.tree, [category])
            for idx, motion in enumerate(motions):
                motion_item = QTreeWidgetItem(category_item, ["%s_%d" % (category, idx)])
                motion_item.setData(0, Qt.ItemDataRole.UserRole, motion)

    def on_item_clicked(self, item):
        motion = item.data(0, Qt.ItemDataRole.UserRole)
        if motion:
            self.display_motion_details(motion)
        else:
            self.clear_motion_details()

    def display_motion_details(self, motion):
        self.clear_motion_details()

        f_lbl = BodyLabel("文件")
        f_btn = SplitPushButton(FluentIcon.FOLDER, motion.file())
        f_btn.clicked.connect(lambda: self.set_file(f_btn, motion))
        menu1 = RoundMenu(parent=f_btn)
        menu1.addAction(Action(FluentIcon.DELETE, '清除', triggered=lambda: self.clear_file(f_btn, motion)))
        f_btn.setFlyout(menu1)
        self.detail_layout.addRow(f_lbl, f_btn)

        s_lbl = BodyLabel("音频")
        s_btn = SplitPushButton(FluentIcon.FOLDER, motion.sound())
        s_btn.clicked.connect(lambda: self.set_sound(s_btn, motion))
        menu2 = RoundMenu(parent=s_btn)
        menu2.addAction(Action(FluentIcon.DELETE, "清除", triggered=lambda: self.clear_sound(s_btn, motion)))
        s_btn.setFlyout(menu2)
        self.detail_layout.addRow(s_lbl, s_btn)

        t_lbl = BodyLabel("文本")
        t_ld = TextEdit()
        t_ld.setText(motion.text())
        t_ld.textChanged.connect(lambda: motion.set_text(t_ld.toPlainText()))
        self.detail_layout.addRow(t_lbl, t_ld)

    def set_file(self, sender, motion: Motion):
        s1, _ = QFileDialog.getOpenFileName(self, "动作文件", self.model_dir,
                                            "Motion3(*.motion3.json)")
        if s1:
            s1 = os.path.relpath(s1, self.model_dir)
            sender.setText(s1)
            motion.set_file(s1)

    def clear_file(self, sender, motion: Motion):
        sender.setText("")
        motion.set_file("")

    def set_sound(self, sender, motion: Motion):
        s1, _ = QFileDialog.getOpenFileName(self, "音频文件", self.model_dir, "Wav(*.wav)")
        if s1:
            s1 = os.path.relpath(s1, self.model_dir)
            sender.setText(s1)
            motion.set_sound(s1)

    def clear_sound(self, sender, motion: Motion):
        sender.setText("")
        motion.set_sound("")

    def clear_motion_details(self):
        for i in reversed(range(self.detail_layout.count())):
            widget = self.detail_layout.itemAt(i).widget()
            if widget:
                widget.deleteLater()

    def open_menu(self, position):
        item = self.tree.itemAt(position)
        menu = RoundMenu()
        menu.setShadowEffect(offset=(0,0))

        if not item:  # container
            add_action = Action("添加组", self)
            add_action.triggered.connect(lambda: self.add_group())
            menu.addAction(add_action)
        else:
            if item.parent() is None:  # Group
                add_action = Action("添加动作", self)
                add_action.triggered.connect(lambda: self.add_motion(item))
                menu.addAction(add_action)

                rename_action = Action("重命名组", self)
                rename_action.triggered.connect(lambda: self.rename_group(item))
                menu.addAction(rename_action)

                delete_action = Action("删除组", self)
                delete_action.triggered.connect(lambda: self.delete_group(item))
                menu.addAction(delete_action)
            else:  # Motion
                add_action = Action("添加动作", self)
                add_action.triggered.connect(lambda: self.add_motion(item.parent()))
                menu.addAction(add_action)

                play_action = Action("播放动作", self)
                play_action.triggered.connect(lambda: self.playMotionFunc(item.parent().text(0), item.parent().indexOfChild(item)))
                menu.addAction(play_action)

                delete_action = Action("删除动作", self)
                delete_action.triggered.connect(lambda: self.delete_motion(item))
                menu.addAction(delete_action)

        menu.exec(self.tree.viewport().mapToGlobal(position))

    def add_group(self):
        group_name, ok = InputDialog.getText(self, "添加组")
        if ok and group_name:
            self.data.add(group_name, MotionGroup())
            self.populate_tree()

    def add_motion(self, group_item):
        new_motion = Motion({"File": "", "Sound": "", "Text": ""})
        group_name = group_item.text(0)
        self.data.group(group_name).add(new_motion)
        self.populate_tree()

    def rename_group(self, group_item):
        new_name, ok = InputDialog.getText(self, "重命名组", group_item.text(0))
        if ok and new_name:
            old_name = group_item.text(0)
            self.data.meta()[new_name] = self.data.meta().pop(old_name)
            self.populate_tree()

    def delete_group(self, group_item):
        dialog = Dialog('删除组',
                        f"Are you sure you want to delete the group '{group_item.text(0)}'?",
                        self)
        if dialog.exec():
            self.data.remove(group_item.text(0))
            self.populate_tree()

        dialog.deleteLater()

    def delete_motion(self, motion_item):
        dialog = Dialog('删除动作',
                        f"Are you sure you want to delete the motion '{motion_item.text(0)}'?", self)
        if dialog.exec():
            parent = motion_item.parent()
            motion = motion_item.data(0, Qt.ItemDataRole.UserRole)
            self.data.group(parent.text(0)).remove(motion)
            self.populate_tree()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    m = Model3Json()
    m.load("../../Resources/Haru/Haru.model3.json")
    window = MotionEditorDesign(m.motion_groups())
    window.show()
    sys.exit(app.exec())
