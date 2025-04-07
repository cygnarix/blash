import sys
import re
import markdown
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QTextEdit, QPushButton, QHBoxLayout
from PyQt6.QtCore import Qt, QRect, QSize, pyqtSignal
from PyQt6.QtGui import QPainter, QColor, QPalette, QFontDatabase
import ctypes
import qtawesome as qta
import pygments
import pygments.lexers
import pygments.formatters.html

SYNTAX_HIGHLIGHTING = True

class ResponseDialog(QWidget):
    closed = pyqtSignal()
    move_requested = pyqtSignal(int, int)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setAttribute(Qt.WidgetAttribute.WA_NoSystemBackground, True)
        self.mono_font_family = "Consolas, 'Courier New', monospace"
        
        self.text_edit = QTextEdit(self)
        self.text_edit.setReadOnly(True)
        self.text_edit.setStyleSheet(f"""
        QTextEdit {{
            color: white;
            background-color: transparent;
            border: none;
            padding: 10px;
            font-size: 13px;
            font-family: Segoe UI, sans-serif;
        }}
        code {{
            background-color: #404040;
            color: #e0e0e0;
            padding: 1px 4px;
            border-radius: 3px;
            font-family: '{self.mono_font_family}';
            font-size: 12px;
        }}
        pre {{
            background-color: #282c34;
            border: 1px solid #444;
            border-radius: 6px;
            padding: 12px;
            margin: 8px 0;
            font-family: '{self.mono_font_family}';
            font-size: 12px;
            white-space: pre-wrap;
            word-wrap: break-word;
        }}
        QScrollBar:vertical {{
            border: none;
            background: #2A2A2A;
            width: 10px;
        }}
        QScrollBar::handle:vertical {{
            background: #606060;
            min-height: 20px;
            border-radius: 5px;
        }}
        QScrollBar::handle:vertical:hover {{
            background: #707070;
        }}
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
            border: none; background: none; height: 0px;
            subcontrol-position: top; subcontrol-origin: margin;
        }}
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {{
            background: none;
        }}
        QScrollBar:horizontal {{
            border: none; background: #2A2A2A; height: 10px; margin: 0px 0px 0px 0px;
        }}
        QScrollBar::handle:horizontal {{
            background: #606060; min-width: 20px; border-radius: 5px;
        }}
        QScrollBar::handle:horizontal:hover {{ background: #707070; }}
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {{
            border: none; background: none; width: 0px;
            subcontrol-position: left; subcontrol-origin: margin;
        }}
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {{
            background: none;
        }}
        """)
        palette = self.text_edit.palette()
        palette.setColor(QPalette.ColorRole.Base, QColor(0, 0, 0, 0))
        self.text_edit.setPalette(palette)
        self.close_button = QPushButton(self)
        close_icon = qta.icon('fa5s.times', color='white')
        self.close_button.setIcon(close_icon)
        self.close_button.setIconSize(QSize(12, 12))
        self.close_button.setFixedSize(QSize(20, 20))
        self.close_button.setFlat(True)
        self.close_button.setStyleSheet("""
            QPushButton { background-color: transparent; border-radius: 10px; }
            QPushButton:hover { background-color: rgba(255, 0, 0, 0.7); }
            QPushButton:pressed { background-color: rgba(200, 0, 0, 0.9); }
        """)
        self.close_button.setCursor(Qt.CursorShape.PointingHandCursor)
        self.close_button.clicked.connect(self._handle_close)
        top_bar_layout = QHBoxLayout()
        top_bar_layout.addStretch()
        top_bar_layout.addWidget(self.close_button)
        top_bar_layout.setContentsMargins(0, 0, 5, 0)
        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(1, 1, 1, 1)
        main_layout.setSpacing(0)
        main_layout.addLayout(top_bar_layout)
        main_layout.addWidget(self.text_edit)
        self.setGeometry(QRect(100, 100, 500, 400))
        self._offset = None

    def _handle_close(self):
        self.close()

    def set_response_text(self, text):
        exts = ['fenced_code']
        if SYNTAX_HIGHLIGHTING:
            exts.append('codehilite')
        html_text = markdown.markdown(text, extensions=exts, extension_configs={'codehilite': {'guess_lang': False}})
        if SYNTAX_HIGHLIGHTING:
            formatter = pygments.formatters.html.HtmlFormatter(style='dracula')
            style_defs = formatter.get_style_defs('.codehilite')
            style_defs = re.sub(r'background(?:-color)?\s*:\s*[^;]+;', '', style_defs, flags=re.IGNORECASE)
            html_text = f"<style>{style_defs}</style>" + html_text
        self.text_edit.setHtml(html_text)

    def showEvent(self, event):
        super().showEvent(event)
        if sys.platform == 'win32':
            try:
                hwnd = int(self.winId())
                ctypes.windll.user32.SetWindowDisplayAffinity(hwnd, 0x11)
            except Exception as e:
                print(f"Warning: Could not set display affinity (SetWindowDisplayAffinity failed) - {e}")
                pass

    def closeEvent(self, event):
        self.closed.emit()
        super().closeEvent(event)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        painter.setBrush(QColor(20, 20, 20, 210))
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawRoundedRect(self.rect(), 10, 10)

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            if event.position().y() < 25:
                self._offset = event.globalPosition().toPoint() - self.frameGeometry().topLeft()
            else:
                self._offset = None
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):
        if self._offset is not None and event.buttons() == Qt.MouseButton.LeftButton:
            new_pos = event.globalPosition().toPoint() - self._offset
            self.move(new_pos)
            self.move_requested.emit(new_pos.x(), new_pos.y())
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        self._offset = None
        super().mouseReleaseEvent(event)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ResponseDialog()
    window.show()
    sys.exit(app.exec())