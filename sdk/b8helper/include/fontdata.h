/**
 * @file fontdata.h
 * @brief Module for loading and displaying font data using the Picture Processing Unit (PPU).
 * 
 * This module provides functionality for loading specific font data into the PPU and displaying it.
 * It manages the loading of font data, mapping it to specific tiles, and sending it to the PPU to render
 * characters on the screen.
 * 
 * ### Overview of Features
 * 
 * - **Font Data Definition**: The `_font` array defines font data for 96 characters in an 8x8 bitmap format.
 * - **Pixel Setting**: The `_setpix` function sets a pixel at a specified coordinate with a specific color.
 * - **Tile Management**: The `fontdata` namespace provides functions to load font data into specific tiles
 *   and retrieve information about those tiles.
 * - **PPU Command Generation**: Generates and executes PPU commands to load the font data into the PPU.
 * 
 * ### Detailed Description
 * 
 * #### Loading Font Data
 * 
 * - **load Function**: Loads the font data into the PPU. It maps the font data to the specified tiles,
 *   applies shadows, and then sends the data to the PPU. It checks the `_is_loaded` flag to avoid reloading
 *   if the data is already loaded.
 * - **gettc Function**: Returns information about the clear tile.
 * - **dstxtile, dstytile Functions**: Return the X and Y coordinates of the tiles where the font data is loaded.
 * 
 * #### Pixel Setting
 * 
 * - `_setpix` Function: Sets a specific pixel in the image buffer with the given color. This ensures accurate
 *   rendering of the bitmap font data.
 * 
 * #### PPU Command Generation and Execution
 * 
 * - **b8PpuCmdSetBuff**: Sets the PPU command buffer.
 * - **b8PpuLoadimgAlloc**: Generates a command to load image data into the PPU.
 * - **b8PpuFlushAlloc**: Generates a command to flush the transfer of image data.
 * - **b8PpuHaltAlloc**: Generates a command to halt the PPU temporarily.
 * - **b8PpuExec**: Executes the PPU commands.
 * 
 * ### Usage Example
 * 
 * Here is an example of how to use this module to load font data and map it to specific tiles:
 * 
 * @code
 * #include "fontdata.h"
 * 
 * int main() {
 *     // Load font data
 *     fontdata::load();
 * 
 *     // Retrieve tile information
 *     b8PpuBgTile clearTile = fontdata::gettc();
 *     u8 xTile = fontdata::dstxtile();
 *     u8 yTile = fontdata::dstytile();
 * 
 *     // Perform drawing operations using the retrieved tile information
 *     // ...
 * 
 *     return 0;
 * }
 * @endcode
 * 
 * ### Summary
 * 
 * This module manages font data and loads it into the PPU for rendering. It defines font data in a bitmap format,
 * sends it to the PPU, and maps it to specific tiles to display characters on the screen. The primary use is to load
 * and display fonts at specified tile positions.
 * 
 * @version 1.0
 * @date 2024
 * 
 * @note This module depends on the b8/ppu.h header for PPU-related definitions and operations.
 */

#include <b8/ppu.h>
#pragma once
namespace fontdata {
  extern  void  load(
    u8 dstxtile_ = B8_PPU_MAX_WTILE - 2*16,
    u8 dstytile_ = B8_PPU_MAX_HTILE - 16
  );
  extern  b8PpuBgTile gettc();
  extern  u8 dstxtile() ;
  extern  u8 dstytile() ;
}
