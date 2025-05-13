/**
 * @file pico8.h
 * @brief PICO-8-like API definitions for C/C++ in the BEEP-8 environment.
 *
 * This file provides a set of utility functions and structures that emulate the 
 * PICO-8 API in a C/C++ environment. The goal of this library is to make it easier 
 * for developers familiar with PICO-8 to create applications for the BEEP-8 system.
 *
 * Although PICO-8 operates in a Lua environment, BEEP-8 is a C/C++ based system,
 * and as such, certain APIs differ to accommodate the new programming environment.
 * These APIs aim to maintain the simplicity and style of PICO-8 while leveraging
 * the performance and capabilities of C/C++.
 *
 * @note This library is not mandatory for developing applications for BEEP-8.
 *       Developers can build BEEP-8 applications without using this library if preferred.
 *
 * The BEEP-8 system offers a C/C++ environment for developing retro-style games,
 * and this library provides familiar tools for those transitioning from PICO-8 to 
 * the BEEP-8 platform.
 */
#pragma once
#include <vector>
#include <span> 
#include <b8/type.h>
#include <b8/assert.h>
#include <b8/ppu.h>
#include <b8/misc.h>
#include <trace.h>
#include <handle.h>
#include <submath.h>
#include <stdarg.h>
#include <memory>
#include <optional> 

/*
  TODO: Comparison table with original PICO-8
  TODO: Standardize naming with tget / tset
  TODO: Implement sset(x, y, [col]) -- Sets the color of a pixel on the sprite sheet
*/
namespace pico8 {

#define MUST(cond, errcode) \
    do { \
        if (!(cond)) { \
            seterr(errcode); \
            return; \
        } \
    } while(0)

#define MUST_RETURN(cond, errcode, retval) \
    do { \
        if (!(cond)) { \
            seterr(errcode); \
            return (retval); \
        } \
    } while(0)



  /**
   * @brief Represents the color palette for drawing operations.
   *
   * This enumeration defines a set of colors that are used in drawing functions.
   * The color values correspond directly to the PICO-8's default 16-color palette.
   * 
   * The last value, `CURRENT`, represents the current color state, which allows
   * drawing functions to continue using the previously set color.
   * 
   * @note These colors are aligned with the default PICO-8 palette, making this
   * enumeration fully compatible with PICO-8's color indexing system.
   */
  enum Color {
    BLACK,          ///< Color index 0: Black.
    DARK_BLUE,      ///< Color index 1: Dark Blue.
    DARK_PURPLE,    ///< Color index 2: Dark Purple.
    DARK_GREEN,     ///< Color index 3: Dark Green.
    BROWN,          ///< Color index 4: Brown.
    DARK_GREY,      ///< Color index 5: Dark Grey.
    LIGHT_GREY,     ///< Color index 6: Light Grey.
    WHITE,          ///< Color index 7: White.
    RED,            ///< Color index 8: Red.
    ORANGE,         ///< Color index 9: Orange.
    YELLOW,         ///< Color index 10: Yellow.
    GREEN,          ///< Color index 11: Green.
    BLUE,           ///< Color index 12: Blue.
    LAVENDER,       ///< Color index 13: Lavender.
    PINK,           ///< Color index 14: Pink.
    LIGHT_PEACH,    ///< Color index 15: Light Peach.
    CURRENT         ///< Special value representing the current color.
  };

  enum  BgPal {
    BG_PAL_0,
    BG_PAL_1,
    BG_PAL_2,
    BG_PAL_3,
    /* --- */
    BG_PAL_CURRENT
  };

  /**
   * @brief Defines error codes for drawing operations.
   *
   * This enumeration represents different error codes that may be encountered
   * during the execution of drawing functions. Each value corresponds to a specific
   * error condition, which can be used for error handling and debugging.
   * 
   * @note The `NO_ERROR` value indicates that no error has occurred, while other
   * values represent specific error cases.
   */
  enum Error {
    NO_ERROR,           ///< No error has occurred.
    NOT_DURING_DRAWING, ///< Attempt to perform drawing outside of a valid drawing context.
    INVALID_PARAM,      ///< An invalid parameter was passed to a function.
    NOT_INITIALIZED,
    EMPTY_SPAN
  };
  void  seterr( Error error );

  enum MouseBtn {
    LEFT      = 1<<0,
    RIGHT     = 1<<1,  // Right button (currently unused, reserved for future use)
  };

  enum Button {
    BUTTON_LEFT  = 0, ///< Left button
    BUTTON_RIGHT = 1, ///< Right button
    BUTTON_UP    = 2, ///< Up button
    BUTTON_DOWN  = 3, ///< Down button
    BUTTON_O     = 4, ///< O button ('z' key on keyboard)
    BUTTON_X     = 5, ///< X button ('x' key on keyboard)
    BUTTON_MOUSE_LEFT = 6,
    /* --- */
    BUTTON_ANY   = 0x10,
    /* --- */
    BUTTON_MAX
    // Note: Unlike PICO-8, BEEP-8 is primarily designed for smartphone use.
    // These buttons are only functional on PC platforms and referencing them is discouraged
    // for applications intended to run on mobile devices.
  };

  /**
   * @brief Clears the entire screen to the specified color.
   *
   * This function clears the entire graphics buffer, setting every pixel to the specified color.
   * By default, the screen is cleared to black (color index 0). Optionally, a different color index
   * can be provided to clear the screen to a different color.
   *
   * @param color The color index to use for clearing the screen. The default is BLACK (0).
   * 
   * @note This function does not respect the clip() settings, and will always clear the entire screen.
   * If you wish to clear only a part of the screen, use rectfill() instead.
   * 
   * @note This function is not affected by setz(), and will always clear the screen at the lowest depth.
   * It is generally recommended to call this function at the beginning of the _draw() function.
   * 
   * @see rectfill()
   * @see setz()
   */
  void cls(Color color = BLACK);

  /**
   * @brief Sets the current draw color for future drawing operations.
   *
   * This function sets the active color in the draw state, which will be used for all drawing
   * functions that do not explicitly specify a color. The color is represented by an index value
   * that corresponds to a predefined palette.
   * 
   * @param color The color to use for drawing. It should be of type `Color`,
   * representing a value from the predefined palette.
   * 
   * @return The previous draw color before the update, allowing it to be restored if needed.
   * 
   * @note This function does not affect the current depth set by setz(). Graphics functions that do not 
   * specify a color explicitly will use the color set by this function. 
   * to those functions, it overrides the current draw color, and the draw state is updated.
   * 
   * @note This function is commonly called at the beginning of a drawing routine to set the desired pen color.
   * 
   * @note Unlike PICO-8, the drawing color and text color are managed separately in this API.
   * This function does not affect the text color. If you want to change the text color, 
   * please use the `sprint()`, `cursor()`, or escape sequences within strings to specify text colors.
   * 
   */
  Color color(Color color);

  /**
   * @brief Draws a filled rectangle on the screen.
   *
   * This function draws a filled rectangle from the top-left corner (x0, y0) to the bottom-right corner (x1, y1).
   * If (x0, y0) is specified as being lower-right of (x1, y1), the coordinates are automatically swapped to 
   * ensure a correct rectangle. Unlike PICO-8, the coordinates provided are NOT inclusive, meaning the pixels 
   * along the right edge (x1) and the bottom edge (y1) are not drawn.
   *
   * @param x0 The x-coordinate of the first corner (automatically set as the top-left if higher than x1).
   * @param y0 The y-coordinate of the first corner (automatically set as the top-left if higher than y1).
   * @param x1 The x-coordinate of the opposite corner (exclusive).
   * @param y1 The y-coordinate of the opposite corner (exclusive).
   * @param color The color of the rectangle and fill. If omitted, the color from the draw state is used.
   * 
   * @note  The depth (z-coordinate) of the rectangle is determined by a prior call to setz(). 
   *        If no depth is set, the default value from the current state will be used.
   */
  void rectfill(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color = CURRENT);

  /**
   * @brief Draws a pixel on the screen.
   *
   * This function sets a single pixel at the specified coordinates (x0, y0) with the given color.
   * Unlike `rectfill`, this function affects only the specified pixel, not an area or rectangle.
   *
   * @param x0 The x-coordinate of the pixel.
   * @param y0 The y-coordinate of the pixel.
   * @param color The color of the pixel. If omitted, the color from the current draw state is used.
   * 
   * @note This function is affected by the camera settings. If the camera has been moved, the drawing will
   *       reflect those changes.
   * 
   * @note The depth (z-coordinate) of the pixel is determined by a prior call to setz(). 
   *       If no depth is set, the default value from the current state will be used.
   */
  void pset(fx8 x0, fx8 y0, Color color = CURRENT);

  /**
   * @brief Draws the outline of a rectangle on the screen.
   *
   * This function draws the outline of a rectangle from the top-left corner (x0, y0) to the bottom-right corner (x1, y1).
   * Unlike PICO-8, the coordinates provided are NOT inclusive, meaning the pixels along the right edge 
   * (x1) and the bottom edge (y1) are not drawn. The outline is drawn by combining four filled rectangles to 
   * create the top, bottom, left, and right edges of the rectangle.
   *
   * @param x0 The x-coordinate of the top-left corner.
   * @param y0 The y-coordinate of the top-left corner.
   * @param x1 The x-coordinate of the bottom-right corner (exclusive).
   * @param y1 The y-coordinate of the bottom-right corner (exclusive).
   * @param color The color of the rectangle outline. If omitted, the color from the draw state is used.
   * 
   * @note This function is affected by the camera settings. If the camera has been moved, the drawing will
   *       reflect those changes. 
   * 
   * @note  The depth (z-coordinate) of the rectangle is determined by a prior call to setz(). 
   *        If no depth is set, the default value from the current state will be used.
   */
  void  rect(fx8 x0, fx8 y0, fx8 x1, fx8 y1, Color color = CURRENT);

