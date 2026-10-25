A simple C++ tool I created for assembling images. Supported text file formats include .txt, .cpp, .c, .h, and .hpp. Files may contain ONLY spaces, commas, periods, and `//` symbols for comments. Example: `0, 65, 65, 33, 65.` (a period is placed at the end). Numbers from 00 to 99 are currently supported; future versions will support a range from 000 to 201. What does each number represent?
Versions 1.0 - 1.5:
• 00–19: A gradient from black to white (grayscale shades for shadows).
• 20–39: A smooth transition from red to orange and yellow.
• 40–59: A transition from yellow to bright green and lime green.
• 60–79: A transition from green to cyan, blue, and violet.
• 80–99: Magenta, shades of pink, and other vibrant colors.
New versions:
• 000 – 040: A perfect range of grays, from deepest black (000) to pure white (040). Excellent for creating shadows.
• 041 – 080: A vibrant, fiery transition from bright red to rich orange and yellow.
• 081 – 120: A smooth gradient shifting from yellow to lush green and lime.
• 121 – 160: A beautiful transition from green to sky blue and deep blue.
• 161 – 200: A fluid palette of purples and pinks, culminating in a vivid neon hue at 200.
Build command: `BuildImage -i your_filename.txt -o your_image_name.png`
or: `BuildImage.exe -i your_filename.txt -o your_image_name.png`
