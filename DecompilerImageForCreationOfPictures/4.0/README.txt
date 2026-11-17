# DecompilerImageForCreationOfPictures (v4.0)

In version **4.0**, support for **GIF animation decompilation** and an **instant 3D RGB color lookup cache** (up to 50–100x faster processing) has been added!

## Requirements
To decompile **videos** and **GIF animations**, you will need `ffmpeg.exe`.
* **Download `ffmpeg.exe`:** [Yandex Disk](https://yandex.ru)

*(Note: `ffmpeg.exe` is not required when decompiling static images).*

---

## What's New in v4.0
* **GIF Animation Support (`.gif`):** Automatically splits animated GIFs into frames without prompting for audio or adding a `sound("")` header. The output `.txt` file is ready to compile directly with:
  ```cmd
  BuildImage.exe -g file.txt -f ffmpeg.exe -o out.gif
  ```
* **3D Color Lookup Table (LUT):** Precomputes a `32x32x32` RGB color cache at startup for ultra-fast palette matching on large videos and GIFs.
* **Buffered Frame Output:** Writes complete frames to disk in single buffered blocks for maximum speed.

---

## How to Use

### 1. Drag & Drop
Drag and drop any supported image, GIF, or video (up to **2 GB**) directly onto `DecompilerImageForCreationOfPictures.exe`.

### 2. Command Line
Run via command line with a custom FFmpeg path:
```cmd
DecompilerImageForCreationOfPictures.exe animation.gif -f C:\(\path\to\ffmpeg.\)exe
```

---

## Supported Formats
* **Images:** `.png`, `.jpg`, `.jpeg`, `.bmp`, `.webp`, `.ico`, `.tiff`, `.svg`
* **GIF:** `.gif` *(animated and static)*
* **Videos:** `.mp4`, `.avi`, `.mkv`, `.mov`, `.webm`, `.wmv`, `.flv`, `.m4v`, `.mpeg`, `.mpg`, `.3gp`