  /**
   * @brief Draws a line between two points on the screen.
   *
   * This function draws a line from the starting point (x0, y0) to the ending point (x1, y1). 
   * Unlike PICO-8, the coordinates provided are NOT inclusive, meaning the pixel at the ending point (x1, y1) is not drawn.
   *
   * @param x0 The x-coordinate of the starting point.
   * @param y0 The y-coordinate of the starting point.
   * @param x1 The x-coordinate of the ending point (exclusive).
   * @param y1 The y-coordinate of the ending point (exclusive).
   * @param color The color of the line. If omitted, the color from the draw state is used.
   * 
   * @note This function is affected by the camera settings. If the camera has been moved, the drawing will
   *       reflect those changes. 
   * 
   * @note  The depth (z-coordinate) of the line is determined by a prior call to setz(). 
   *        If no depth is set, the default value from the current state will be used.
   */
  void  line(fx8 x0,fx8 y0,fx8 x1,fx8 y1, Color color = CURRENT );

  /**
   * @brief Draws a line using a `Line` structure.
   *
   * This function draws a line between two points specified by the `Line` structure. 
   * The line is drawn from the starting point (`ln.pos0`) to the ending point (`ln.pos1`).
   * The coordinates provided are NOT inclusive, meaning the pixel at the ending point 
   * is not drawn.
   *
   * @param ln A `Line` structure containing the starting and ending points of the line.
   * @param color The color of the line. If omitted, the color from the draw state is used.
   * 
   * @note This function is affected by the camera settings and any camera transformations will
   *       be applied to the line's coordinates.
   * 
   * @note The depth (z-coordinate) of the line is determined by a prior call to setz().
   *       If no depth is set, the default value from the current state will be used.
   */
  void  line(const Line& ln, Color color = CURRENT);

  /**
   * @brief Draws a filled triangle using a `Poly` structure.
   *
   * This function draws a filled triangle based on the three vertices specified in the `Poly`
   * structure. The triangle is filled with the specified color.
   *
   * @param pol A `Poly` structure containing the three vertices of the triangle.
   * @param color The color to fill the triangle. If omitted, the color from the draw state is used.
   * 
   * @note This function is affected by the camera settings, and any camera transformations will
   *       be applied to the triangle's vertices.
   * 
   * @note The depth (z-coordinate) of the triangle is determined by a prior call to setz().
   *       If no depth is set, the default value from the current state will be used.
   */
  void  poly(const Poly& pol, Color color = CURRENT);

  /**
   * @brief Draws a filled triangle using three vertices.
   *
   * This function draws a filled triangle using the three specified vertices `(x0, y0)`, 
   * `(x1, y1)`, and `(x2, y2)`. The triangle is filled with the specified color.
   *
   * @param x0 The x-coordinate of the first vertex.
   * @param y0 The y-coordinate of the first vertex.
   * @param x1 The x-coordinate of the second vertex.
   * @param y1 The y-coordinate of the second vertex.
   * @param x2 The x-coordinate of the third vertex.
   * @param y2 The y-coordinate of the third vertex.
   * @param color The color to fill the triangle. If omitted, the color from the draw state is used.
   * 
   * @note This function is affected by the camera settings, and any camera transformations will
   *       be applied to the triangle's vertices.
   * 
   * @note The depth (z-coordinate) of the triangle is determined by a prior call to setz().
   *       If no depth is set, the default value from the current state will be used.
   */
  void  poly(fx8 x0, fx8 y0, fx8 x1, fx8 y1, fx8 x2, fx8 y2, Color color = CURRENT);

  /**
     * @brief Draws a sprite or a range of sprites on the screen.
     *
     * This function draws a sprite from the sprite sheet at the specified position, optionally flipping 
     * the sprite horizontally or vertically. It can also draw a range of sprites by specifying the width (`w`) and height (`h`).
     * Additionally, a specific palette can be selected using the `selpal` parameter.
     *
     * @param n The sprite number within bank 0. This function only supports sprites from bank 0, making 
     *          the valid range for `n` [0, 255]. When drawing a range of sprites, `n` specifies the 
     *          upper-left corner of the range.
     * @param x The x coordinate (in pixels). The default is 0.
     * @param y The y coordinate (in pixels). The default is 0.
     * @param w The width of the range, in the number of sprites. Non-integer values can be used to draw partial sprites.
     *          Requires `h` to be specified as well. The default is 1.
     * @param h The height of the range, in the number of sprites. Non-integer values can be used to draw partial sprites.
     *          Required if `w` is specified. The default is 1.
     * @param flip_x If true, the sprite is drawn inverted left to right. The default is false.
     * @param flip_y If true, the sprite is drawn inverted top to bottom. The default is false.
     * @param selpal The palette selection for the sprite. The default is 0.
    
     * @note Only bank 0 is accessible for this function, limiting `n` to the range [0, 255]. The BEEP-8 
     *       system’s VRAM is divided into 16 banks of 128x128 pixels each, arranged in a 4x4 grid. While 
     *       other functions may access these additional banks, `spr()` is restricted to bank 0.
     * 
     * @note For drawing sprites from other banks, use `sprb()`, which allows specifying any bank (0-15) 
     *       instead of being limited to bank 0 as `spr()` is.
     * 
     * @note This function is affected by the camera settings. If the camera has been moved, the drawing will
     *       reflect those changes. 
     * 
     * @note The depth (z-coordinate) of the sprite is determined by a prior call to `setz()`. 
     *       If no depth is set, the default value from the current state will be used.
     */
  void spr(int n, fx8 x = fx8(0), fx8 y = fx8(0), u8 w = 1, u8 h = 1, bool flip_x = false, bool flip_y = false, u8 selpal = 0);

  /**
   * @brief Draws a sprite or a range of sprites on the screen from a specified VRAM bank.
   *
   * The `sprb` function draws a sprite from a specific VRAM bank on the BEEP-8 system. Unlike `spr`, which only allows 
   * access to bank 0, `sprb` enables drawing from any bank (0-15). The VRAM in BEEP-8 is structured as a 4-bit color 
   * 512x512 pixel space, divided into 16 banks arranged in a 4x4 grid. Each bank contains a 128x128 region of sprites, 
   * allowing for flexible sprite management.
   *
   * The bank layout is structured as follows:
   * ```
   *  0  1  2  3
   *  4  5  6  7
   *  8  9 10 11
   * 12 13 14 15
   * ```
   *
   * @param bank The bank index (0 to 15) specifying the VRAM region to draw from. This parameter determines
   *             the 128x128 portion of VRAM from which the sprites are sourced.
   * @param n The sprite number within the specified bank. When drawing a range of sprites, `n` specifies the 
   *          upper-left corner of the range.
   * @param x The x coordinate (in pixels). The default is 0.
   * @param y The y coordinate (in pixels). The default is 0.
   * @param w The width of the range, in the number of sprites. Non-integer values can be used to draw partial sprites.
   *          Requires `h` to be specified as well. The default is 1.
   * @param h The height of the range, in the number of sprites. Non-integer values can be used to draw partial sprites.
   *          Required if `w` is specified. The default is 1.
   * @param flip_x If true, the sprite is drawn inverted left to right. The default is false.
   * @param flip_y If true, the sprite is drawn inverted top to bottom. The default is false.
   * @param selpal The palette selection for the sprite. The default is 0.
   * 
   * @note If `w >= 2` or `h >= 2`, ensure that the specified sprite range does not exceed the boundaries of 
   *       the 16x16 grid within the bank, as this may result in undefined behavior.
   * 
   * @note This function is affected by the camera settings. If the camera has been moved, the drawing will
   *       reflect those changes. 
   * 
   * @note The depth (z-coordinate) of the sprite is determined by a prior call to `setz()`. 
   *       If no depth is set, the default value from the current state will be used.
   */
  void sprb(u8 bank, int n, fx8 x = fx8(0), fx8 y = fx8(0), u8 w = 1, u8 h = 1, bool flip_x = false, bool flip_y = false, u8 selpal = 0);

  /**
   * @brief Loads a sprite sheet into a specified VRAM bank on the BEEP-8 system.
   *
   * The `lsp` function (short for "load sprite sheet") transfers a 128x128 sprite sheet into a specified
   * VRAM bank within the BEEP-8 system's sprite/background pattern memory (VRAM). The VRAM structure is 
   * 4-bit color and spans 512x512 pixels (128KB total). Although the BEEP-8 architecture itself does not 
   * natively support a banked layout, this function manages VRAM in a 4x4 bank grid of 128x128 pixel 
   * sheets, with bank values from 0 to 15.
   * 
   * Note: Banks 14 and 15 are reserved for system use (e.g., font data).
   * Please do not specify them.
   *
   * The bank layout is structured as follows:
   * ```
   *  0  1  2  3
   *  4  5  6  7
   *  8  9 10 11
   * 12 13 14 15
   * ```
   *
   * This layout allows 16 banks, each representing a 128x128 section of the VRAM grid.
   * 
   * @param bank The bank index (0 to 15) specifying the destination bank in the VRAM.
   *             This index indicates which 128x128 portion of VRAM to populate.
   * @param srcimg Pointer to the source image data, which must be exactly 8192 bytes (4bpp, 128x128).
   *               The function transfers this data into the VRAM bank specified by `bank`.
   *
   * @pre `bank` must be in the range [0, 15].
   * @pre `srcimg` must point to an image buffer of size 8192 bytes (4bpp, 128x128).
   * @pre The specified `bank` must not be in use (`sprite_sheets[bank] == 0`).
   *
   * @note This function is blocking and will not return until the transfer is complete. After completion, 
   *       `srcimg` is no longer referenced by the PPU, ensuring that `srcimg` can be safely modified or 
   *       freed by the caller.
   * 
   * @warning Attempting to specify a bank that is already in use will trigger an assertion failure.
   * 
   * @warning The `srcimg` parameter must be an 8KB (8192 bytes) array representing a 4bpp, 128x128 image.
   *          If an array of any other size is specified, the behavior is undefined.
   * 
   * Example usage:
   *  * Example usage:
   * ```
   * // Convert .png images to C/C++ arrays by running `make` in the example application directory:
   * // ./beep8/sdk/app/pico8_example
   * // This command processes all *.png files in the ./beep8/sdk/app/pico8_example/data/import/ 
   * // directory and generates corresponding *.cpp files in ./beep8/sdk/app/pico8_example/data/export/.
   * //
   * // For example, if sprite0.png and sprite1.png are present, sprite0.png.cpp and sprite1.png.cpp
   * // will be generated with the following structure:
   * 
   * // sprite0.png.cpp content:
   * // // exported by png2c data/import/sprite0.png
   * // #include <stdint.h>
   * // extern const uint16_t b8_image_sprite0_width = 128;
   * // extern const uint16_t b8_image_sprite0_height = 128;
   * // extern const uint8_t b8_image_sprite0[(128 * 128) >> 1] = {
   * //     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, ... // image data
   * // };
   *
   * // Each array, like `b8_image_sprite0`, holds an image in 4bpp format with dimensions 128x128.
   * // To load this image into bank 0 of the sprite sheet in VRAM, use:
   * lsp(0, b8_image_sprite0);
   *
   * // Note: Although there are 16 banks available, bank 14 and bank 15 are reserved for system use
   * // (e.g., font data).
   * ```
   */
  void lsp(u8 bank,const uint8_t* srcimg);

