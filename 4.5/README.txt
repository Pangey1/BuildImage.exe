# BuildImage (v4.5) — Compiler for Pictures, GIFs & Videos

## What's New in v4.5
In version **4.5**, full automatic support for **3 pixel code formats** without interactive prompts has been added:

1. **Decimal (`000`–`200`):** Classic palette numbers (`000, 050, 200`).
2. **Pure HEX (`00`–`FF`):** Compact two-character hex codes without a prefix (`00, 4B, C8, FF`). Tokens containing letters `A–F` are recognized automatically; add the `-hex` flag if your file contains only numeric hex codes (`10`, `55`, `80`).
3. **C++ / Code HEX (`0x00`–`0xFF`):** Standard programming hex format (`0x00, 0x4B, 0xC8, 0xFF`), ready for C/C++, C#, Python arrays, and raw binary dumps (such as 512-byte MBR sectors).

*(All features from previous versions — GIF compilation via `-g`, CLI help via `-h help`, and Zero-RAM streaming frame rendering — remain fully supported).*

---

## CLI Commands & Usage

### 1. Help & Reference
```cmd
BuildImage.exe -h help
```

### 2. Compile Static Image (`.bmp`)
```cmd
BuildImage.exe -o image.txt -b picture.bmp
```
*(Optional: add `-hex` to force pure HEX parsing for numeric-only hex files).*

### 3. Compile GIF Animation (`.gif`)
```cmd
BuildImage.exe -g anim.txt -f ffmpeg.exe -o anim.gif
```
* Optional flags: `-fps 24` (default is `60` FPS), `-hex`.

### 4. Compile Video (`.mp4`, `.avi`, `.mkv`, `.mov`, etc.)
```cmd
BuildImage.exe -o movie.txt -f ffmpeg.exe -b video.mp4
```
* Optional flags: `-fps 30` (default is `60` FPS), `-hex`.

---

## Source File Syntax
* **Static Image:** Pixel matrix ending with a single dot `.`.
* **GIF Animation (`-g`):** No `sound()` header required. Separate frames (minimum 2 frames of equal size) with `};`, and close the final frame with `.`.
* **Video (`-o ... -b ...`):** Start with `sound("audio.mp3")` (or `sound("")` for no audio), separate frames with `};`, and close the final frame with `.`.
