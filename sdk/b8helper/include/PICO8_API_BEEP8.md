
# PICO-8-like API for BEEP-8 (C/C++)

This document provides a reference for the PICO-8-like API implemented for the BEEP-8 system in C/C++. This API helps developers familiar with PICO-8 (a Lua-based fantasy console) transition to creating applications for BEEP-8 using C/C++.

## API Overview

### Enum: `Color`
The `Color` enum represents the 16-color palette used in BEEP-8, fully compatible with PICO-8's palette.

- `BLACK` (0): Black
- `DARK_BLUE` (1): Dark Blue
- `DARK_PURPLE` (2): Dark Purple
- `DARK_GREEN` (3): Dark Green
- `BROWN` (4): Brown
- `DARK_GREY` (5): Dark Grey
- `LIGHT_GREY` (6): Light Grey
- `WHITE` (7): White
- `RED` (8): Red
- `ORANGE` (9): Orange
- `YELLOW` (10): Yellow
- `GREEN` (11): Green
- `BLUE` (12): Blue
- `LAVENDER` (13): Lavender
- `PINK` (14): Pink
- `LIGHT_PEACH` (15): Light Peach
- `CURRENT`: The current color state (used in drawing functions)

### Function: `pal(Color c0, Color c1, u8 palsel=0)`
Replaces instances of a specified color `c0` with another color `c1` in the selected palette. This function modifies the palette used for drawing operations.

**Parameters:**
- `c0`: The original color to replace (from the `Color` enum).
- `c1`: The new color to use (from the `Color` enum).
- `palsel`: The palette selection index (default is `0`). BEEP-8 allows multiple palettes, so this argument specifies which palette to modify.

**Example:**
```cpp
pal(Color::DARK_GREEN, Color::LIGHT_GREY);  // Replaces DARK_GREEN with LIGHT_GREY in the default palette.
pal(Color::DARK_PURPLE, Color::BLUE, 1);    // Replaces DARK_PURPLE with BLUE in palette 1.
```

**Notes:**
- Unlike PICO-8, BEEP-8 supports multiple palettes (up to 16).
- In PICO-8, the third argument modifies either the draw or screen palette, but in BEEP-8, it selects which palette to modify.

### Function: `cls(Color color = BLACK)`
Clears the entire screen to the specified color. By default, the screen is cleared to black (color index 0). Optionally, a different color can be provided.

**Parameters:**
- `color`: The color to clear the screen with (default is `BLACK`).

### Function: `rectfill(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color = CURRENT)`
Draws a filled rectangle between `(x0, y0)` and `(x1, y1)`.

**Parameters:**
- `x0, y0`: Coordinates of the top-left corner.
- `x1, y1`: Coordinates of the bottom-right corner.
- `color`: The color of the rectangle. Defaults to the current draw color.

### Function: `line(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color = CURRENT)`
Draws a line from `(x0, y0)` to `(x1, y1)`.

**Parameters:**
- `x0, y0`: Starting point coordinates.
- `x1, y1`: Ending point coordinates.
- `color`: The color of the line. Defaults to the current draw color.

### Function: `circ(fx8 x, fx8 y, fx8 r = fx8(4), Color col = CURRENT)`
Draws a circle with center `(x, y)` and radius `r`.

**Parameters:**
- `x, y`: The center of the circle.
- `r`: Radius (default is `4`).
- `col`: Color of the circle (default is the current draw color).

### Function: `spr(int n, fx8 x = fx8(0), fx8 y = fx8(0), u8 w = 1, u8 h = 1, bool flip_x = false, bool flip_y = false, u8 selpal = 0)`
Draws a sprite from the sprite sheet at position `(x, y)`.

**Parameters:**
- `n`: Sprite number.
- `x, y`: Coordinates to draw the sprite.
- `w, h`: Width and height of the sprite (in number of sprites).
- `flip_x, flip_y`: Boolean values to flip the sprite horizontally or vertically.
- `selpal`: Palette selection index (default is `0`).

---

## Example Usage

Here is a simple example of how to use the `Pico8` class in BEEP-8:

```cpp
#include "pico8.h"

class MyApp : public pico8::Pico8 {
public:
    void _init() override {
        // Initialization code
    }

    void _update() override {
        // Game logic
    }

    void _draw() override {
        // Draw to the screen
        cls(pico8::Color::BLACK);
        spr(1, 10, 10);
        rectfill(20, 20, 40, 40, pico8::Color::RED);
    }
};

int main() {
    MyApp app;
    app.run();
    return 0;
}
```

---

## Key Differences from PICO-8

1. **Multiple Palettes:**  
   BEEP-8 allows selecting from multiple palettes (up to 16), unlike PICO-8, which only allows switching between the draw and screen palettes.

2. **C/C++ Environment:**  
   While PICO-8 is Lua-based, BEEP-8 runs in a C/C++ environment, offering improved performance and access to C/C++ libraries and functions.

3. **Enhanced Drawing Functions:**  
   Drawing functions such as `rectfill` and `circ` in BEEP-8 are slightly different from PICO-8, including additional features like depth handling with `setz()`.