  /**
   * @brief Sets the palette using the specified palette selection index and palette data.
   *
   * This function allows you to set a specific palette by providing a selection index and an array of 16 color indices.
   * The palette selection index determines which palette slot will be updated, and the array contains the color indices
   * for that palette.
   * 
   * @param palsel The palette selection index. Determines which palette slot to update.
   * @param pidx An array of 16 unsigned char values representing the color indices for the palette.
   *
   * @note Before setting the palette, ensure the z depth is set using the setz() function to define the depth
   *       for subsequent drawing operations. The depth set by setz() will affect how the palette is applied during rendering.
   * @note This function may only be called during the execution of draw(); calling it elsewhere is not permitted.
   *
   * @code
   * #include <array>
   * 
   * {
   *     // Set the color indices for the palette
   *     std::array<unsigned char, 16> palette = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
   * 
   *     // Palette selection index
   *     int palsel = 1;
   * 
   *     // Set the z depth
   *     setz(3);
   * 
   *     // Set the palette (must be called inside draw())
   *     setpal(palsel, palette);
   * 
   *     // Draw sprite
   *     spr(37, 100, 200, 1, 1, false, false, palsel);
   * 
   *     return 0;
   * }
   * @endcode
   */
  void setpal(int palsel, const std::array<unsigned char, 16>& pidx);

  /**
   * @brief Changes the palette for the draw operations.
   *
   * The pal() function replaces instances of a specified color (c0) with another color (c1)
   * in the selected palette. It allows for dynamic color manipulation in drawing operations 
   * like sprites and backgrounds, offering versatility in creating different visual effects.
   *
   * @param c0 The original color to replace. Must be a value from the Color enum, such as 
   *           `Color::DARK_GREEN` or `Color::RED`.
   * @param c1 The new color to use instead. Must be a value from the Color enum.
   * @param palsel The palette selection (default is 0). BEEP-8 allows multiple palettes,
   *               and this argument specifies which palette to modify. Must be within the
   *               range 0-15.
   *
   * The Color enum defines a set of colors that correspond to PICO-8's default 16-color palette,
   * ensuring compatibility with PICO-8 style color manipulation. The special value `Color::CURRENT`
   * is also provided to allow drawing operations to continue using the last set color, although 
   * it is not applicable in this context.
   *
   * Key differences from PICO-8:
   *  - The third parameter `palsel` allows selection of different palettes in BEEP-8, whereas 
   *    PICO-8 uses the third argument to specify whether to modify the draw or screen palette. 
   *    In BEEP-8, you modify the active palette by selecting it via `palsel`.
   *  - BEEP-8 supports multiple palettes (up to 16), allowing more advanced palette management.
   *
   * Usage examples:
   *  - pal(Color::DARK_GREEN, Color::LIGHT_GREY); // Replaces DARK_GREEN with LIGHT_GREY in the default palette.
   *  - pal(Color::DARK_PURPLE, Color::BLUE, 1);   // Replaces DARK_PURPLE with BLUE in palette 1.
   *
   * Ensure that the values for c0, c1, and palsel are within valid ranges. If an invalid 
   * parameter is provided, the function will throw an INVALID_PARAM error.
   */
  void pal( Color c0 , Color c1 , u8 palsel=0 );

  /**
   * @brief Sets the camera offset in the draw state.
   *
   * This function applies a camera offset to all subsequent drawing operations, shifting the view by the specified
   * x and y values. The offset remains active until `camera()` is called again with new values, making it persistent 
   * across multiple `_draw()` calls. If you wish to reset the offset, call `camera()` with the default values at the 
   * beginning of the `_draw()` function.
   *
   * @param camera_x The x-coordinate offset (in pixels). Defaults to 0.
   * @param camera_y The y-coordinate offset (in pixels). Defaults to 0.
   * 
   * @return An x, y tuple (`vec` object) representing the previous camera offset before the update.
   */
  const Vec& camera(fx8 camera_x = fx8(0), fx8 camera_y = fx8(0));

  /**
   * @brief Sets the depth (z-coordinate) for subsequent drawing operations.
   *
   * This function sets the depth, or z-coordinate, for all subsequent drawing operations.
   * Any drawing function that does not explicitly take a depth argument will use this value.
   * 
   * @param otz The z-coordinate (depth) value, which must be in the range [0, maxz()]. 
   *            Higher values are drawn behind lower values.
   * 
   * @return The previous z-coordinate before the update, allowing the depth to be restored if needed.
   *
   * @note The z-coordinate specified in this function must be between 0 and the value returned by maxz().
   *       This function must be called before any drawing operations where depth is required.
   *       Once set, the specified depth will persist until changed by another call to setz().
   */
  int setz(int otz);

  /**
   * @brief Retrieves the current depth (z-coordinate) value.
   *
   * This function returns the current z-coordinate that is being used for subsequent drawing operations.
   * The value returned reflects the last value set by setz(), clamped within the range [0, maxz()].
   *
   * @return The current z-coordinate (depth) value.
   */
  int getz();

  /**
   * @brief Returns the maximum allowable depth (z-coordinate) value for drawing operations.
   *
   * This function provides the upper limit of the z-coordinate that can be used with setz().
   * The value returned by this function indicates the maximum depth allowed, and values 
   * assigned to setz() must be in the range [0, maxz()].
   *
   * @return The maximum allowable z-coordinate (depth) value.
   * 
   * @note The z-coordinate specified in setz() must be greater than or equal to 0 and less than or 
   *       equal to the value returned by this function.
   *
   * @note Currently, this function returns 16. However, this value may be expanded in the future 
   *       to support deeper z-coordinate ranges.
   */
  int maxz();

  /**
   * @brief Draws an unfilled circle on the screen.
   * 
   * Draws a circle with the specified center coordinates (x, y) and radius r. 
   * If the radius is omitted, the default radius is 4. 
   * If the color is omitted, the current drawing color is used.
   * 
   * @param x The x coordinate of the center of the circle.
   * @param y The y coordinate of the center of the circle.
   * @param r The radius of the circle (optional, default is 4).
   * @param col The color of the circle (optional, default is the current draw color).
   * 
   * @note This function is affected by the camera settings. If the camera has been moved, the drawing will
   *       reflect those changes. 
   * 
   * @note  The depth (z-coordinate) of the line is determined by a prior call to setz(). 
   *        If no depth is set, the default value from the current state will be used.
   *
   */
  void circ(fx8 x, fx8 y, fx8 r = fx8(4), Color col = CURRENT);

  /**
   * @brief Draws a filled circle on the screen.
   * 
   * Draws a filled circle with the specified center coordinates (x, y) and radius r.
   * If the radius is omitted, the default radius is 4. 
   * If the color is omitted, the current drawing color is used.
   * 
   * @param x The x coordinate of the center of the circle.
   * @param y The y coordinate of the center of the circle.
   * @param r The radius of the circle (optional, default is 4).
   * @param col The color of the circle fill (optional, default is the current draw color).
   * 
   * @note This function is affected by the camera settings. If the camera has been moved, the drawing will 
   *       reflect those changes.
   * 
   * @note The depth (z-coordinate) of the filled circle is determined by a prior call to setz(). 
   *       If no depth is set, the default value from the current state will be used.
   */
  void circfill(fx8 x, fx8 y, fx8 r = fx8(4), Color col = CURRENT);

  /**
   * @brief Sets the clipping rectangle and returns the previous clipping rectangle.
   *
   * This function updates the current clipping rectangle using the specified position
   * and size, and then returns the previous clipping rectangle.
   * 
   * @param x The x-coordinate of the top-left corner of the new clipping rectangle.
   * @param y The y-coordinate of the top-left corner of the new clipping rectangle.
   * @param w The width of the new clipping rectangle.
   * @param h The height of the new clipping rectangle.
   * @return A reference to the previous clipping rectangle before the update.
   * 
   * @note This function is affected by the depth (z-coordinate) set by a prior call to setz().
   *       If no explicit depth is provided in drawing functions, the depth value set by setz()
   *       will be used and persist until changed by another call to setz().
   */
  const Rect& clip(fx8 x, fx8 y, fx8 w, fx8 h);

  /**
   * @brief Sets the clipping rectangle using the provided Rect structure and returns the previous clipping rectangle.
   *
   * This function updates the current clipping rectangle using the position and size specified
   * in the provided `Rect` structure. After setting the new clipping area, it returns the previous
   * clipping rectangle before the update.
   * 
   * @param rc A `Rect` structure containing the position (x, y) and size (w, h) of the new clipping rectangle.
   * @return A reference to the previous clipping rectangle before the update.
   * 
   * @note This function is affected by the depth (z-coordinate) set by a prior call to setz().
   *       If no explicit depth is provided in drawing functions, the depth value set by setz()
   *       will be used and persist until changed by another call to setz().
   */
  const Rect& clip(const Rect& rc);

