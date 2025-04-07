import ctypes
import ctypes.wintypes
from PyQt6.QtWidgets import (QDialog, QVBoxLayout, QHBoxLayout, QLabel, QPushButton)
from PyQt6.QtCore import Qt, QSize, pyqtSignal
from PyQt6.QtGui import QPainter, QColor, QShowEvent
import qtawesome as qta

class DialogBox(QDialog):
    screenshot_requested = pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setAttribute(Qt.WidgetAttribute.WA_NoSystemBackground, True)
        self.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.hwnd = None

        layout = QVBoxLayout(self)
        layout.setContentsMargins(15, 10, 15, 10)
        layout.setSpacing(10)

        self.label = QLabel("Do you want to take another screenshot?")
        self.label.setStyleSheet("color: white; font-size: 10pt;")
        self.label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        button_layout = QHBoxLayout()
        button_layout.setSpacing(30)

        self.yes_button = QPushButton()
        yes_icon = qta.icon('fa5s.check', color='lime')
        self.yes_button.setIcon(yes_icon)
        self.yes_button.setIconSize(QSize(20, 20))
        self.yes_button.setFixedSize(QSize(40, 30))
        self.yes_button.setFlat(True)
        self.yes_button.setStyleSheet("""
            QPushButton { background-color: rgba(255, 255, 255, 20); border: none; border-radius: 5px; }
            QPushButton:hover { background-color: rgba(255, 255, 255, 40); }
            QPushButton:pressed { background-color: rgba(255, 255, 255, 55); }
        """)
        self.yes_button.setCursor(Qt.CursorShape.PointingHandCursor)

        self.no_button = QPushButton()
        no_icon = qta.icon('fa5s.times', color='#FF474C')
        self.no_button.setIcon(no_icon)
        self.no_button.setIconSize(QSize(20, 20))
        self.no_button.setFixedSize(QSize(40, 30))
        self.no_button.setFlat(True)
        self.no_button.setStyleSheet("""
            QPushButton { background-color: rgba(255, 255, 255, 20); border: none; border-radius: 5px; }
            QPushButton:hover { background-color: rgba(255, 255, 255, 40); }
            QPushButton:pressed { background-color: rgba(255, 255, 255, 55); }
        """)
        self.no_button.setCursor(Qt.CursorShape.PointingHandCursor)

        button_layout.addStretch()
        button_layout.addWidget(self.yes_button)
        button_layout.addWidget(self.no_button)
        button_layout.addStretch()

        layout.addWidget(self.label)
        layout.addLayout(button_layout)

        self.setLayout(layout)
        self.adjustSize()

        self.yes_button.clicked.connect(self._handle_yes)
        self.no_button.clicked.connect(self._handle_no)

    def _handle_yes(self):
        self.screenshot_requested.emit()
        self.accept()

    def _handle_no(self):
        self.reject()

    def showEvent(self, event: QShowEvent):
        super().showEvent(event)
        self.hwnd = int(self.winId())
        if self.hwnd:
            user32 = ctypes.windll.user32
            WDA_EXCLUDEFROMCAPTURE = 0x00000011
            WDA_MONITOR = 0x00000001
            success = user32.SetWindowDisplayAffinity(self.hwnd, WDA_EXCLUDEFROMCAPTURE)
            if success == 0:
                user32.SetWindowDisplayAffinity(self.hwnd, WDA_MONITOR)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        painter.setBrush(QColor(0, 0, 0, 160))
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawRoundedRect(self.rect(), 10, 10)