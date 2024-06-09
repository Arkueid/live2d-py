from PySide6.QtWidgets import QMessageBox
from qfluentwidgets import MessageBoxBase, SubtitleLabel, LineEdit


class InputDialog(MessageBoxBase):
    """ Custom message box """
    res: QMessageBox.StandardButton

    def __init__(self, parent=None):
        super().__init__(parent)
        self.titleLabel = SubtitleLabel()
        self.lineEdit = LineEdit()

        self.lineEdit.setClearButtonEnabled(True)

        self.viewLayout.addWidget(self.titleLabel)
        self.viewLayout.addWidget(self.lineEdit)

        self.widget.setMinimumWidth(350)

    @staticmethod
    def getText(parent, title, default=""):
        w = InputDialog(parent)
        w.titleLabel.setText(title)
        w.lineEdit.setText(default)

        ok = w.exec()
        res = (w.lineEdit.text(), ok)
        w.deleteLater()

        return res