  /**
   * @brief Resets the clipping rectangle to the entire screen and returns the previous clipping rectangle.
   *
   * This function resets the current clipping rectangle to cover the entire screen resolution, effectively
   * disabling clipping. After resetting the clipping area, it returns the previous clipping rectangle.
   * 
   * @return A reference to the previous clipping rectangle before the reset.
   * 
   * @note This function is affected by the depth (z-coordinate) set by a prior call to setz().
   *       If no explicit depth is provided in drawing functions, the depth value set by setz()
   *       will be used and persist until changed by another call to setz().
   */
  const Rect& clip();

  /**
   * @brief Represents the cursor state for sprite-based text rendering.
   *
   * This structure stores the current cursor position, color, and depth (Z-value)
   * for rendering text on the screen using sprites. It allows pixel-level control
   * over the rendering position and supports fine-grained color and depth management.
   *
   * @note This is specific to the sprite rendering layer and requires `sprint()`
   *       or similar functions to be called every frame to maintain visibility.
   *       Clearing the framebuffer (e.g., with `cls()`) will remove all sprite-based
   *       text, requiring re-rendering in the subsequent frame.
   */
  struct SprCursor {
    int x; ///< The x-coordinate in pixels (column position).
    int y; ///< The y-coordinate in pixels (row position).
    Color color; ///< The drawing color for text (default: CURRENT).
    int z; ///< The depth (Z-value) for rendering order (default: 0).

    /**
     * @brief Resets the cursor state to its default values.
     *
     * Sets the position to (0, 0), color to `CURRENT`, and depth to 0.
     */
    inline void Reset() {
        x = y = z = 0;
        color = CURRENT;
    }

    SprCursor(){ Reset(); }
  };

  /**
   * @brief Sets the cursor position, color, and depth (Z-value) for printing text using sprites.
   *
   * This function positions the sprite cursor at the specified (x, y) coordinates
   * for subsequent calls to `sprint()`. It allows precise pixel-level positioning
   * and supports color and depth settings for flexible text rendering.
   *
   * The cursor position set by this function is in absolute screen coordinates
   * and is not affected by `camera()` transformations.
   *
   * @param x The x-coordinate in pixels (default: 0).
   * @param y The y-coordinate in pixels (default: 0).
   * @param color The text color (default: CURRENT).
   * @param z The Z-value (depth) for rendering order (default: 0).
   *
   * @return The previous cursor state as a `SprCursor` struct.
   *
   * @note This is a unique extension for sprite rendering, not present in PICO-8.
   *       The `s` prefix stands for "Sprite". As sprites are part of the framebuffer,
   *       this function must be called every frame to maintain visibility.
   *
   * @see sprint()
   */
  const SprCursor& scursor(int x = 0, int y = 0, Color color = CURRENT, int z = 0);

  /**
   * @brief Prints formatted text using sprites for rendering.
   *
   * This function outputs text rendered as sprites, allowing for fine control over
   * position, color, and depth. Text is drawn on the sprite layer and cleared with
   * framebuffer-clearing functions (e.g., `cls()`).
   *
   * ### Example usage with BEEP-8 Color codes:
   * @code
   * sprint( "\e[50mRed text in BEEP-8 palette\e[0m\n" );  // Red text using BEEP-8 color 50
   * sprint( "\e[51mDark Blue text\e[0m\n" );  // Dark Blue text using BEEP-8 color 51
   * sprint( "\e[62;72mBlue text with Dark Blue background\e[0m\n" );  // Blue text with Dark Blue background
   * sprint( "\e[57;72mWhite/Purple\n" ); // Foreground: White, Background: Dark Purple
   * sprint( "\e[59;73mOrange/Green\n" ); // Foreground: Orange, Background: Dark Green
   * sprint( "\e[60;74mYellow/Brown\n" ); // Foreground: Yellow, Background: Brown
   * sprint( "\e[61;75mGreen/DarkGrey\n" ); // Foreground: Green, Background: Dark Grey
   * sprint( "\e[62;76mBlue/LightGrey\n" ); // Foreground: Blue, Background: Light Grey
   * sprint( "\e[63;77mLavender/White\n" ); // Foreground: Lavender, Background: White
   * sprint( "\e[64;78mPink/Red\n" ); // Foreground: Pink, Background: Red
   * sprint( "\e[65;79mPeach/Orange\n" ); // Foreground: Light Peach, Background: Orange
   * sprint( "\e[50;71mBlack/Blue\n" ); // Foreground: Black, Background: Dark Blue
   * sprint( "\e[51;70mDarkBlue/Black\n" ); // Foreground: Dark Blue, Background: Black
   * @endcode
   *
   * @param format The format string for text output (supports printf-style formatting).
   * @param ... Additional arguments for formatting.
   *
   * @note Unlike PICO-8's `print()`, this function requires explicit cursor and color
   *       settings through `scursor()`. Text rendering via sprites provides per-pixel
   *       precision and supports depth sorting.
   *
   * @note Since text drawn with `sprint()` is part of the framebuffer and cleared by `cls()`,
   *       it must be reissued every frame to remain visible. Excessive or frequent use of 
   *       `sprint()` may increase CPU load, so it is recommended to use it carefully,
   *       especially when rendering large amounts of dynamic text.
   *
   * @see scursor()
   */
  void sprint(const std::string_view format, ...);

  /**
   * @brief Prints formatted text at a specified position and color using sprites.
   *
   * Combines the functionality of `scursor()` and `sprint()`, allowing position
   * and color to be specified directly for a single call.
   *
   * @param x The x-coordinate in pixels.
   * @param y The y-coordinate in pixels.
   * @param color The text color.
   * @param format The format string for text output.
   * @param ... Additional arguments for formatting.
   *
   * ### Example usage:
   * @code
   * sprint(100, 200, RED, "Red text at pixel position (x=100, y=200)\n");
   * sprint(150, 250, BLUE, "Blue text at pixel position (x=150, y=250)\n");
   * sprint(300, 400, GREEN, "Green text at pixel position (x=300, y=400)\n");
   * @endcode
   *
   * @see scursor()
   * @see sprint()
   */
  void sprint(int x, int y, Color color, std::string_view format, ...);

  /**
   * @brief Represents the cursor state for background-based text rendering.
   *
   * This structure manages the cursor for rendering text on the background layer.
   * Background rendering operates in 8x8 TILE units and uses palette-based colors.
   *
   * @note Unlike sprites, background text persists across frames and does not
   *       require re-rendering unless explicitly cleared or overwritten.
   */
  struct BgCursor {
    int x; ///< The x-coordinate in 8x8 TILE units.
    int y; ///< The y-coordinate in 8x8 TILE units.
    BgPal pal; ///< The palette index for text rendering (default: BG_PAL_CURRENT).

    /**
     * @brief Resets the cursor state to its default values.
     *
     * Sets the position to (0, 0) and palette to `BG_PAL_CURRENT`.
     */
    inline void Reset() {
      x = y = 0;
      pal = BG_PAL_CURRENT;
    }
    BgCursor(){
      Reset();
    }
  };

  /**
   * @brief Sets the cursor position and palette for background-based text rendering.
   *
   * Positions the cursor in TILE units for rendering text on the background layer.
   * Supports selecting one of four available palettes (0, 1, 2, or 3).
   *
   * @param x The x-coordinate in TILE units (default: 0).
   * @param y The y-coordinate in TILE units (default: 0).
   * @param pal The palette index (default: BG_PAL_CURRENT).
   *
   * @return The previous cursor state as a `BgCursor` struct.
   *
   * @note This function is designed for background rendering, closely mimicking
   *       PICO-8's `cursor()`. Background text persists without requiring
   *       continuous updates.
   */
  const BgCursor& cursor(int x = 0, int y = 0, BgPal pal = BG_PAL_CURRENT);

  /**
   * @brief Prints formatted text on the background layer.
   *
   * Outputs text using the current `cursor()` position and palette. Text is rendered
   * in TILE units (8x8) and respects the current background settings.
   *
   * ### Example usage:
   * @code
   * // Print text
   * print("Hello, World!\n");
   *
   * // Move cursor to TILE position (2, 2)
   * print("\e[2;2H");
   *
   * // Select background palette (palette index 3)
   * print("\e[3q SelectPal");
   *
   * // Clear the entire screen
   * print("\e[2J");
   * @endcode
   *
   * @param format The format string for text output.
   * @param ... Additional arguments for formatting.
   *
   * @note Background text rendering uses palettes for color management. To change
   *       colors, use the `pal()`or `setpal()` function.
   *
   * @see cursor()
   * @see pal()
   * @see setpal()
   */
  void print(std::string_view format, ...);
  
  /**
   * @brief Prints formatted debug output to the screen in the foremost layer.
   *
   * This function behaves similarly to `printf`, allowing formatted text output
   * for debugging purposes. It accepts a format string followed by a variable
   * number of arguments.
   *
   * The debug text is always rendered in the topmost layer, ensuring that it remains
   * visible regardless of other graphics or UI elements.
   *
   * Debug output can be enabled or disabled globally by using `dprintenable()`.
   * When disabled, calls to `dprint()` have no effect.
   *
   * @param format The format string specifying how to format the output.
   * @param ... Additional arguments to be formatted according to the format string.
   */
  void dprint(std::string_view format, ...);

  /**
   * @brief Enables or disables debug text output generated by `dprint()`.
   *
   * This function globally controls whether debug text printed by `dprint()` appears on screen.
   * When disabled, all `dprint()` calls are ignored.
   *
   * By default, debug output is enabled at startup.
   *
   * @param enable Set to `true` to enable debug output, or `false` to disable it.
   *
   * @note Disabling debug output may slightly improve performance during runtime.
   */
  void dprintenable(bool enable);

