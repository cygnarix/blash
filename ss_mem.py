import sys, ctypes
from PyQt6.QtWidgets import QWidget, QVBoxLayout, QScrollArea, QHBoxLayout, QPushButton, QApplication, QLabel
from PyQt6.QtCore import Qt, QSize, QRectF, QPointF, pyqtSignal, QTimer, QThread, QObject, pyqtSlot
from PyQt6.QtGui import QPainter, QColor, QPixmap, QMouseEvent
import qtawesome as qta
from response import ResponseDialog
from gemini import generate_code_from_screenshots as generate_gemini
import gpt

# CHANGE MODEL HERE
model_selection = 'gpt' # Options: 'gemini' or 'gpt' Gemini = Gemini, GPT = ChatGPT


THUMBNAIL_WIDTH = 180
ZOOM_FACTOR = 0.6

class AIWorker(QObject):
    finished = pyqtSignal(str)
    def __init__(self, pixmaps, model_choice):
        super().__init__()
        self.pixmaps = pixmaps
        self.model_choice = model_choice

    @pyqtSlot()
    def run(self):
        try:
            valid = [p for p in self.pixmaps if p and not p.isNull()]
            if not valid:
                 result = "Error: No valid screenshots to send."
            else:
                 if self.model_choice == 'gemini':
                     result = generate_gemini(valid)
                 elif self.model_choice == 'gpt':
                     result = gpt.generate_code_from_screenshots_gpt(valid)
                 else:
                     result = f"Error: Unknown model selection '{self.model_choice}'"
        except Exception as e:
            print(f"Error during {self.model_choice} processing thread: {e}")
            result = f"Error: An exception occurred during {self.model_choice} processing: {e}"
        self.finished.emit(result)

class ThumbnailLabel(QWidget):
    def __init__(self, pixmap, thumbnail_width=THUMBNAIL_WIDTH, parent=None):
        super().__init__(parent)
        self.full_pixmap = pixmap
        self.thumb = self.full_pixmap.scaledToWidth(thumbnail_width, Qt.TransformationMode.SmoothTransformation) if not self.full_pixmap.isNull() else QPixmap()
        self.zoomed = False
        self.mouse_pos = QPointF()
        self.setAttribute(Qt.WidgetAttribute.WA_Hover)
        self.setMouseTracking(True)
        h = int(thumbnail_width * (self.thumb.height() / self.thumb.width())) if self.thumb.width() else thumbnail_width
        self.setFixedSize(thumbnail_width, h)

    def enterEvent(self, event: QMouseEvent):
        self.zoomed = True
        self.mouse_pos = event.position()
        self.update()

    def leaveEvent(self, event):
        self.zoomed = False
        self.update()

    def mouseMoveEvent(self, event: QMouseEvent):
        if self.zoomed:
            self.mouse_pos = event.position()
            self.update()

    def paintEvent(self, event):
        p = QPainter(self)
        p.setRenderHint(QPainter.RenderHint.Antialiasing)
        p.setClipRect(self.rect())

        if not self.zoomed or self.full_pixmap.isNull():
            p.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
            pix = self.thumb.scaled(self.size(), Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation)
            x = (self.width() - pix.width()) / 2
            y = (self.height() - pix.height()) / 2
            p.drawPixmap(int(x), int(y), pix)
        else:
            w = self.width() / ZOOM_FACTOR
            h = self.height() / ZOOM_FACTOR
            cx = self.mouse_pos.x() / self.width() * self.full_pixmap.width()
            cy = self.mouse_pos.y() / self.height() * self.full_pixmap.height()
            x = max(0, min(cx - w/2, self.full_pixmap.width() - w))
            y = max(0, min(cy - h/2, self.full_pixmap.height() - h))
            source_rect = QRectF(x, y, w, h)
            target_rect = QRectF(0, 0, self.width(), self.height())
            p.drawPixmap(target_rect, self.full_pixmap, source_rect)

