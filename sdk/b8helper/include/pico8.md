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