  /**
   * @brief Prints formatted text at a specified position and palette on the background layer.
   *
   * Combines the functionality of `cursor()` and `print()`, allowing position and
   * palette to be specified directly for a single call.
   *
   * @param x The x-coordinate in TILE units.
   * @param y The y-coordinate in TILE units.
   * @param pal The palette index for rendering.
   * @param format The format string for text output.
   * @param ... Additional arguments for formatting.
   *
   * ### Example usage:
   * @code
   * print(5, 10, BG_PAL_1, "Background text at TILE position (5, 10)\n");
   * @endcode
   *
   * @see cursor()
   * @see print()
   */
  void print(int x, int y, BgPal pal, std::string_view format, ...);

  /**
   * @brief Sets attribute flags for a sprite or background (BG) pattern.
   *
   * Each sprite or BG pattern has an associated 8-bit attribute flag field, 
   * which can be freely customized using this function. 
   * These flags are commonly used to define properties such as collision types 
   * ("wall", "water", etc.) or other gameplay-related attributes for each pattern.
   *
   * Flags are numbered from 0 to 7. This function allows setting an individual flag 
   * or all flags at once by specifying a bit field.
   *
   * @param sprite_index The index of the sprite or BG pattern whose flag(s) will be set.
   * @param flag_index The flag index to set (0–7). If omitted or set to 0xff, 
   *                   all flags for the pattern will be set using the provided value as a bit field.
   * @param value The value to assign to the flag (1 to set, 0 to clear).
   *              If `flag_index` is omitted, this value is treated as a complete 8-bit flag field.
   * @param sprite_pattern_bank The sprite pattern bank to operate on. BEEP-8 supports multiple 
   *                            banks (up to 16), whereas PICO-8 supports only one. 
   *                            Defaults to 0 (the first sprite bank).
   *
   * @note Unlike PICO-8, BEEP-8 allows selecting different sprite pattern banks, enabling more flexible asset organization.
   *
   * Example usage:
   * @code
   * fset(10, 1, 1);    // Set flag 1 of sprite/BG pattern 10 to 1 (true)
   * fset(5, 0xff, 7);  // Set all flags of sprite/BG pattern 5 to the bit field value 7
   * @endcode
   */
  void fset(u8 sprite_index, u8 flag_index = 0xff, u8 value = 0, u8 sprite_pattern_bank = 0);

  /**
   * @brief Retrieves attribute flags for a sprite or background (BG) pattern.
   *
   * Each sprite or BG pattern has an associated 8-bit attribute flag field,
   * which can be used to define gameplay-related properties such as collision
   * types ("wall", "water", etc.). This function retrieves either the value
   * of a specific flag or the entire flag bit field for a given pattern.
   *
   * Flags are numbered from 0 to 7, with flag 0 representing the least significant bit.
   *
   * @param sprite_index The index of the sprite or BG pattern whose flag(s) will be retrieved.
   * @param flag_index The flag index to retrieve (0–7). If omitted or set to 0xff,
   *                   the entire 8-bit flag field will be returned.
   * @param sprite_pattern_bank The sprite pattern bank to operate on. BEEP-8 supports multiple
   *                            sprite banks (up to 16), whereas PICO-8 only has one.
   *                            Defaults to 0 (the first sprite bank).
   * @return The value of the specified flag (0 or 1), or the full 8-bit flag field
   *         if no flag index is provided.
   *
   * @note Unlike PICO-8, BEEP-8 allows selecting different sprite pattern banks,
   *       enabling more flexible asset management.
   *
   * Example usage:
   * @code
   * u8 flag = fget(10, 1);  // Get the value of flag 1 of sprite/BG pattern 10
   * u8 flags = fget(5);     // Get the full bit field of flags for sprite/BG pattern 5
   * @endcode
   */
  u8 fget(u8 sprite_index, u8 flag_index = 0xff, u8 sprite_pattern_bank = 0);


  enum  BgIndex{ BG_0, BG_1, BG_2, BG_3 , BG_MAX };
  enum  BgTiles{
    TILES_8   = 1<<3,
    TILES_16  = 1<<4,
    TILES_32  = 1<<5,
    TILES_64  = 1<<6,
    TILES_128 = 1<<7,
    TILES_256 = 1<<8,
    TILES_512 = 1<<9,
    TILES_1024= 1<<10,
    TILES_2048= 1<<11,
  };
  using BgTilesPtr = std::shared_ptr<std::vector<b8PpuBgTile>>;

  /**
   * @brief Configures a background layer for the PPU.
   *
   * This function sets up the background layer configuration, including the width and height
   * in tiles, the tile data, and wrapping behavior for the specified background index. 
   * 
   * This configuration is mandatory before using the background map for rendering.
   * It is possible to reconfigure the background multiple times, but modifications are 
   * prohibited during the drawing phase.
   * 
   * If the `tiles` argument is not provided, a new vector of `b8PpuBgTile` will be created with 
   * a size matching the specified width (`wtile`) and height (`htile`).
   *
   * @param wtile The width of the background in tiles (must be a power of 2).
   * @param htile The height of the background in tiles (must be a power of 2).
   * @param tiles An optional shared pointer to a vector of `b8PpuBgTile` tiles. 
   *              If not provided, a new tile vector is allocated.
   * @param uwrap Specifies the horizontal wrapping behavior. The possible values are:
   *              - `B8_PPU_BG_WRAP_CLAMP` (default): No wrapping.
   *              - `B8_PPU_BG_WRAP_CLAMP_TO_EDGE`: Clamp to edge wrapping.
   *              - `B8_PPU_BG_WRAP_REPEAT`: Repeats the background tiles.
   *              These constants are defined in `ppu.h`.
   * @param vwrap Specifies the vertical wrapping behavior. It follows the same three options as `uwrap`.
   * @param index The background index to configure (from 0 to BG_MAX-1). If omitted, `BG_0` is used as the default.
   */
  void  mapsetup(BgTiles wtile, BgTiles htile, std::optional<BgTilesPtr> tiles = std::nullopt , u8 uwrap = B8_PPU_BG_WRAP_CLAMP, u8 vwrap = B8_PPU_BG_WRAP_CLAMP , BgIndex index = BG_0 );

  /**
   * @brief Draws the configured background layer at the specified pixel offset.
   *
   * This function renders a background layer at the given horizontal (`upix`) and vertical (`vpix`) 
   * pixel offsets. The background index specifies which configured background to draw.
   * 
   * The `setz()` setting is respected, and the background will be drawn at the specified depth.
   * However, note that `camera()` settings do not affect the rendering of this background layer,
   * meaning the background will always be drawn without camera transformations.
   * 
   * Unlike PICO-8's `map()` function, this function always renders the entire screen area as the background,
   * covering the full display with the configured background tiles.
   *
   * @param upix The horizontal pixel offset.
   * @param vpix The vertical pixel offset.
   * @param index The background index to draw (from 0 to BG_MAX-1). If omitted, `BG_0` is used as the default.
   * 
   * @note Before using this function, ensure that `mapsetup()` has been called to configure the 
   * background layer. Failure to do so will result in undefined behavior.
   */
  void  map(s16 upix,s16 vpix, BgIndex index = BG_0 );

  /**
   * @brief Alias for `map()` function to draw the configured background layer at a specified pixel offset.
   *
   * @see map()
   */
  inline  void  mapdraw(s16 upix,s16 vpix, BgIndex index = BG_0 ){
    pico8::map(upix,vpix,index);
  }

  /**
   * @brief Retrieves the full tile information at a specific position in the background map.
   *
   * This function fetches the complete `b8PpuBgTile` structure at the given (x, y) coordinates
   * within the specified background index. If the coordinates are out of bounds, a default
   * "zero" tile is returned.
   *
   * The returned `b8PpuBgTile` contains detailed attributes, including the tile ID components 
   * (XTILE, YTILE) and palette selection information.
   *
   * @param x The x-coordinate of the tile in the background map.
   * @param y The y-coordinate of the tile in the background map.
   * @param index The background index from which to retrieve the tile (from 0 to BG_MAX-1).
   * @return The `b8PpuBgTile` at the specified position, or a default tile if out of bounds.
   *
   * @see mget(u32, u32, BgIndex) — If you only need the tile ID (as a single u16 value), use mget().
   */
  b8PpuBgTile mgett(u32 x, u32 y, BgIndex index = BG_0);

  /**
   * @brief Retrieves the tile ID at a specific position in the background map.
   *
   * This function fetches the tile at the given (x, y) coordinates within the specified background index
   * and returns its tile ID, computed as `YTILE * 16 + XTILE`.
   * 
   * If the coordinates are out of bounds, the `mgett` function internally returns a default "zero" tile,
   * and this function returns the corresponding ID for that tile.
   *
   * This function is suitable when you only need to know which pattern (tile ID) is placed at a given location,
   * without requiring full tile attribute information.
   *
   * @param x The x-coordinate of the tile in the background map.
   * @param y The y-coordinate of the tile in the background map.
   * @param index The background index from which to retrieve the tile (default is `BG_0`).
   * @return The ID of the tile at the specified position, computed as `YTILE * 16 + XTILE`.
   *
   * @see mgett(u32, u32, BgIndex) — If you need full tile attribute details, use mgett().
   */
  u16 mget(u32 x, u32 y, BgIndex index = BG_0);

