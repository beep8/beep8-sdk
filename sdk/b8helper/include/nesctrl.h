/**
 * @file nesctrl.h
 * @brief NES controller emulation module for touch screen devices.
 *
 * This module provides an emulation of the NES controller, allowing touch screen
 * devices to simulate NES controller inputs. It is designed for use in debugging
 * environments and tool development. In actual games, a serial console is not available,
 * so this module is intended solely for debugging purposes and is not mandatory for users.
 */
#pragma once

#ifdef  __cplusplus
extern  "C" {
#endif

#include <beep8.h>

/**
 * @class CfgNesCtrl
 * @brief Configuration for the NES controller.
 *
 * This class holds the configuration settings for the NES controller, including
 * the position, size, and appearance of the controller on the screen.
 */
class CfgNesCtrl {
public:
  s16 left_pix    =   8;   /**< X-coordinate offset in pixels. */
  s16 top_pix     = 160;   /**< Y-coordinate offset in pixels. */
  s16 radius_pix  =   4;   /**< Radius for touch detection in pixels. */

  u8  img_utile = B8_PPU_MAX_WTILE -  8; /**< Tile coordinate for image loading (horizontal). */
  u8  img_vtile = B8_PPU_MAX_HTILE - 16; /**< Tile coordinate for image loading (vertical). */

  u8  img_pal = 1; /**< Palette index for the controller image. */

  u8  clear_color = B8_PPU_COLOR_BLACK; /**< Clear color for the background. */

  bool  debug_visual = false; /**< Enable or disable debug visualization. */
};

/**
 * @enum ENesCtrl
 * @brief Enumeration of NES controller buttons.
 *
 * This enum defines the buttons available on the NES controller.
 */
enum class ENesCtrl : u8 {
  LEFT,   /**< Left button */
  UP,     /**< Up button */
  RIGHT,  /**< Right button */
  DOWN,   /**< Down button */
  BTN_A,  /**< A button */
  BTN_B,  /**< B button */
  MAX     /**< Maximum value */
};

class ImplCNesCtrl;

/**
 * @class CNesCtrl
 * @brief NES controller emulation class.
 *
 * This class emulates an NES controller using touch screen inputs. It processes
 * touch events, updates the controller state, and renders the controller on the screen.
 */
class CNesCtrl {
  ImplCNesCtrl* impl;
  CfgNesCtrl    _cfg;
public:
  /**
   * @brief Reset the NES controller state and load images.
   */
  void  Reset();

  /**
   * @brief Process touch events and update the controller state.
   */
  void  Step();

  /**
   * @brief Draw the NES controller on the screen.
   * @param pcmd Pointer to the PPU command buffer.
   * @param otz Order of drawing.
   */
  void  Draw( b8PpuCmd* pcmd , u32 otz );

  /**
   * @brief Get the hold count for a specific button.
   * @param enc NES controller button.
   * @return Number of frames the button has been held.
   */
  u32   GetCntHold(    ENesCtrl enc );

  /**
   * @brief Get the release count for a specific button.
   * @param enc NES controller button.
   * @return Number of frames since the button was released.
   */
  u32   GetCntRelease( ENesCtrl enc );

  /**
   * @brief Constructor for CNesCtrl.
   * @param cfg Configuration for the NES controller.
   */
  CNesCtrl( const CfgNesCtrl& cfg );

  /**
   * @brief Destructor for CNesCtrl.
   */
  ~CNesCtrl();
};

#ifdef  __cplusplus
}
#endif
