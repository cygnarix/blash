import io
import base64
from openai import OpenAI
from PIL import Image
from PyQt6.QtGui import QPixmap, QImage
from PyQt6.QtCore import QBuffer, QIODevice


API_KEY = "YOUR_API_KEY_HERE" #PAID - Your OpenAI API key here
MODEL_NAME = "gpt-4o" # options: gpt-4o, gpt-4o-mini, o3-mini, o3-mini-high, o1

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
    if qimage.format() not in (QImage.Format.Format_RGBA8888_Premultiplied, QImage.Format.Format_RGB32, QImage.Format.Format_ARGB32_Premultiplied, QImage.Format.Format_ARGB32):
         qimage = qimage.convertToFormat(QImage.Format.Format_ARGB32) # Use ARGB32 for PNG with alpha

    buf = QBuffer()
    buf.open(QIODevice.OpenModeFlag.WriteOnly)
    if not qimage.save(buf, "PNG"):
        return None
    buf.seek(0)
    img_data = buf.data()
    img = Image.open(io.BytesIO(img_data))
    return img

def pil_to_base64(image: Image.Image) -> str:
    buffered = io.BytesIO()
    image.save(buffered, format="PNG")
    img_str = base64.b64encode(buffered.getvalue()).decode('utf-8')
    return img_str

def generate_code_from_screenshots_gpt(pixmaps: list[QPixmap]) -> str:
    if not API_KEY or API_KEY == "YOUR_API_KEY_HERE":
        return "Error: OpenAI API Key not configured in gpt.py."

    try:
        client = OpenAI(api_key=API_KEY)

        content = [{"type": "text", "text": fixed_prompt}]
        valid_images_processed = 0

        for p in pixmaps:
            if p and not p.isNull():
                pil_img = qpixmap_to_pil(p)
                if pil_img:
                    base64_image = pil_to_base64(pil_img)
                    content.append({
                        "type": "image_url",
                        "image_url": {
                             "url": f"data:image/png;base64,{base64_image}"
                        }
                    })
                    valid_images_processed += 1

        if valid_images_processed == 0:
            return "Error: No valid images to process."

        response = client.chat.completions.create(
            model=MODEL_NAME,
            messages=[
                {
                    "role": "user",
                    "content": content,
                }
            ],
            max_tokens=4000
        )
        return response.choices[0].message.content

    except Exception as e:
        return f"Error during OpenAI API call: {e}"