  /**
   * @brief Sets a tile on the background map with an extended `bank` parameter for flexible tile selection.
   *
   * This function sets the tile at the specified x and y coordinates in the background map for a
   * specified background index. In addition to the PICO-8 `mset()` functionality, this version
   * includes a `bank` parameter, which allows accessing additional tile pages.
   *
   * This function is useful for placing or modifying objects on the map, generating levels procedurally,
   * or managing complex backgrounds by storing sprites across multiple banks.
   *
   * @param x The column (x) coordinate of the cell in the background map.
   * @param y The row (y) coordinate of the cell in the background map.
   * @param v The sprite number to set at the specified location, using the same specification as PICO-8's `mset()`.
   *          This number determines the sprite ID to be set on the background map.
   * @param bank The bank number for additional tiles (default is 0). `bank` enables selection of additional
   *             tile sets by grouping tiles into pages, with `bank` shifting tile coordinates as follows:
   *             - `XTILE`: `(bank & 3) << 4` + lower 4 bits of `v`
   *             - `YTILE`: `(bank >> 2) << 4` + upper 4 bits of `v`
   * @param index The background index to update (default is `BG_0`).
   * @param pal The palette index for the tile (default is 0). This parameter allows selecting the color palette 
   *            to be applied to the tile.
   *
   * @note This function is ideal for simpler use cases where each tile is set primarily by its sprite ID, 
   *       without needing detailed control over the palette or flip settings.
   *       For advanced tile configurations where individual tile attributes, such as palette selection,
   *       horizontal and vertical flipping, and specific tile coordinates, are required, consider using
   *       the `mset(u32 x, u32 y, b8PpuBgTile tile, BgIndex index = BG_0);` function instead.
   *       This variant allows precise control over each tile's appearance.
   *
   * @see mset(u32 x, u32 y, b8PpuBgTile tile, BgIndex index = BG_0) for configurations with detailed attributes.
   */
  void mset(u32 x, u32 y, u8 v, u8 bank = 0, BgIndex index = BG_0, uint8_t pal = 0);

  /**
   * @brief Sets a detailed tile configuration at a specific position in the background map.
   *
   * This function sets a tile with detailed attributes (palette, flipping, and specific tile coordinates) 
   * at the given x and y coordinates within the background map of the specified background index.
   * This function is intended for advanced tile configuration, allowing control over individual tile appearance.
   * If the coordinates are out of bounds, the function does nothing.
   *
   * @param x The x-coordinate of the tile in the background map.
   * @param y The y-coordinate of the tile in the background map.
   * @param tile The `b8PpuBgTile` structure representing the tile to set at the specified position. This structure 
   *             includes the following fields:
   *             - `XTILE`: The X-coordinate of the tile in the background map (in tiles).
   *             - `YTILE`: The Y-coordinate of the tile in the background map (in tiles).
   *             - `VFP`: A flag indicating if the tile should be vertically flipped.
   *             - `HFP`: A flag indicating if the tile should be horizontally flipped.
   *             - `PAL`: The palette selection for the tile (2-bit value).
   * @param index The background index to update (from 0 to BG_MAX-1). If omitted, `BG_0` is used as the default.
   *
   * @note This function is ideal when precise control over tile properties is required, such as individual palette selection
   *       and flipping, or when specific tiles need unique visual configurations.
   *       For simpler cases where only the sprite ID is required, without additional attributes, 
   *       consider using `mset(u32 x, u32 y, u8 v, u8 bank = 0, BgIndex index = BG_0);`.
   *
   * @see mset(u32 x, u32 y, u8 v, u8 bank = 0, BgIndex index = BG_0) for a simplified tile setting function with a bank parameter.
   */
  void  msett(u32 x,u32 y,b8PpuBgTile tile,BgIndex index = BG_0 );

  /**
   * @brief Clears the entire background map to a specific tile.
   *
   * This function fills the entire background map for the specified background index 
   * with the given tile. If the `tile` argument is omitted, it defaults to a tile with 
   * no flip, palette 0, and top-left corner (XTILE=0, YTILE=0).
   *
   * @param tile The `b8PpuBgTile` to fill the background with. Defaults to a tile with
   *             HFP=0, VFP=0, PAL=0, XTILE=0, YTILE=0 if omitted.
   * @param index The background index to clear (from 0 to BG_MAX-1).
   */
  void mcls(b8PpuBgTile tile = b8PpuBgTile{0, 0, 0, 0, 0}, BgIndex index = BG_0);

  /**
   * Checks the state of a specific button for a given player or returns the state of all buttons.
   * 
   * @param button The button to check (e.g., BUTTON_LEFT, BUTTON_RIGHT, etc.). Use `btn()` without specifying a button
   *               to check if any button is pressed.
   * @param player The player number (default is 0). Only player 0 is currently supported. If player >= 1, the function returns false.
   * @return If a specific button is specified (e.g., BUTTON_LEFT), returns true if the specified button is pressed; otherwise, false.
   *         If no specific button is specified, returns a 32-bit integer where each bit represents the state of each button:
   *         - Bit 0: BUTTON_LEFT
   *         - Bit 1: BUTTON_RIGHT
   *         - Bit 2: BUTTON_UP
   *         - Bit 3: BUTTON_DOWN
   *         - Bit 4: BUTTON_O
   *         - Bit 5: BUTTON_X
   * 
   * @note This implementation currently supports only player 0. For player 1 and above, this function will return false.
   * 
   * @code
   * // Check individual buttons
   * if (btn(BUTTON_LEFT))  printf("LEFT\n");
   * if (btn(BUTTON_RIGHT)) printf("RIGHT\n");
   * if (btn(BUTTON_UP))    printf("UP\n");
   * if (btn(BUTTON_DOWN))  printf("DOWN\n");
   * if (btn(BUTTON_O))     printf("O\n");
   * if (btn(BUTTON_X))     printf("X\n");
   *
   * // Check if any button is pressed
   * if (btn()) printf("A button is pressed\n");
   *
   * // Check multiple buttons with bitwise operation
   * if (btn() & (1 << BUTTON_LEFT)) printf("LEFT is pressed\n");
   * if (btn() & (1 << BUTTON_RIGHT)) printf("RIGHT is pressed\n");
   * @endcode
   */
  u32 btn(Button button=BUTTON_ANY, u8 player = 0);

  /**
   * Checks if a button has just been pressed for a given player.
   * Similar to `btn`, but returns true only if the button was pressed
   * on this frame and was not pressed in the previous frame. Additionally,
   * if the button remains pressed, the function will return true every 4 frames
   * after the initial 15 frames, matching the original PICO-8 behavior.
   * 
   * @param button The button to check (e.g., BUTTON_LEFT, BUTTON_RIGHT, etc.).
   *               If `BUTTON_ANY` is passed, the function checks all buttons
   *               (including `BUTTON_MOUSE_LEFT`) and returns true if any of them
   *               meet the `btnp` criteria.
   * @param player The player number (default is 0). Only player 0 is currently supported.
   *               If `player >= 1`, the function returns false.
   * @return True if the specified button has just been pressed or if it remains pressed
   *         under the repeating interval; otherwise, false.
   * 
   * @note The implementation currently supports only player 0. For player 1 and above,
   *       this function will return false.
   * @note When `BUTTON_ANY` is used as the button parameter, all standard buttons and
   *       `BUTTON_MOUSE_LEFT` are checked.
   * @attention `BUTTON_MOUSE_LEFT` corresponds to both mouse left-clicks and touch taps,
   *            providing a unified input mechanism for mouse and touch devices.
   */
  bool btnp(Button button, u8 player=0);

  /**
   * Returns the number of frames since a specific button was released for a given player.
   * While the button is still pressed, this function returns 0. Once the button is released,
   * it returns the number of frames since the release moment, incrementing each frame.
   * A return value of 1 indicates the exact frame the button was released.
   * 
   * @param button The button to check (e.g., BUTTON_LEFT, BUTTON_RIGHT, etc.).
   *               If `BUTTON_MOUSE_LEFT` is specified, the function also tracks touch taps 
   *               in addition to mouse left-clicks, providing unified input tracking across 
   *               mouse and touch devices.
   * @param player The player number (default is 0). Only player 0 is currently supported.
   *               If `player >= 1`, the function always returns 0.
   * @return The number of frames since the specified button was released. 
   *         Returns 0 if the button is currently pressed or if `player >= 1`.
   * 
   * @note This function is unique to this implementation and is not part of the original PICO-8 API.
   * @note For `BUTTON_ANY`, this function is not applicable and will always return 0.
   * @attention `BUTTON_MOUSE_LEFT` corresponds to both mouse left-clicks and touch taps,
   *            ensuring compatibility across input devices.
   */
  u32 btnr(Button button, u8 player=0);

  /**
   * Retrieves specific system information based on the provided index, 
   * primarily for PICO-8 compatibility. While PICO-8 only supports integer 
   * coordinates for mouse positions, BEEP-8 extends functionality by providing
   * sub-pixel precision using `fx8` (8-bit fixed-point format).
   * 
   * - `stat(32)`: Returns the current mouse X position in integer pixels for 
   *               PICO-8 compatibility. Use `mousex()` in BEEP-8 for 
   *               sub-pixel precision.
   * - `stat(33)`: Returns the current mouse Y position in integer pixels for 
   *               PICO-8 compatibility. Use `mousey()` in BEEP-8 for 
   *               sub-pixel precision.
   * - `stat(34)`: Returns 1 if the left mouse button is pressed. Use 
   *               `mousestatus()` for full mouse button status in BEEP-8.
   * 
   * @param index The index of the system information to retrieve. Use 32, 33, or 34 
   *              only for legacy PICO-8 compatibility. BEEP-8 provides clearer 
   *              and more precise alternatives: `mousex()`, `mousey()`, and 
   *              `mousestatus()`.
   * 
   * @return The requested system information value. For button states, refer to 
   *         the `MouseBtn` enumeration:
   *         - `MouseBtn::LEFT` for left click
   * 
   * @note Mouse coordinates retrieved using `stat(32)` and `stat(33)` in BEEP-8 
   *       are integers for PICO-8 compatibility. For sub-pixel precision, use 
   *       `mousex()` and `mousey()`, which return values in `fx8` format.
   * 
   * @warning The use of hard-coded indices (32, 33, 34) is discouraged and is 
   *          retained solely for PICO-8 compatibility. It is strongly recommended 
   *          to use `mousex()`, `mousey()`, and `mousestatus()` for future-proof 
   *          and precise implementations in BEEP-8.
   */
  s32 stat(int index);

