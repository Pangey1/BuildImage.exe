My simple image assembler in C++. Requirements: Supported text document formats include .txt, .cpp, .c, .h, and .hpp. The files may contain ONLY spaces, commas, periods, and `//` for comments. Example: `0, 65, 65, 33, 65.` (A period is placed at the very end). Numbers from 00 to 99 are permitted; here is what each range represents:
• 00–19: A gradient from black to white (grayscale shades for shadows). 
• 20–39: A smooth transition from red to orange and yellow. 
• 40–59: A transition from yellow to vibrant green and lime green. 
• 60–79: A transition from green to cyan, blue, and violet. 
• 80–99: Magenta and pink shades, as well as custom bright colors.
Build command: `BuildImage -i your_filename.txt -o your_image_name.png`
or: `BuildImage.exe -i your_filename.txt -o your_image_name.png`
