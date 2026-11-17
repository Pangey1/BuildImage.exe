# BuildImage (v4.0) — Compiler for Pictures, GIFs & Videos

In version **4.0**, support for compiling **GIF animations**, **built-in CLI help**, and **Zero-RAM streaming frame rendering** (allowing you to compile multi-gigabyte video projects without memory limits) has been added!

## Requirements
To compile **videos** and **GIF animations**, you will need `ffmpeg.exe`.
* **Download `ffmpeg.exe`:** [Yandex Disk](https://disk.yandex.ru/d/O8CSv2tNEe9Buw)

*(Note: `ffmpeg.exe` is not required for compiling static `.bmp` images).*

---

## CLI Commands & Usage

### 1. Help & Instructions
Display the built-in command reference directly in the console:
```cmd
BuildImage.exe -h help
```

### 2. Compile Static Image (`.bmp`)
```cmd
BuildImage.exe -o image.txt -b picture.bmp
```

### 3. Compile GIF Animation (`.gif`) — *NEW in v4.0*
```cmd
BuildImage.exe -g anim.txt -f ffmpeg.exe -o anim.gif
```
* Optional frame rate flag: `-fps 24` (default is `60` FPS).

### 4. Compile Video (`.mp4`, `.avi`, `.mkv`, `.mov`, etc.)
```cmd
BuildImage.exe -o movie.txt -f ffmpeg.exe -b video.mp4
```
* Optional frame rate flag: `-fps 30` (default is `60` FPS).

---

## Source File Syntax

* **Palette Values:** Decimal (`0`–`200`) or Hexadecimal (`0x00`–`0xC8`), separated by commas.
* **Static Image:** Pixel matrix ending with a single dot `.`.
* **GIF Animation (`-g`):** No `sound()` header required. Separate frames (minimum 2 frames of equal size) with `};`, and close the final frame with `.`.
* **Video (`-o ... -b ...`):** Start with `sound("audio.mp3")` (or `sound("")` for no audio), separate frames with `};`, and close the final frame with `.`.