  /**
   * @brief Retrieves the current X position of the mouse or touch input.
   *
   * On PC environments, this function returns the current mouse X position.
   * On smartphone or tablet environments, it returns the X coordinate of the active touch input.
   *
   * The returned value is a fixed-point 8-bit fractional number (fx8), 
   * ranging from 0 to 127 (corresponding to the 128-pixel screen width).
   *
   * @return The current X position as a fixed-point 8-bit fractional value.
   */
  fx8 mousex();

  /**
   * @brief Retrieves the current Y position of the mouse or touch input.
   *
   * On PC environments, this function returns the current mouse Y position.
   * On smartphone or tablet environments, it returns the Y coordinate of the active touch input.
   *
   * The returned value is a fixed-point 8-bit fractional number (fx8), 
   * ranging from 0 to 240 (corresponding to the 240-pixel screen height).
   *
   * @return The current Y position as a fixed-point 8-bit fractional value.
   */
  fx8 mousey();

  /**
   * @brief Retrieves the current status of the mouse buttons.
   *
   * The status is represented as a bitmask using the `MouseBtn` enumeration,
   * where each bit indicates the state of a specific mouse button:
   * - `MouseBtn::LEFT` for the left button
   *
   * Example usage:
   * @code
   * if (mousestatus() & MouseBtn::LEFT) {
   *     // The left mouse button is pressed
   * }
   * @endcode
   *
   * @return The current button status as a bitmask.
   *
   * @note Although mouse button input is supported, it is generally not recommended in BEEP-8,
   *       as the platform primarily targets smartphones and tablets where touch input is standard.
   */
  u32 mousestatus();

  /**
   * @brief Calculates the cosine of an angle in radians.
   *
   * This function returns the cosine of an angle specified in radians. Unlike PICO-8's implementation,
   * which uses an angle range of [0.0, 1.0] for a full circle, this function uses the standard
   * mathematical convention of a full circle being represented by 2π radians.
   *
   * @param rad The angle in radians, where 2π represents a full circle.
   * @return The cosine of the specified angle.
   * 
   * @note To avoid accidentally calling the standard libc `cos()` function, it is recommended to
   *       use `pico8::cos()` explicitly when calling this function.
   */
  fx8 cos(fx8 rad);

  /**
   * @brief Calculates the sine of an angle in radians.
   *
   * This function returns the sine of an angle specified in radians. Unlike PICO-8's implementation,
   * which uses an angle range of [0.0, 1.0] for a full circle, this function uses the standard
   * mathematical convention of a full circle being represented by 2π radians.
   *
   * @param rad The angle in radians, where 2π represents a full circle.
   * @return The sine of the specified angle.
   * 
   * @note To avoid accidentally calling the standard libc `sin()` function, it is recommended to
   *       use `pico8::sin()` explicitly when calling this function.
   */
  fx8 sin(fx8 rad);

  /**
   * @brief Calculates the arctangent of y/x, returning the angle in radians.
   *
   * This function calculates the arctangent of `y/x`, representing the angle between the positive x-axis
   * and the point `(x, y)` on the Cartesian plane. Unlike PICO-8, which uses a range of [0.0, 1.0] for 
   * a full circle, this function follows the standard mathematical convention, using radians where a full 
   * circle is represented by 2π. Additionally, note that the argument order here is `(y, x)`, which follows 
   * the standard library convention but differs from PICO-8's `(dx, dy)` order.
   *
   * **Corner Cases**:
   * - **(x, y) = (0, 0)**: Returns `0` as the angle is undefined at the origin. This choice avoids 
   *   potential errors without causing an assertion.
   * - **x = 0 and y > 0**: Returns `π/2` radians (90 degrees), representing the positive y-axis.
   * - **x = 0 and y < 0**: Returns `-π/2` radians (-90 degrees), representing the negative y-axis.
   * - **x < 0**: The angle returned will be in the range `[π, -π]`, adjusted by ±π to account for the 
   *   correct quadrant.
   *
   * @param y The vertical component (y-coordinate).
   * @param x The horizontal component (x-coordinate).
   * @return The angle in radians from the positive x-axis to the point `(x, y)`, measured counterclockwise.
   *
   * @note To avoid accidentally calling the standard libc `atan2()` function, it is recommended to
   *       use `pico8::atan2()` explicitly when calling this function.
   */
  fx8 atan2(fx8 y, fx8 x);
  
  /**
   * @brief Returns the absolute value of the specified fixed-point number.
   *
   * This function computes and returns the absolute (non-negative) value of the given fixed-point 
   * number `x`, effectively removing any negative sign. If `x` is already non-negative, it is returned 
   * unchanged.
   *
   * @param x The fixed-point number to compute the absolute value of.
   * @return The absolute value of `x` as a fixed-point number.
   *
   * @note To avoid calling a standard library `abs()` function by mistake, it is recommended to use
   *       `pico8::abs()` explicitly if the function is defined within the `pico8` namespace.
   */
  fx8 abs(fx8 x);

  /**
   * @brief Returns the largest integer less than or equal to the specified fixed-point number.
   *
   * This function computes and returns the floor value of the given fixed-point number `x`, which is 
   * the largest integer value that is less than or equal to `x`. For positive values, this behaves 
   * as a simple truncation, but for negative values, it rounds down to the next lower integer.
   *
   * @param x The fixed-point number to compute the floor value of.
   * @return The largest integer less than or equal to `x`, as a fixed-point number.
   *
   * @note To avoid calling the standard library `floor()` function by mistake, it is recommended to use
   *       `pico8::flr()` explicitly if the function is defined within the `pico8` namespace.
   */
  fx8 flr(fx8 x);

  /**
   * @brief Returns the smallest integer greater than or equal to the specified fixed-point number.
   *
   * This function calculates the ceiling of the given fixed-point number `x`, which is the smallest 
   * integer value that is greater than or equal to `x`. If `x` already represents an integer or is 
   * negative, it remains unchanged. Otherwise, `x` is rounded up to the next higher integer.
   *
   * @param x The fixed-point number to compute the ceiling value of.
   * @return The smallest integer greater than or equal to `x`, as a fixed-point number.
   *
   * @note This function is named `cel` following PICO-8's convention of using three-letter function names.
   */
  fx8 cel(fx8 x);

  /**
   * @brief Returns the greater of two fixed-point numbers.
   *
   * This function compares two fixed-point numbers, `x` and `y`, and returns the larger of the two.
   * If `x` is greater than or equal to `y`, `x` is returned; otherwise, `y` is returned.
   *
   * @param x The first fixed-point number to compare.
   * @param y The second fixed-point number to compare.
   * @return The greater of `x` and `y`, as a fixed-point number.
   *
   * @note To avoid calling a standard library `max()` function by mistake, it is recommended to use
   *       `pico8::max()` explicitly if the function is defined within the `pico8` namespace.
   */
  fx8 max(fx8 x, fx8 y);

  /**
   * @brief Returns the lesser of two fixed-point numbers.
   *
   * This function compares two fixed-point numbers, `x` and `y`, and returns the smaller of the two.
   * If `x` is less than or equal to `y`, `x` is returned; otherwise, `y` is returned.
   *
   * @param x The first fixed-point number to compare.
   * @param y The second fixed-point number to compare.
   * @return The lesser of `x` and `y`, as a fixed-point number.
   *
   * @note To avoid calling a standard library `min()` function by mistake, it is recommended to use
   *       `pico8::min()` explicitly if the function is defined within the `pico8` namespace.
   */
  fx8 min(fx8 x, fx8 y);

  /**
   * @brief Returns the middle (median) value of three fixed-point numbers.
   *
   * This function takes three fixed-point numbers, `first`, `second`, and `third`, and returns 
   * the middle (or median) value among them. It works by identifying the minimum and maximum 
   * values among the three inputs and returning the remaining value, effectively excluding 
   * the smallest and largest values to provide the median. This behavior is particularly useful 
   * for clamping a value within a specific range.
   *
   * **Clamping Behavior**:
   * By calling this function with arguments as `mid(min_val, value, max_val)`, the function 
   * acts as a clamp, ensuring that `value` stays within the bounds of `min_val` and `max_val`. 
   * The result will be `min_val` if `value` is below the range, `max_val` if it exceeds the range, 
   * or `value` itself if it is within the specified range.
   *
   * @param first The first fixed-point number.
   * @param second The second fixed-point number.
   * @param third The third fixed-point number.
   * @return The middle value among `first`, `second`, and `third`.
   */
  fx8 mid( fx8 first, fx8 second, fx8 third );

  /**
   * @brief Alias for `mid` to provide clamping functionality.
   *
   * This alias enables the use of `mid` as a `clamp` function, allowing the function 
   * to be used with arguments in the form of `clamp(min_val, value, max_val)`. 
   * This usage ensures that `value` stays within the range specified by `min_val` 
   * and `max_val`, effectively clamping `value` to the range limits if it falls outside.
   */
  inline  fx8 clamp( fx8 first, fx8 second, fx8 third ){
    return pico8::mid(first,second,third);
  }

  /**
   * @brief Returns the sign of the specified fixed-point number.
   *
   * This function returns the sign of the fixed-point number `x` as either `1` or `-1`. 
   * If `x` is positive, it returns `1`; if `x` is negative, it returns `-1`. In accordance 
   * with the PICO-8 specification, if `x` is `0`, this function also returns `1`.
   *
   * @param x The fixed-point number for which to determine the sign.
   * @return `1` if `x` is positive or `0`, and `-1` if `x` is negative.
   */
  fx8 sgn(fx8 x);

