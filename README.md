A simple C++ tool I created for assembling images. Supported text file formats include .txt, .cpp, .c, .h, and .hpp. Files may contain ONLY spaces, commas, periods, and `//` symbols for comments. Example: `0, 65, 65, 33, 65.` (a period is placed at the end). Numbers from 00 to 99 are currently supported; future versions will support a range from 000 to 201. What does each number represent?
• 00–19: A gradient from black to white (grayscale shades for shadows).
• 20–39: A smooth transition from red to orange and yellow.
• 40–59: A transition from yellow to bright green and lime green.
• 60–79: A transition from green to cyan, blue, and violet.
• 80–99: Magenta, shades of pink, and other vibrant colors.
Build command: `BuildImage -i your_filename.txt -o your_image_name.png`
or: `BuildImage.exe -i your_filename.txt -o your_image_name.png`
