# Blash

## Note: This is only version 1.0.0, updates coming soon!

**Blash** is a better, open-source alternative to Interview Coder — designed to be stealthy, customizable, and powerful. It offers real-time coding assistance with support for multiple AI models, and it works invisibly with many popular apps — including the latest version of Zoom (which Interview Coder cannot do).

## 🚀 Features

- 🧠 **Customizable AI Model Support** (Gemini or GPT)
- 🫥 **Stealth Mode** – Invisible in EVERY screen capturing app, including the LATEST verion of Zoom (Which Interview Coder isn't)
- 🎛️ **Configurable Shortcuts**
- 🔧 Easy to modify and extend
- 💡 Open source and community-driven

---

## 🧩 Keyboard Shortcuts

| Shortcut               | Action             |
|------------------------|--------------------|
| `Ctrl + Alt`           | Take screenshot    |
| `F2 + Arrow Keys`      | Move overlay       |
| `F2 + J`               | Show/Hide window   |
| `F2 + Enter`           | Exit Blash         |

---

## ⚙️ Setup & Configuration

```bash
# Clone the repository
git clone https://github.com/cygnarix/blash.git
cd blash

# Install dependencies
pip install -r requirements.txt

# Run the app
python main.py
```

---

## 🛠️ Configuration Guide

After first launch, make sure to configure the following files:

- `gemini.py`: Set your **Google Gemini API key** and configuration
- `gpt.py`: Set your **OpenAI API key** and model preferences
- `ss_mem.py`: Select the model (`gpt` or `gemini`)

### Example:

To use OpenAI GPT:

1. Open `ss_mem.py` and set:
   ```python
   MODEL_SELECTION = 'gpt'
   ```

2. Open `gpt.py` and update:
   ```python
   YOUR_API_KEY_HERE = 'your-openai-key-here'
   ```

To use Google Gemini:

1. Open `ss_mem.py` and set:
   ```python
   MODEL = 'gemini'
   ```

2. Open `gemini.py` and update:
   ```python
   YOUR_API_KEY_HERE = 'your-gemini-key-here' # Go to https://aistudio.google.com/app/apikey to get 1 for free
   ```

---

## 📦 Current Version

**v1.0.0**

---

## 🔐 Disclaimer

Blash is intended for ethical use only. Using AI tools during live interviews may violate terms or policies of hiring organizations. Use responsibly.

---

## 📬 Contributions

Contributions are welcome! Fork the repo, submit a PR, or open an issue to suggest improvements.
