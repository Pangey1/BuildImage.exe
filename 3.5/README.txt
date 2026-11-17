# BuildImage — Compiler 'Creation of Pictures & Videos'

Support for video assembly has been added!
You will need `ffmpeg.exe` to create videos.

## Download FFmpeg
Link to `ffmpeg.exe`: https://disk.yandex.ru/d/O8CSv2tNEe9Buw

## Video Compilation Command
```cmd
BuildImage.exe -o your_filename.txt -f ffmpeg.exe -b your_name.mp4
```
*(Other video formats like `.avi`, `.mkv`, `.mov` are also supported. Default frame rate is 60 FPS, or use `-fps <number>`).*

## Video Source Syntax
* Start the file with `sound("audio.mp3")` (or `sound("")` for no audio).
* Separate frames (minimum 2 frames of equal size) with `};`.
* End the last frame with a dot `.`.
