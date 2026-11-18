# DecompilerImageForCreationOfPictures (v4.5)

## What's New in v4.5
In version **4.5**, an **interactive 3-mode output format selector** has been added so you can export pixel matrices in the exact format you need:

1. **`1` (`-dec`) — Decimal (`000`–`200`):** Classic three-digit palette numbers (`000, 050, 200`).
2. **`2` (`-hex`) — Pure HEX (`00`–`C8`):** Compact two-character hex codes without a prefix (`00, 4B, C8`).
3. **`3` (`-0x`) — C++ / Code HEX (`0x00`–`0xC8`):** Standard programming hex format (`0x00, 0x4B, 0xC8`), ready to paste directly into C/C++, C#, Java, or Python arrays.

*(All features from previous versions — GIF animation decompilation without a `sound()` header, video audio extraction, and the instant 3D RGB color cache — remain fully supported).*

---

## How to Use

### 1. Drag & Drop (Interactive Mode)
Drag and drop any supported image, `.gif`, or video (up to **2 GB**) directly onto `DecompilerImageForCreationOfPictures.exe`, then choose format **`1`**, **`2`**, or **`3`** in the console.

### 2. Command Line (Silent / Batch Mode)
Pass a format flag directly to skip the interactive prompt:
```cmd
DecompilerImageForCreationOfPictures.exe picture.bmp -hex
DecompilerImageForCreationOfPictures.exe animation.gif -0x
```

---

## Supported Formats
* **Images:** `.png`, `.jpg`, `.jpeg`, `.bmp`, `.webp`, `.ico`, `.tiff`, `.svg`
* **GIF:** `.gif` *(animated and static)*
* **Videos:** `.mp4`, `.avi`, `.mkv`, `.mov`, `.webm`, `.wmv`, `.flv`, `.m4v`, `.mpeg`, `.mpg`, `.3gp`
