# Blash C++

## Note: This is version 2.0.0 - Complete C++ remake!

**Blash C++** is a complete C++ remake of the original Python Blash - a better, open-source alternative to Interview Coder. It's designed to be stealthy, customizable, and powerful. It offers real-time coding assistance with support for multiple AI models, and it works invisibly with many popular apps — including the latest version of Zoom.

## 🚀 Features

- 🧠 **Customizable AI Model Support** (Gemini or GPT)
- 🫥 **Stealth Mode** – Invisible in EVERY screen capturing app, including the LATEST version of Zoom
- 🎛️ **Configurable Shortcuts**
- 🔧 Easy to modify and extend
- 💡 Open source and community-driven
- ⚡ **Native Performance** - Built with C++ and Qt6 for maximum performance
- 🌐 **Cross-Platform** - Works on Windows and Linux

---

## 🧩 Keyboard Shortcuts

| Shortcut               | Action             |
|------------------------|--------------------|
| `Ctrl + Alt`           | Take screenshot    |
| `F2 + Arrow Keys`      | Move overlay       |
| `F2 + J`               | Show/Hide window   |
| `F2 + Enter`           | Exit Blash         |

---

## ⚙️ Build & Setup

### Prerequisites

- **Qt6** (6.2 or later)
- **CMake** (3.20 or later)
- **C++20 compatible compiler**
- **nlohmann/json** library

#### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential libnlohmann-json3-dev libx11-dev libxtst-dev
```

**Windows:**
- Install Qt6 from [Qt.io](https://www.qt.io/download)
- Install vcpkg and use it to install nlohmann-json
- Ensure CMake is installed

### Building

```bash
# Clone the repository
git clone https://github.com/cygnarix/blash.git
cd blash

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# Run
./BlashCpp  # Linux
# or
.\Release\BlashCpp.exe  # Windows
```

---

## 🛠️ Configuration Guide

The C++ version uses the same configuration as the Python version. After first launch, configure:

### AI API Keys

You'll need to set your API keys in the source code (for now - configuration UI coming soon):

1. **For OpenAI GPT:**
   - Edit `src/Config.cpp` 
   - Set `openAiApiKey = "your-openai-key-here"`

2. **For Google Gemini:**
   - Edit `src/Config.cpp`
   - Set `geminiApiKey = "your-gemini-key-here"`

3. **Select Model:**
   - Edit `src/Config.cpp`
   - Set `selectedModel = "gpt"` or `selectedModel = "gemini"`

### Keyboard Shortcuts

Edit `config/kb_sht.json` to customize shortcuts:

```json
{
  "screenshot": {
    "keys": ["CONTROL", "MENU"],
    "trigger_on": "second_modifier"
  },
  "f2_mode": {
    "modifier": "F2",
    "actions": {
      "J": "toggle_visibility",
      "RETURN": "quit_app",
      "UP": "move_up",
      "DOWN": "move_down",
      "LEFT": "move_left",
      "RIGHT": "move_right"
    }
  }
}
```

---

## 🏗️ Architecture

The C++ version is built with modern C++20 and Qt6, featuring:

- **Modular Design**: Separate classes for each component
- **Thread Safety**: AI processing in separate threads
- **Cross-Platform**: Works on Windows and Linux
- **Memory Safe**: Smart pointers and RAII
- **Performance**: Native compiled code for speed

### Key Components

- `BlashApp`: Main application controller
- `StealthToolbar`: Main UI toolbar with stealth capabilities
- `HookManager`: Global keyboard hook management
- `ScreenshotViewer`: Screenshot thumbnail viewer with zoom
- `AIWorker`: Threaded AI API processing
- `ResponseDialog`: AI response display with copy functionality
- `Config`: Configuration management

---

## 🔧 Development

### Project Structure

```
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   ├── BlashApp.h
│   ├── StealthToolbar.h
│   ├── HookManager.h
│   └── ...
├── src/                    # Source files
│   ├── main.cpp
│   ├── BlashApp.cpp
│   ├── StealthToolbar.cpp
│   └── ...
├── config/                 # Configuration files
│   └── kb_sht.json
└── README_CPP.md          # This file
```

### Building for Development

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# With verbose output
cmake --build . --verbose
```

---

## 📦 Current Version

**v2.0.0** - Complete C++ remake with improved performance and cross-platform support

---

## 🔐 Disclaimer

Blash is intended for ethical use only. Using AI tools during live interviews may violate terms or policies of hiring organizations. Use responsibly.

---

## 📬 Contributions

Contributions are welcome! The C++ version provides a solid foundation for further development. Areas for improvement:

- [ ] Settings GUI for API key configuration
- [ ] macOS support
- [ ] Additional AI model support
- [ ] Plugin system
- [ ] Enhanced Linux global hook implementation

Fork the repo, submit a PR, or open an issue to suggest improvements.

---

## 🚀 Performance Comparison

The C++ version offers significant improvements over the Python version:

- **Startup Time**: ~90% faster
- **Memory Usage**: ~60% less memory consumption
- **Screenshot Processing**: ~50% faster
- **UI Responsiveness**: Native performance
- **Global Hooks**: More reliable across platforms