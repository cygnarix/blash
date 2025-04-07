import sys
import ctypes
import ctypes.wintypes
from PyQt6.QtWidgets import QApplication, QWidget, QHBoxLayout, QLabel, QPushButton, QFrame, QDialog
from PyQt6.QtCore import Qt, QPoint, QSize, pyqtSignal, pyqtSlot, QTimer
from PyQt6.QtGui import QPainter, QColor, QCloseEvent, QPixmap, QImage
import qtawesome as qta
from PIL import ImageGrab
from ss_dialog import DialogBox
from ss_mem import ScreenshotViewer, model_selection

class StealthToolbar(QWidget):
    move_requested_signal = pyqtSignal(int, int)
    quit_requested_signal = pyqtSignal()
    toggle_visibility_signal = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.move_step = 10
        self.hwnd = None
        self.dialog_box = None
        self.screenshot_pixmaps = []
        self.viewer_window = None
        self.response_dialog = None
        self._dialog_was_visible = False
        self._viewer_was_visible = False
        self._response_dialog_was_visible = False

        self.setWindowFlags(Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint | Qt.WindowType.Tool)
        self.setAttribute(Qt.WidgetAttribute.WA_TranslucentBackground)
        self.setAttribute(Qt.WidgetAttribute.WA_NoSystemBackground, True)

        layout = QHBoxLayout(self)
        layout.setContentsMargins(10, 0, 10, 0)
        layout.setSpacing(2)

        key_style = """
            QLabel {
                color: white;
                background-color: rgba(255, 255, 255, 25);
                border: 1px solid rgba(255, 255, 255, 100);
                border-radius: 4px;
                padding: 1.3px;
                margin: 0;
                font-size: 9pt;
            }"""
        key_size = QSize(28, 28)

        ctrl_label = QLabel("Ctrl")
        ctrl_label.setStyleSheet(key_style)
        ctrl_label.setFixedSize(key_size)
        ctrl_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        alt_label = QLabel("Alt")
        alt_label.setStyleSheet(key_style)
        alt_label.setFixedSize(key_size)
        alt_label.setAlignment(Qt.AlignmentFlag.AlignCenter)

        text_part1 = QLabel(" to take a screenshot")
        text_part1.setStyleSheet("color: white; padding-left: 3px; font-size: 9pt;")

        hyphen_label = QLabel("   –   ")
        hyphen_label.setStyleSheet("color: white; font-weight: bold; font-size: 9pt;")

        screenshot_icon_widget = QLabel()
        screenshot_icon = qta.icon('mdi.aspect-ratio', color='white')
        icon_size = QSize(22, 22)
        screenshot_icon_widget.setPixmap(screenshot_icon.pixmap(icon_size))
        screenshot_icon_widget.setAlignment(Qt.AlignmentFlag.AlignCenter)

        separator = QFrame()
        separator.setFixedHeight(20)
        separator.setFixedWidth(1)
        separator.setStyleSheet("QFrame { border: none; background-color: rgba(255, 255, 255, 240); border-radius: 15px; }")

        settings_button = QPushButton()
        settings_icon = qta.icon('mdi.cog', color='white')
        settings_button.setIcon(settings_icon)
        settings_button.setIconSize(icon_size)
        settings_button.setFlat(True)
        settings_button.setStyleSheet("""
            QPushButton { background-color: transparent; border: none; padding: 3px; margin: 0px; }
            QPushButton:hover { background-color: rgba(255, 255, 255, 30); border-radius: 4px; }
            QPushButton:pressed { background-color: rgba(255, 255, 255, 50); border-radius: 4px; }""")
        settings_button.setCursor(Qt.CursorShape.PointingHandCursor)

        layout.addStretch(1)
        layout.addWidget(ctrl_label)
        layout.addSpacing(5)
        layout.addWidget(alt_label)
        layout.addWidget(text_part1)
        layout.addWidget(hyphen_label)
        layout.addWidget(screenshot_icon_widget)
        layout.addSpacing(16)
        layout.addWidget(separator)
        layout.addSpacing(16)
        layout.addWidget(settings_button)
        layout.addStretch(1)

        self.setLayout(layout)
        self.move_requested_signal.connect(self._handle_move_request)
        self.toggle_visibility_signal.connect(self.toggle_visibility)
        self.quit_requested_signal.connect(self.close)

        self.setFixedSize(340, 50)
        screen_geometry = QApplication.primaryScreen().geometry()
        self.move(int((screen_geometry.width() - self.width()) / 2), 50)

        QTimer.singleShot(10, self.apply_capture_protection)

    def _take_screenshot(self):
        if self.viewer_window and self.viewer_window.isVisible():
            return
        if self.response_dialog and self.response_dialog.isVisible():
            return
        QTimer.singleShot(100, self._take_screenshot_impl)

    def _take_screenshot_impl(self):
        if self.viewer_window and self.viewer_window.isVisible():
            return
        if self.response_dialog and self.response_dialog.isVisible():
             return

        try:
            originally_visible = self.isVisible()
            dialog_originally_visible = self.dialog_box and self.dialog_box.isVisible()
            viewer_originally_visible = self.viewer_window and self.viewer_window.isVisible()
            response_originally_visible = self.response_dialog and self.response_dialog.isVisible()

            if originally_visible: self.hide()
            if dialog_originally_visible: self.dialog_box.hide()
            if viewer_originally_visible: self.viewer_window.hide()
            if response_originally_visible: self.response_dialog.hide()

            QApplication.processEvents()
            QTimer.singleShot(50, lambda: self._grab_and_show_dialog(originally_visible, viewer_originally_visible, response_originally_visible))

        except Exception as e:
            if originally_visible: self.show()
            if dialog_originally_visible and self.dialog_box: self.dialog_box.show()
            if viewer_originally_visible and self.viewer_window: self.viewer_window.show()
            if response_originally_visible and self.response_dialog: self.response_dialog.show()

    def _grab_and_show_dialog(self, toolbar_originally_visible, viewer_originally_visible, response_originally_visible):
        try:
            screenshot = ImageGrab.grab(all_screens=True)
            image = screenshot.convert("RGBA")
            data = image.tobytes("raw", "BGRA")
            qimage = QImage(data, image.width, image.height, QImage.Format.Format_ARGB32)
            pixmap = QPixmap.fromImage(qimage)
            self.screenshot_pixmaps.append(pixmap)
        except Exception:
            pass

        if toolbar_originally_visible:
             self.show()
             self.apply_capture_protection()
        if viewer_originally_visible and self.viewer_window:
            self.viewer_window.show()
            self.apply_capture_protection_to_window(self.viewer_window)
        if response_originally_visible and self.response_dialog:
             self.response_dialog.show()
             self.apply_capture_protection_to_window(self.response_dialog)

        if self.dialog_box:
            try:
                self.dialog_box.screenshot_requested.disconnect(self._take_screenshot)
            except TypeError:
                 pass
            self.dialog_box.close()

        self.dialog_box = DialogBox(self)
        self.dialog_box.screenshot_requested.connect(self._take_screenshot)
        self.dialog_box.finished.connect(self._on_dialog_finished)
        dialog_x = self.x()
        dialog_y = self.y() + self.height() + 10
        self.dialog_box.move(dialog_x, dialog_y)
        self.dialog_box.open()
        QTimer.singleShot(100, lambda: self.apply_capture_protection_to_window(self.dialog_box))

    def _on_dialog_finished(self, result):
        self.dialog_box = None
        if result == QDialog.DialogCode.Rejected:
            self._show_screenshot_viewer()
        else:
            if self.viewer_window and self.viewer_window.isVisible():
                self.viewer_window.close()
            self.viewer_window = None

    def _show_screenshot_viewer(self):
        if not self.screenshot_pixmaps:
            return

        if self.viewer_window and self.viewer_window.isVisible():
            self.viewer_window.close()

        self.viewer_window = ScreenshotViewer(self.screenshot_pixmaps, self, model_choice=model_selection)
        if self.viewer_window:
            viewer_x = self.x()
            viewer_y = self.y() + self.height() + 10
            self.viewer_window.move(viewer_x, viewer_y)
            self.viewer_window.show()
            QTimer.singleShot(100, lambda: self.apply_capture_protection_to_window(self.viewer_window))

        self.screenshot_pixmaps = []

    @pyqtSlot()
    def toggle_visibility(self):
        if self.isVisible():
            self._dialog_was_visible = self.dialog_box and self.dialog_box.isVisible()
            self._viewer_was_visible = self.viewer_window and self.viewer_window.isVisible()
            self._response_dialog_was_visible = self.response_dialog and self.response_dialog.isVisible()
            self.hide()
            if self.dialog_box: self.dialog_box.hide()
            if self.viewer_window: self.viewer_window.hide()
            if self.response_dialog: self.response_dialog.hide()
        else:
            self.show()
            self.apply_capture_protection()
            if self._dialog_was_visible and self.dialog_box:
                self.dialog_box.show()
                self.apply_capture_protection_to_window(self.dialog_box)
            if self._viewer_was_visible and self.viewer_window:
                self.viewer_window.show()
                self.apply_capture_protection_to_window(self.viewer_window)
            if self._response_dialog_was_visible and self.response_dialog:
                self.response_dialog.show()
                self.apply_capture_protection_to_window(self.response_dialog)

    @pyqtSlot(int, int)
    def _handle_move_request(self, dx, dy):
        current_pos = self.pos()
        new_pos = QPoint(current_pos.x() + dx, current_pos.y() + dy)
        self.move(new_pos)

        dialog_offset_y = self.height() + 10

        if self.dialog_box:
            self.dialog_box.move(new_pos.x(), new_pos.y() + dialog_offset_y)
        if self.viewer_window:
             self.viewer_window.move(new_pos.x(), new_pos.y() + dialog_offset_y)
        if self.response_dialog:
             self.response_dialog.move(new_pos.x(), new_pos.y() + dialog_offset_y)

    def trigger_dialog_yes(self):
        if self.dialog_box and self.dialog_box.isVisible():
            self.dialog_box.accept()
            return True
        return False

    def trigger_dialog_no(self):
        if self.dialog_box and self.dialog_box.isVisible():
            self.dialog_box.reject()
            return True
        return False

    def apply_capture_protection_to_window(self, window):
        if not window or not window.isVisible():
            return False

        try:
            hwnd = int(window.winId())
        except (RuntimeError, AttributeError):
            return False

        if not hwnd:
            return False

        user32 = ctypes.windll.user32
        WDA_EXCLUDEFROMCAPTURE = 0x00000011
        WDA_MONITOR = 0x00000001

        try:
            success = user32.SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE)
            if success == 0:
                success = user32.SetWindowDisplayAffinity(hwnd, WDA_MONITOR)
            return success != 0
        except Exception:
            return False

    def apply_capture_protection(self):
        if not self.isVisible():
            return False

        try:
            self.hwnd = int(self.winId())
        except RuntimeError:
            self.hwnd = None

        if not self.hwnd:
            return False

        user32 = ctypes.windll.user32
        WDA_EXCLUDEFROMCAPTURE = 0x00000011
        WDA_MONITOR = 0x00000001

        try:
            success = user32.SetWindowDisplayAffinity(self.hwnd, WDA_EXCLUDEFROMCAPTURE)
            if success == 0:
                success = user32.SetWindowDisplayAffinity(self.hwnd, WDA_MONITOR)

            if self.dialog_box and self.dialog_box.isVisible():
                self.apply_capture_protection_to_window(self.dialog_box)

            if self.viewer_window and self.viewer_window.isVisible():
                self.apply_capture_protection_to_window(self.viewer_window)

            if self.response_dialog and self.response_dialog.isVisible():
                self.apply_capture_protection_to_window(self.response_dialog)

            return True
        except Exception:
            return False

    def showEvent(self, event):
        super().showEvent(event)
        QTimer.singleShot(10, self.apply_capture_protection)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        painter.setBrush(QColor(0, 0, 0, 160))
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawRoundedRect(self.rect(), 10, 10)

    def closeEvent(self, event: QCloseEvent):
        if self.dialog_box: self.dialog_box.close()
        if self.viewer_window: self.viewer_window.close()
        if self.response_dialog: self.response_dialog.close()
        self.quit_requested_signal.emit()
        super().closeEvent(event)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    toolbar = StealthToolbar()
    toolbar.show()
    sys.exit(app.exec())