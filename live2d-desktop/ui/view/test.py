import sys

from PySide6.QtWidgets import (
    QApplication, QWidget, QTreeWidget, QTreeWidgetItem, QFormLayout,
    QLineEdit, QLabel, QSplitter, QMainWindow, QMenu, QInputDialog, QMessageBox
)
from PySide6.QtCore import Qt
from PySide6.QtGui import QAction
from utils.model3json import MotionGroups, Model3Json


class MotionEditor(QMainWindow):
    data: MotionGroups

    def __init__(self, data: MotionGroups):
        super().__init__()
        self.data = data

        self.setWindowTitle("Tree Editor Example")
        self.resize(800, 600)

        splitter = QSplitter(Qt.Orientation.Horizontal)
        splitter.setSizes([200, 600])

        self.tree = QTreeWidget()
        self.tree.setHeaderLabels(["Index"])
        self.tree.setFixedWidth(200)
        self.populate_tree()
        self.tree.itemClicked.connect(self.on_item_clicked)
        self.tree.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.tree.customContextMenuRequested.connect(self.open_menu)
        splitter.addWidget(self.tree)

        self.detail_widget = QWidget()
        self.detail_layout = QFormLayout()
        self.detail_widget.setLayout(self.detail_layout)
        splitter.addWidget(self.detail_widget)

        self.setCentralWidget(splitter)

    def populate_tree(self):
        self.tree.clear()
        for category, motions in self.data:
            category_item = QTreeWidgetItem(self.tree, [category])
            for motion in motions:
                motion_item = QTreeWidgetItem(category_item, [motion.file()])
                motion_item.setData(0, Qt.ItemDataRole.UserRole, motion)
        self.tree.expandAll()

    def on_item_clicked(self, item, column):
        motion = item.data(0, Qt.ItemDataRole.UserRole)
        if motion:
            self.display_motion_details(motion)
        else:
            self.clear_motion_details()

    def display_motion_details(self, motion):
        self.clear_motion_details()

        fields = ["File", "Sound", "Text"]
        for field in fields:
            label = QLabel(field)
            line_edit = QLineEdit(str(motion.meta().get(field, "")))
            self.detail_layout.addRow(label, line_edit)

    def clear_motion_details(self):
        for i in reversed(range(self.detail_layout.count())):
            widget = self.detail_layout.itemAt(i).widget()
            if widget:
                widget.deleteLater()

    def open_menu(self, position):
        item = self.tree.itemAt(position)
        if not item:
            return

        menu = QMenu()
        if item.parent() is None:  # Group
            add_action = QAction("Add Motion", self)
            add_action.triggered.connect(lambda: self.add_motion(item))
            menu.addAction(add_action)

            rename_action = QAction("Rename Group", self)
            rename_action.triggered.connect(lambda: self.rename_group(item))
            menu.addAction(rename_action)

            delete_action = QAction("Delete Group", self)
            delete_action.triggered.connect(lambda: self.delete_group(item))
            menu.addAction(delete_action)
        else:  # Motion
            add_action = QAction("Add Motion", self)
            add_action.triggered.connect(lambda: self.add_motion(item.parent()))
            menu.addAction(add_action)

            delete_action = QAction("Delete Motion", self)
            delete_action.triggered.connect(lambda: self.delete_motion(item))
            menu.addAction(delete_action)

        menu.exec(self.tree.viewport().mapToGlobal(position))

    def add_motion(self, group_item):
        motion, ok = QInputDialog.getText(self, "Add Motion", "Enter motion file name:")
        if ok and motion:
            new_motion = {"File": motion, "FadeInTime": 0.5, "FadeOutTime": 0.5}
            group_name = group_item.text(0)
            self.populate_tree()

    def rename_group(self, group_item):
        new_name, ok = QInputDialog.getText(self, "Rename Group", "Enter new group name:", text=group_item.text(0))
        if ok and new_name:
            old_name = group_item.text(0)
            # data[new_name] = data.pop(old_name)
            self.populate_tree()

    def delete_group(self, group_item):
        reply = QMessageBox.question(self, 'Delete Group',
                                     f"Are you sure you want to delete the group '{group_item.text(0)}'?",
                                     QMessageBox.Yes | QMessageBox.No, QMessageBox.No)
        if reply == QMessageBox.Yes:
            # del data[group_item.text(0)]
            self.populate_tree()

    def delete_motion(self, motion_item):
        reply = QMessageBox.question(self, 'Delete Motion',
                                     f"Are you sure you want to delete the motion '{motion_item.text(0)}'?",
                                     QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No, QMessageBox.StandardButton.No)
        if reply == QMessageBox.StandardButton.Yes:
            group_item = motion_item.parent()
            group_name = group_item.text(0)
            motion_file = motion_item.text(0)
            # for motion in data[group_name]:
            #     if motion["File"] == motion_file:
            #         data[group_name].remove(motion)
            #         break
            self.populate_tree()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    m = Model3Json()
    m.load("../../Resources/Haru/Haru.model3.json")
    window = MotionEditor(m.motion_groups())
    window.show()
    sys.exit(app.exec())