  /**
   * @brief Computes the square root of a fixed-point number.
   *
   * This function returns the square root of the given fixed-point number `x`. 
   * If `x` is negative, it returns `0` instead of producing an error or `NaN`, 
   * as per the PICO-8 specification. This differs from the behavior of the 
   * standard libc `sqrt()` function, which would return `NaN` or set an error 
   * flag when passed a negative value.
   *
   * @param x The fixed-point number for which to compute the square root.
   * @return The square root of `x` if `x` is non-negative; `0` if `x` is negative.
   *
   * @note This function's behavior is intended to be compatible with PICO-8, 
   *       where negative inputs to `sqrt()` do not cause an error but instead return `0`.
   */
  fx8 sqrt(fx8 x);

  /**
   * @brief Generates a random fixed-point number in the range [0, x).
   *
   * This function returns a random fixed-point number `n` in the range `0 <= n < x`. The upper bound `x`
   * is not inclusive, meaning the returned value will never reach the value of `x`. For example, `rnd(fx8(10))`
   * could return values from `0.0` up to `9.99999`, but never exactly `10.0`.
   *
   * The generated random number includes a fractional part. 
   * If an integer is needed, use flr(rnd(x)) or simply call rndi(x) for an integer result directly.
   *
   * @param x The upper bound for the random number generation. If `x <= 0`, the function will return `0`.
   *          If no limit is provided, it defaults to `1.0`, and values are generated in the range `[0.0, 1.0)`.
   *
   * @return A random fixed-point number between `0` and `x`, exclusive.
   *
   * @note The random number generator is initialized with an unpredictable seed value at the start of the program.
   *       To produce a consistent sequence of random numbers for testing, use `srand()` with an explicit seed 
   *       before calling `rnd()`.
   */
  fx8 rnd(fx8 x = fx8(1.0));

  /**
   * @brief Generates a 32-bit unsigned random number.
   *
   * This function returns a uniformly distributed random number
   * in the range [0, 0xFFFFFFFF].
   *
   * @return A 32-bit unsigned integer representing the generated random number.
   */
  u32 rndu();

  /**
   * @brief Generates a random integer in the range [0, x) as a fixed-point number.
   *
   * This function returns a random integer `n` in the range `0 <= n < x`, where `x` is an `fx8` fixed-point
   * number. The integer part of `x` is used as the upper limit for the random value, meaning that `n` 
   * will always be less than the integer part of `x`. The result is returned as an `fx8` fixed-point 
   * number with no fractional part.
   *
   * For example, calling `rndi(fx8(10.5))` could return an `fx8` value in the range `[0, 10)`.
   *
   * @param x The upper limit for the random number generation. If `x <= 0`, the function returns `fx8(0)`.
   * @return A random integer as an `fx8` fixed-point number, within the range `[0, x)`.
   *
   * @note The result includes no fractional part. The random number generator should be seeded using `srand()` 
   *       if repeatability is desired, allowing the function to return a predictable sequence of random values.
   */
  fx8 rndi(fx8 x);

  /**
   * @brief Generates a random fixed-point number in the range [x0, x1].
   *
   * This function returns a random `fx8` fixed-point number `n` in the inclusive range `x0 <= n <= x1`.
   * The result includes fractional parts, unlike `rndi`, which returns an integer value. If `x0` is greater
   * than `x1`, the values are automatically swapped to ensure the correct range.
   *
   * For example, calling `rndf(fx8(1.5), fx8(5.75))` will return a random `fx8` value between 1.5 and 5.75, inclusive.
   *
   * @param x0 The lower bound of the random number range.
   * @param x1 The upper bound of the random number range.
   * @return A random `fx8` fixed-point number in the range [x0, x1].
   *
   * @note The random number generator should be seeded using `srand()` if a consistent sequence of values
   *       is desired for testing.
   */
  fx8 rndf(fx8 x0, fx8 x1);

  /**
   * @brief Generates a random index and returns a reference to a randomly selected element in the given span.
   *
   * This function selects a random element from a given `std::span` of elements.
   * If the span is empty, it returns a static dummy value of type `T`.
   *
   * @tparam T The type of the elements in the span.
   * @param sp A span representing a contiguous sequence of elements.
   * @return const T& A reference to the randomly chosen element, or a dummy value if empty.
   *
   * @note This function does not modify the original data.
   *
   * @usage
   * ### Example 1: C++ Fixed-Size Array (C-style array)
   * ```
   * static const int values[] = {10, 20, 30, 40, 50};
   * const int& random_value = rndt(values);
   * ```
   *
   * ### Example 2: std::vector
   * ```
   * std::vector<int> vec = {10, 20, 30, 40, 50};
   * const int& random_vec_value = rndt(vec);
   * ```
   *
   * ### Example 3: std::to_array (C++20)
   * ```
   * auto arr = std::to_array({10, 20, 30, 40, 50});
   * const int& random_arr_value = rndt(arr);
   * ```
   *
   * ### Example 4: std::array
   * ```
   * std::array<int, 5> std_arr = {10, 20, 30, 40, 50};
   * const int& random_std_arr_value = rndt(std_arr);
   * ```
   *
   * ### Example 5: std::span
   * ```
   * std::span<const int> span(values);
   * const int& random_span_value = rndt(span);
   * ```
   *
   * ### Example 6: User-Defined Class Array
   * ```
   * struct SomeClass {
   *     int id;
   *     std::string name;
   * };
   * static const SomeClass objects[] = {
   *     {1, "Alice"}, {2, "Bob"}, {3, "Charlie"}
   * };
   * const SomeClass& random_object = rndt(objects);
   * ```
   */
  template <typename T>
  const T& rndt(std::span<const T> sp) {
      static const T dummy_value{};
      if (sp.empty()) return dummy_value;
      std::size_t index = qmod( std::rand() , sp.size() );
      return sp[index];
  }

  /**
   * @brief Overload of `rndt` to accept a C-style array directly.
   *
   * This function allows passing a C-style array directly to `rndt`,
   * automatically converting it to `std::span`.
   *
   * @tparam T The type of the elements in the array.
   * @tparam N The number of elements in the array.
   * @param arr A reference to a C-style array.
   * @return const T& A reference to the randomly chosen element.
   *
   * @usage
   * ```
   * static const int values[] = {10, 20, 30, 40, 50};
   * const int& random_value = rndt(values);
   * ```
   */
  template <typename T, std::size_t N>
  const T& rndt(const T (&arr)[N]) {
      return rndt(std::span<const T>(arr));
  }

  /**
   * @brief Initializes the random number generator with a specified seed.
   *
   * This function sets the initial seed value for the random number generator using `init_genrand`.
   * By setting a specific seed, you can produce a predictable sequence of random numbers, which is 
   * useful for debugging or when repeatability is desired.
   *
   * @param seed The seed value to initialize the random number generator.
   *
   * @note The initialization process includes multiple iterations to ensure high-quality randomness.
   *       Therefore, it may be slightly computationally intensive. It is recommended to call `srand`
   *       once at the start of the program rather than repeatedly.
   */
  void  srand(u32 seed );

  /**
   * @brief Retrieves the screen resolution width in pixels.
   * 
   * This function returns the width of the screen resolution in pixels
   * for the current environment.
   * 
   * @return fx8 The screen width in pixels.
   */
  fx8 resw();

  /**
   * @brief Retrieves the screen resolution height in pixels.
   * 
   * This function returns the height of the screen resolution in pixels
   * for the current environment.
   * 
   * @return fx8 The screen height in pixels.
   */
  fx8 resh();

  /**
   * @brief Retrieves the color value of a specific pixel from a sprite sheet.
   *
   * This function fetches the color of the pixel located at coordinates `(x, y)`
   * in the sprite sheet specified by the `bank`. The sprite sheets are indexed
   * by the `bank` parameter. If the specified sprite sheet bank is not available,
   * the function will return `BLACK`.
   *
   * @param x The x-coordinate of the pixel to retrieve. Must be within the bounds of the sprite sheet (0-127).
   * @param y The y-coordinate of the pixel to retrieve. Must be within the bounds of the sprite sheet (0-127).
   * @param bank The index of the sprite sheet bank to use. Defaults to 0 if not specified. Valid values for
   *             `bank` range from 0 to 13, as the system supports up to 14 sprite sheet banks.
   * @return The color of the pixel at the specified coordinates as a `Color` enum value.
   *         Returns `BLACK` if the specified bank is not initialized.
   * 
   * @note An assertion failure will occur if the `bank` parameter is greater than 13,
   *       as only 14 sprite sheet banks are supported in the current implementation.
   */
  Color sget(u8 x, u8 y , u8 bank = 0 );

  class ImplPico8;

  /**
   * @brief Base class for creating PICO-8 compatible applications in C/C++.
   *
   * In the original PICO-8 environment, `init()`, `update()`, and `draw()` are reserved functions
   * that are used to initialize, update, and render the game, respectively. This class provides
   * a C/C++ compatible API for users to implement similar functionality by creating a derived class.
   *
   * Users are expected to create a subclass of `Pico8` and override the following virtual functions:
   * 
   * - `_init()`: Called once at the beginning, for any initialization code.
   * - `_update()`: Called every frame to handle game logic and state updates.
   * - `_draw()`: Called every frame after `_update()`, used to render the graphics.
   * 
   * Example:
   * @code
   * class Pico8App : public Pico8 {
   * public:
   *     void _init() override {
   *         // Initialize game variables
   *     }
   *     void _update() override {
   *         // Update game logic
   *     }
   *     void _draw() override {
   *         // Render the game screen
   *     }
   * };
   * @endcode
   * 
   * The `run()` function in this class is responsible for executing the application by calling
   * `_init()`, `_update()`, and `_draw()` in the appropriate order.
   */
  class Pico8 {
      friend class ImplPico8;
      ImplPico8* _impl;  ///< Pointer to the internal implementation.
      virtual void _init(){}   ///< User-defined initialization function.
      virtual void _update(){} ///< User-defined update function, called every frame.
      virtual void _draw(){}   ///< User-defined draw function, called every frame after `_update()`.

  public:
      /**
       * @brief Runs the application by calling the overridden `_init()`, `_update()`, and `_draw()`.
       */
      void run();
  };
}