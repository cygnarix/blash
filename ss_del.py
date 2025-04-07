import os

current_directory = os.getcwd()

for filename in os.listdir(current_directory):
    if filename.endswith(".png"):
        file_path = os.path.join(current_directory, filename)
        os.remove(file_path)
    