class ScreenshotViewer(QWidget):
    def __init__(self, pixmaps, parent=None, model_choice='gemini'):
        super().__init__(parent)
        self.toolbar = parent
        self.pixmaps = [p for p in pixmaps if p and not p.isNull()]
        self.model_selection = model_choice

        if not self.pixmaps:
            QTimer.singleShot(0, self.close)
            return

        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setAttribute(Qt.WidgetAttribute.WA_NoSystemBackground, True)
        self.setStyleSheet("border-radius:6px;")

        scroll = QScrollArea(self)
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll.setStyleSheet("""
            QScrollArea {
                background-color: transparent;
                border: none;
            }
            QScrollBar:horizontal {
                border: none;
                background: #1A1A1A;
                height: 9px;
                margin: 0 10px;
                border-radius: 4px;
            }
            QScrollBar::handle:horizontal {
                background: rgba(90,90,90,0.85);
                min-width: 25px;
                border-radius: 4px;
                border: none;
            }
            QScrollBar::handle:horizontal:hover {
                background: rgba(110,110,110,0.9);
            }
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                border: none;
                background: none;
                width: 0px;
                subcontrol-position: right;
                subcontrol-origin: margin;
            }
            QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
                background: none;
            }
            QScrollBar:vertical {
                border: none;
                background: transparent;
                width: 0px; /* Effectively hide it */
                margin: 0;
            }
            QScrollBar::handle:vertical {
                 background: transparent;
                 min-height: 0px;
                 border: none;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                 border: none;
                 background: none;
                 height: 0px;
            }
            QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                 background: none;
            }
        """)
        scroll.viewport().setStyleSheet("background-color: transparent;")


        container = QWidget()
        container.setStyleSheet("background-color: transparent;")
        lay = QHBoxLayout(container)
        lay.setSpacing(12)
        lay.setContentsMargins(12, 8, 12, 8)
        lay.setAlignment(Qt.AlignmentFlag.AlignLeft)

        maxh = 0
        for pm in self.pixmaps:
            lbl = ThumbnailLabel(pm)
            lay.addWidget(lbl)
            maxh = max(maxh, lbl.height())

        lay.addStretch(1)
        scroll.setWidget(container)

        self.loading = QLabel("Processing...", self)
        self.loading.setStyleSheet("color:white;background:rgba(0,0,0,150);padding:5px;border-radius:3px;")
        self.loading.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.loading.setFixedSize(100, 30)
        self.loading.hide()

        vlay = QVBoxLayout(self)
        vlay.setContentsMargins(0, 0, 0, 0)
        vlay.addWidget(scroll)
        vlay.addWidget(self.loading, alignment=Qt.AlignmentFlag.AlignCenter)

        htotal = maxh + 8 + 8 + 9 + 4
        self.setFixedHeight(htotal)
        est = THUMBNAIL_WIDTH + lay.spacing()
        cnt = len(self.pixmaps)
        w = min(cnt, 4) * est - lay.spacing() + 24
        w = max(w, est + 24)
        self.setFixedWidth(w)

        btns = []
        for icon, color, hover, pressed, slot in (
            ('fa5s.times', '#D0D0D0', 'rgba(230,0,0,0.9)', 'rgba(180,0,0,1)', self.close),
            ('fa5s.check', '#77DD77', 'rgba(0,200,0,0.8)', 'rgba(0,150,0,1)', self._confirm),
            ('fa5s.sync-alt', '#ADD8E6', 'rgba(0,100,200,0.8)', 'rgba(0,70,150,1)', self._restart),
        ):
            btn = QPushButton(self)
            btn.setIcon(qta.icon(icon, color=color))
            btn.setIconSize(QSize(10, 10))
            btn.setFixedSize(QSize(19, 19))
            btn.setStyleSheet(f"""
                QPushButton{{background:transparent;border:none;border-radius:{btn.height() // 2}px}}
                QPushButton:hover{{background:{hover};color:white}}
                QPushButton:pressed{{background:{pressed}}}""")
            btn.setCursor(Qt.CursorShape.PointingHandCursor)
            btn.clicked.connect(slot)
            btns.append(btn)

        self.close_btn, self.confirm_btn, self.restart_btn = btns
        self.confirm_btn.setEnabled(True)

        self.loading.raise_()
        self.close_btn.raise_()
        self.confirm_btn.raise_()
        self.restart_btn.raise_()

        self.thread = None
        self.worker = None
        self.response_dialog = None

    def _confirm(self):
        if self.thread and self.thread.isRunning():
            return

        self.loading.move((self.width() - self.loading.width()) // 2, (self.height() - self.loading.height()) // 2)
        self.loading.show()
        self.confirm_btn.setEnabled(False)
        self.restart_btn.setEnabled(False)
        self.close_btn.setEnabled(False)

        self.thread = QThread(self)
        self.worker = AIWorker(self.pixmaps, self.model_selection)
        self.worker.moveToThread(self.thread)
        self.worker.finished.connect(self._result)
        self.thread.started.connect(self.worker.run)
        self.thread.finished.connect(self.thread.deleteLater)
        self.worker.finished.connect(self.worker.deleteLater)
        self.thread.start()

    @pyqtSlot(str)
    def _result(self, text):
        self.loading.hide()
        self.confirm_btn.setEnabled(True)
        self.restart_btn.setEnabled(True)
        self.close_btn.setEnabled(True)
        self.thread = None
        self.worker = None

        self.close()

        self.response_dialog = ResponseDialog(self.toolbar)
        self.response_dialog.set_response_text(text)
        self.response_dialog.closed.connect(self._on_response_closed)

        if hasattr(self.toolbar, 'response_dialog'):
            self.toolbar.response_dialog = self.response_dialog

        x = self.toolbar.x()
        y = self.toolbar.y() + self.toolbar.height() + 10
        self.response_dialog.move(x, y)
        self.response_dialog.show()

    def _on_response_closed(self):
         if hasattr(self.toolbar, 'response_dialog'):
              self.toolbar.response_dialog = None
         self.response_dialog = None


    def _restart(self):
        if self.thread and self.thread.isRunning():
            return
        self.close()
        if self.toolbar and hasattr(self.toolbar, '_take_screenshot'):
             getattr(self.toolbar, '_take_screenshot', lambda: None)()

    def closeEvent(self, e):
        if self.thread and self.thread.isRunning():
            self.thread.quit()
            self.thread.wait(1000)
        if self.toolbar and hasattr(self.toolbar, 'viewer_window') and self.toolbar.viewer_window == self:
            self.toolbar.viewer_window = None
        super().closeEvent(e)

    def showEvent(self, e):
        super().showEvent(e)
        sp = 3
        bw = self.close_btn.width()
        total = bw * 3 + sp * 2
        x = self.width() - total - 5
        self.restart_btn.move(x, 3)
        self.confirm_btn.move(x + bw + sp, 3)
        self.close_btn.move(x + (bw + sp) * 2, 3)
        self.loading.move((self.width() - self.loading.width()) // 2, (self.height() - self.loading.height()) // 2)

        if sys.platform == 'win32':
            try:
                ctypes.windll.user32.SetWindowDisplayAffinity(int(self.winId()), 0x11)
            except Exception as ex:
                 print(f"Failed to set display affinity for viewer: {ex}")
                 pass


    def paintEvent(self, e):
        p = QPainter(self)
        p.setRenderHint(QPainter.RenderHint.Antialiasing)
        p.setPen(Qt.PenStyle.NoPen)
        p.setBrush(QColor(30, 30, 30, 128))
        p.drawRoundedRect(self.rect(), 6, 6)
        p.setBrush(QColor(55, 55, 55, 160))
        bw = self.close_btn.width()
        sp = 3
        total_button_width = bw * 3 + sp * 2 + 8
        button_bg_rect = QRectF(self.width() - total_button_width, 0, float(total_button_width), float(bw + 6))
        p.drawRect(button_bg_rect)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    class MockToolbar(QWidget):
        def __init__(self):
            super().__init__()
            self.screenshot_pixmaps = []
            self.viewer_window = None
            self.response_dialog = None
            self.selected_model = model_selection

        def _take_screenshot(self):
             print("Mock screenshot requested")
             if self.viewer_window: self.viewer_window.close(); self.viewer_window = None
             QTimer.singleShot(100, self._show)
        def _show(self):
             pm = QPixmap(220, 160); pm.fill(QColor("orange"))
             self.screenshot_pixmaps = [pm]
             QTimer.singleShot(500, self._show_viewer)
        def _show_viewer(self):
             if self.screenshot_pixmaps:

                 self.viewer_window = ScreenshotViewer(self.screenshot_pixmaps, self, model_choice=self.selected_model)
                 self.viewer_window.move(100, 100); self.viewer_window.show()
                 print(f"Mock viewer shown (Model: {self.selected_model})")

    tb = MockToolbar()

    p1 = QPixmap("screenshot1.png"); p2 = QPixmap("screenshot2.png"); p3 = QPixmap(180, 200)
    if p1.isNull(): p1 = QPixmap(250, 150); p1.fill(QColor("lightblue"))
    if p2.isNull(): p2 = QPixmap(200, 180); p2.fill(QColor("lightgreen"))
    p3.fill(QColor("lightcoral"))
    pixmaps = [p1, p2, p3]

    tb.screenshot_pixmaps = pixmaps
    v = ScreenshotViewer(pixmaps, tb, model_choice=tb.selected_model); tb.viewer_window = v

    if v and not v.isHidden():
        v.show(); sys.exit(app.exec())
    else:
        print("Viewer failed to initialize or was hidden immediately.")
        sys.exit(0)