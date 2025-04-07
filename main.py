import sys
import ctypes
import threading
import pythoncom
import pyWinhook as pyHook
import win32con
from PyQt6.QtWidgets import QApplication
import json
from toolbar import StealthToolbar

KEY_MAP = {
    "F2": win32con.VK_F2,
    "J": ord('J'),
    "RETURN": win32con.VK_RETURN,
    "UP": win32con.VK_UP,
    "DOWN": win32con.VK_DOWN,
    "LEFT": win32con.VK_LEFT,
    "RIGHT": win32con.VK_RIGHT,
    "CONTROL": win32con.VK_CONTROL,
    "LCONTROL": win32con.VK_LCONTROL,
    "RCONTROL": win32con.VK_RCONTROL,
    "MENU": win32con.VK_MENU,
    "LMENU": win32con.VK_LMENU,
    "RMENU": win32con.VK_RMENU,
}

VK_MAP = {v: k for k, v in KEY_MAP.items()}
VK_MAP[win32con.VK_LCONTROL] = "CONTROL"
VK_MAP[win32con.VK_RCONTROL] = "CONTROL"
VK_MAP[win32con.VK_LMENU] = "MENU"
VK_MAP[win32con.VK_RMENU] = "MENU"

class HookManager:
    def __init__(self, toolbar_instance, json_path='kb_sht.json'):
        self.toolbar = toolbar_instance
        self.shortcuts = self._load_shortcuts(json_path)
        self._hook_thread = None
        self._hook_manager = None
        self._f2_pressed = False
        self._ctrl_pressed = False
        self._alt_pressed = False
        self._up_pressed = False
        self._down_pressed = False
        self._left_pressed = False
        self._right_pressed = False
        self._pressed_keys = set()

    def _load_shortcuts(self, json_path):
        try:
            with open(json_path, 'r') as f:
                config = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError):
            config = {}
        processed_shortcuts = {'screenshot': {}, 'f2_mode': {'actions': {}}}
        ss_config = config.get('screenshot', {})
        ss_keys_str = ss_config.get('keys', [])
        processed_shortcuts['screenshot']['keys'] = {
            KEY_MAP.get(k.upper()) for k in ss_keys_str if KEY_MAP.get(k.upper())
        }
        processed_shortcuts['screenshot']['trigger_on'] = ss_config.get('trigger_on')
        f2_config = config.get('f2_mode', {})
        f2_modifier_str = f2_config.get('modifier', 'F2')
        processed_shortcuts['f2_mode']['modifier'] = KEY_MAP.get(f2_modifier_str.upper())
        f2_actions = f2_config.get('actions', {})
        for key_str, action in f2_actions.items():
            vk_code = KEY_MAP.get(key_str.upper())
            if vk_code:
                processed_shortcuts['f2_mode']['actions'][vk_code] = action
            elif len(key_str) == 1 and key_str.isalnum():
                processed_shortcuts['f2_mode']['actions'][ord(key_str.upper())] = action
        return processed_shortcuts

    def _hook_callback(self, event):
        if not hasattr(event, 'KeyID'):
            return True
        key_id = event.KeyID
        is_down = event.Message in (win32con.WM_KEYDOWN, win32con.WM_SYSKEYDOWN)
        ctrl_keys = {win32con.VK_CONTROL, win32con.VK_LCONTROL, win32con.VK_RCONTROL}
        alt_keys = {win32con.VK_MENU, win32con.VK_LMENU, win32con.VK_RMENU}
        arrow_keys = {win32con.VK_UP, win32con.VK_DOWN, win32con.VK_LEFT, win32con.VK_RIGHT}
        f2_key = win32con.VK_F2
        if key_id == f2_key:
            self._f2_pressed = is_down
        elif key_id in ctrl_keys:
            self._ctrl_pressed = is_down
        elif key_id in alt_keys:
            self._alt_pressed = is_down
        if key_id == win32con.VK_UP:
            self._up_pressed = is_down
        elif key_id == win32con.VK_DOWN:
            self._down_pressed = is_down
        elif key_id == win32con.VK_LEFT:
            self._left_pressed = is_down
        elif key_id == win32con.VK_RIGHT:
            self._right_pressed = is_down
        if is_down:
            self._pressed_keys.add(key_id)
            ss_config = self.shortcuts.get('screenshot', {})
            ss_keys = ss_config.get('keys', set())
            if ss_config.get('trigger_on') == 'second_modifier' and ss_keys:
                is_ctrl = key_id in ctrl_keys
                is_alt = key_id in alt_keys
                required_ctrl = any(k in ctrl_keys for k in ss_keys)
                required_alt = any(k in alt_keys for k in ss_keys)
                if required_ctrl and required_alt:
                    response_visible = hasattr(self.toolbar, 'response_dialog') and self.toolbar.response_dialog and self.toolbar.response_dialog.isVisible()
                    viewer_visible = hasattr(self.toolbar, 'viewer_window') and self.toolbar.viewer_window and self.toolbar.viewer_window.isVisible()
                    dialog_visible = hasattr(self.toolbar, 'dialog_box') and self.toolbar.dialog_box and self.toolbar.dialog_box.isVisible()
                    if (is_alt and self._ctrl_pressed) or (is_ctrl and self._alt_pressed):
                        if not (response_visible or viewer_visible or dialog_visible):
                            self.toolbar._take_screenshot()
                        return False
            if self._f2_pressed:
                f2_config = self.shortcuts.get('f2_mode', {})
                f2_actions = f2_config.get('actions', {})
                if key_id == f2_key:
                    return True
                if key_id in arrow_keys:
                    dx = 0
                    dy = 0
                    move_step = self.toolbar.move_step
                    if self._up_pressed:
                        dy -= move_step
                    if self._down_pressed:
                        dy += move_step
                    if self._left_pressed:
                        dx -= move_step
                    if self._right_pressed:
                        dx += move_step
                    if dx or dy:
                        self.toolbar.move_requested_signal.emit(dx, dy)
                    return False
                action = f2_actions.get(key_id)
                if action == "toggle_visibility":
                    self.toolbar.toggle_visibility_signal.emit()
                    return False
                elif action == "quit_app":
                    self.toolbar.quit_requested_signal.emit()
                    return False
                elif action is not None:
                    return False
        else:
            if key_id in self._pressed_keys:
                self._pressed_keys.remove(key_id)
            if key_id == f2_key:
                self._f2_pressed = False
            if key_id in ctrl_keys:
                self._ctrl_pressed = False
            if key_id in alt_keys:
                self._alt_pressed = False
            if key_id == win32con.VK_UP:
                self._up_pressed = False
            elif key_id == win32con.VK_DOWN:
                self._down_pressed = False
            elif key_id == win32con.VK_LEFT:
                self._left_pressed = False
            elif key_id == win32con.VK_RIGHT:
                self._right_pressed = False
        return True

    def _run_hook_manager(self):
        pythoncom.CoInitialize()
        self._hook_manager = pyHook.HookManager()
        self._hook_manager.KeyDown = self._hook_callback
        self._hook_manager.KeyUp = self._hook_callback
        self._hook_manager.HookKeyboard()
        pythoncom.PumpMessages()
        try:
            self._hook_manager.UnhookKeyboard()
        except Exception:
            pass
        pythoncom.CoUninitialize()
        self._hook_manager = None

    def start_listeners(self):
        if self._hook_thread is None or not self._hook_thread.is_alive():
            self._hook_thread = threading.Thread(target=self._run_hook_manager, daemon=True)
            self._hook_thread.start()

    def stop_listeners(self):
        if self._hook_manager and self._hook_thread and self._hook_thread.is_alive():
            thread_id = self._hook_thread.ident
            if thread_id:
                ctypes.windll.user32.PostThreadMessageW(thread_id, win32con.WM_QUIT, 0, 0)
        if self._hook_thread and self._hook_thread.is_alive():
            self._hook_thread.join(timeout=2.0)
        self._hook_thread = None

if __name__ == "__main__":
    is_admin = ctypes.windll.shell32.IsUserAnAdmin() != 0
    if not is_admin:
        pass
    app = QApplication(sys.argv)
    app.setQuitOnLastWindowClosed(False)
    window = StealthToolbar()
    hook_manager = HookManager(window, 'kb_sht.json')
    window.quit_requested_signal.connect(app.quit)
    app.aboutToQuit.connect(hook_manager.stop_listeners)
    hook_manager.start_listeners()
    window.show()
    exit_code = app.exec()
    sys.exit(exit_code)
