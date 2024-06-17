from PySide2.QtWidgets import QWidget, QVBoxLayout

from qfluentwidgets import SingleDirectionScrollArea


class ScrollDesign(QWidget):

    def __init__(self):
        super().__init__()
        vbox = QVBoxLayout()
        scrollArea = SingleDirectionScrollArea(self)
        scrollArea.setWidgetResizable(True)
        scrollArea.setStyleSheet("QScrollArea{background: transparent; border: none}")

        view = QWidget()
        view.setStyleSheet("QWidget{background: transparent}")

        self.vBoxLayout = QVBoxLayout(view)

        scrollArea.setWidget(view)
        vbox.addWidget(scrollArea)
        self.setLayout(vbox)

        self.vBoxLayout.setContentsMargins(10, 10, 20, 10)


