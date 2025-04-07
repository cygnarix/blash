import io
import google.generativeai as genai
from PIL import Image
from PyQt6.QtGui import QPixmap, QImage
from PyQt6.QtCore import QBuffer, QIODevice

API_KEY = "YOUR_API_KEY_HERE" #Goto https://aistudio.google.com/apikey (Google's website) to get your own FREE API key
MODEL_NAME = "gemini-2.0-flash" #Model names: gemini-2.0-flash, gemini-2.0-flash-lite, gemini-2.5-pro-preview-03-25, gemini-1.5-pro, gemini-2.0-flash-thinking-exp-01-21

fixed_prompt = (
    "Greetings. Imagine you're a senior software engineer, a very skilled individual, in terms of programming, "
    "so your approach of ANY coding task would be to break down the problem, "
    "and try your best to solve those problems, and code the entire problem. "
    "In all of these screenshots, piece ALL of the text of the screenshots together "
    "to identify the coding task(s). Now solve the coding task(s) in Python. "
    "Here is your response format:\n"
    "1. Thinking process (By the way, organize the thinking process into points)\n"
    "2. The code/solution\n"
    "3. A detailed explanation\n"
    "4. A summary\n"
    "Be as ACCURATE in your code solution as possible. Also do the bonus challenge(s).\n"
    "Try to achieve ALL of the requirements in the coding task(s), and put in AS MUCH effort AS POSSIBLE\n"
    "Write clean and simple code, and if the problem requires, be creative as well."
)

def qpixmap_to_pil(pixmap: QPixmap) -> Image.Image | None:
    qimage = pixmap.toImage()
    if qimage.isNull():
        return None
    if qimage.format() not in (QImage.Format.Format_RGBA8888_Premultiplied, QImage.Format.Format_RGB32):
        qimage = qimage.convertToFormat(QImage.Format.Format_RGBA8888_Premultiplied)
    buf = QBuffer()
    buf.open(QIODevice.OpenModeFlag.WriteOnly)
    if not qimage.save(buf, "PNG"):
        return None
    img = Image.open(io.BytesIO(buf.data()))
    return img.convert("RGB") if img.mode in ("RGBA", "P") else img

def generate_code_from_screenshots(pixmaps: list[QPixmap]) -> str:
    try:
        genai.configure(api_key=API_KEY)
        client = genai.GenerativeModel(MODEL_NAME)
        images = [img for img in (qpixmap_to_pil(p) for p in pixmaps if not p.isNull()) if img]
        if not images:
            return "Error: No valid images to process."
        safety = [
            {"category": c, "threshold": "BLOCK_NONE"}
            for c in (
                "HARM_CATEGORY_HARASSMENT",
                "HARM_CATEGORY_HATE_SPEECH",
                "HARM_CATEGORY_SEXUALLY_EXPLICIT",
                "HARM_CATEGORY_DANGEROUS_CONTENT",
            )
        ]
        resp = client.generate_content(contents=[fixed_prompt] + images, safety_settings=safety)
        return resp.text
    except Exception as e:
        return f"Error during Gemini API call: {e}"