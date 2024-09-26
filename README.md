# Rags to Raccoon

Author: Sarah Di (sarahdi)

Design: My game is about a raccoon spending a day in New York.

Text Drawing: The main files used for Text Drawing are Font.cpp/Font.hpp and TextureProgram.cpp/hpp. The text for this game is precomputed and rendered at runtime using code adapted from Qiru Hu's Font.cpp/hpp (with permission!). Using the TextureProgram.cpp/hpp by Jim McCann which was covered in class, I then render the texture into the blender scene.

(TODO: how does the text drawing in this game work? What files or utilities are involved?)

Choices: The game stores choices in Story.cpp/hpp. Since there are only three endings and no additional variables/inventory, I just hardcoded all the possible body text responses using a struct of string vectors.

Screen Shot:

![Screen Shot](screenshot.png)

How To Play:

You are a Raccoon and are really excited to spend a day in the Big Apple!
Used left and right arrow keys to switch between options. Press return to enter a choice.

Sources: I made the blender scene, the music, and the script myself. I got the Roboto font from from Google Fonts. I adapted the text drawing from a combination of Qiru Hu's Font.cpp/hpp and Jim McCann's TextureProgram.cpp/hpp.

This game was built with [NEST](NEST.md).

