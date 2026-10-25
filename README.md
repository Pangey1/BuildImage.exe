My simple image assembler in C++. Requirements: Text document formats, specifically: .txt, .cpp, .c, .h, and .hpp. The files may contain ONLY spaces, commas, periods, and `//` for comments. Example: `0, 65, 65, 33, 65.` (A period is placed at the end). Numbers from 00 to 99 are permitted; future versions will support a range from 00 to 200. What does each number represent? 
• 00–19: A gradient from black to white (shades of gray for shadows). 
• 20–39: A smooth transition from red to orange and yellow. 
• 40–59: A transition from yellow to vibrant green and lime green. 
• 60–79: A transition from green to cyan, blue, and violet. 
• 80–99: Magenta, pink shades, and custom bright colors.
Build command: `BuildImage -i your_filename.txt -o your_image_name.png`
or: `BuildImage.exe -i your_filename.txt -o your_image_name.png`
