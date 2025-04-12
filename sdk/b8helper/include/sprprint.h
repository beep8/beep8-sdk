#include <b8/ppu.h>
#pragma once
/**
 * @namespace sprprint
 * @brief This namespace contains functions and structures for managing sprite printing and positioning using the BEEP-8 PPU.
 */

/**
 * @brief Sample usage of the sprite printing system with various ANSI escape sequences.
 * 
 * This sample demonstrates how to set up and use sprite printing using BEEP-8 PPU with
 * different escape sequences for cursor movement, foreground/background color, and Z-index.
 * 
 * @code
 * static b8PpuCmd _ppu_cmd;
 * static FILE* _fp_sprprint;
 * 
 * void start_sprprint_sample() {
 *     // Initialize the sprite printing system and open a channel
 *     sprprint::Reset();
 *     sprprint::Context ctx;
 *     ctx._cmd = &_ppu_cmd;
 *     _fp_sprprint = sprprint::Open(sprprint::CH1, ctx);
 *     
 *     // Set cursor position to x=113, y=37 in pixel units (note: coordinates are (y, x)), and Z-index to 3
 *     // Unlike typical systems, cursor movement is in pixel units, not character cells.
 *     fprintf(_fp_sprprint, "\e[37;113H\e[3z");
 *     
 *     // Set foreground color to red and background color to blue
 *     fprintf(_fp_sprprint, "\e[31;44m");
 *     fprintf(_fp_sprprint, "Red text on blue background\n");
 *     
 *     // Reset color settings
 *     fprintf(_fp_sprprint, "\e[0m");
 *     fprintf(_fp_sprprint, "Default text color\n");
 *     
 *     // Set foreground color to BEEP-8 green and background color to BEEP-8 dark blue
 *     // Foreground color codes 50-65 correspond to BEEP-8 color indices 0-15.
 *     // Background color codes 70-85 correspond to BEEP-8 color indices 0-15.
 *     fprintf(_fp_sprprint, "\e[61;71m");
 *     
 *     // Enable shadow effect
 *     fprintf(_fp_sprprint, "\e[101mShadow enabled\n");
 *     
 *     // Disable shadow effect
 *     fprintf(_fp_sprprint, "\e[100mShadow disabled\n");
 * }
 * @endcode
 */


namespace sprprint {

  /**
   * @enum EnCmd
   * @brief Commands for controlling the sprite printing system.
   * 
   * - SET_SLOT_CONTEXT: Sets the context for the slot.
   * - GET_INFO: Retrieves the current state information.
   */
  enum EnCmd {
    SET_SLOT_CONTEXT,  ///< Command to set the slot context.
    GET_INFO           ///< Command to retrieve information.
  };

  /**
   * @enum EnCh
   * @brief Enumeration for selecting the channel used for sprite printing.
   * 
   * - CH0: First channel.
   * - CH1: Second channel.
   * - CHMAX: Maximum number of channels.
   */
  enum EnCh {
    CH0,   ///< Channel 0.
    CH1,   ///< Channel 1.
    CHMAX  ///< Maximum number of channels.
  };

  /**
   * @struct Context
   * @brief Holds the command context for sprite printing operations.
   * 
   * This structure contains a pointer to a command list used by the BEEP-8 PPU.
   */
  struct Context {
    b8PpuCmd* _cmd = nullptr;  ///< Pointer to PPU command list.
  };

  /**
   * @brief Resets the sprite printing system.
   * 
   * This function initializes the system and registers the necessary drivers.
   * It must be called before any sprite printing operations.
   */
  void Reset();

  /**
   * @struct Info
   * @brief Contains information about the current sprite printing state.
   * 
   * This structure stores the pixel location, foreground and background colors, and the z-index.
   */
  struct Info {
    s16 _xpix_locate = 0;   ///< X pixel location.
    s16 _ypix_locate = 0;   ///< Y pixel location.
    b8PpuColor _fg;         ///< Foreground color.
    b8PpuColor _bg;         ///< Background color.
    u16 _otz = 0;           ///< Z-index (order of display).
  };

  /**
   * @brief Retrieves information about the current state of sprite printing.
   * 
   * @param fp_ The file pointer to the current open sprite printing context.
   * @param dest The structure to store the retrieved information.
   * @return int Returns 0 on success, or a negative value on failure.
   *         Negative values indicate different error conditions (-1, -2, -3, etc.).
   */
  int GetInfo(FILE* fp_, Info& dest);

  /**
   * @brief Opens a sprite printing channel.
   * 
   * This function opens a channel for sprite printing, assigns it a context, and returns a file pointer for use in further operations.
   * 
   * @param ch_ The channel to open (CH0 or CH1).
   * @param ctx The context to be used with the opened channel.
   * @return FILE* Returns a file pointer to the opened channel, or NULL on failure.
   */
  FILE* Open(EnCh ch_, sprprint::Context& ctx);

  /**
   * @brief Sets the pixel location and Z-index for sprite printing.
   * 
   * This function positions the cursor at a specific pixel location and sets the Z-index (depth) for drawing operations.
   * 
   * @param fp_ The file pointer to the sprite printing context.
   * @param lx_ The X pixel location.
   * @param ly_ The Y pixel location.
   * @param otz_ The Z-index (depth) for drawing.
   */
  void Locate(FILE* fp_, s16 lx_, s16 ly_, u16 otz_);
  void LocateZ(FILE* fp_, u16 otz_);
  void Color(FILE* fp_, b8PpuColor b8col_ );
} // namespace sprprint
