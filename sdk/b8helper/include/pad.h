/**
 * @file pad.h
 * @brief This module provides classes and functions for managing gamepad input.
 *
 * The pad module offers functionalities to track and handle the state of gamepad buttons,
 * integrating with NES controller emulation for touch screen inputs. It is designed for use in 
 * debugging environments, providing visual feedback and interaction through gamepad input.
 *
 * The module is not intended for direct use in production games where a serial console does not exist.
 * Its primary purpose is for debugging and development tools.
 * 
 * @note This module serves only as a utility to emulate an NES controller on a touch panel,
 * and its use is not recommended.
 *
 * Usage example:
 * @code
 * #include <stdio.h>
 * #include <beep8.h>
 * #include <nesctrl.h>
 * #include "pad.h"
 *
 * // Define NES controller configuration
 * CfgNesCtrl cfg;
 * cfg.left_pix = 10;
 * cfg.top_pix = 10;
 * cfg.radius_pix = 5;
 * cfg.img_utile = B8_PPU_MAX_WTILE - 10;
 * cfg.img_vtile = B8_PPU_MAX_HTILE - 20;
 * cfg.img_pal = 1;
 * cfg.clear_color = B8_PPU_COLOR_BLACK;
 * cfg.debug_visual = true;
 *
 * // Create an instance of CPadObj
 * CPadObj pad(0, &cfg);
 *
 * // Main loop
 * while (true) {
 *     pad.vOnStep();
 *     // Initialize draw command buffer
 *     b8PpuCmd cmd;
 *     pad.vOnDraw(&cmd);
 *     
 *     // Check button states
 *     if (pad.GetCntHold(EPad::BTN_A) > 0) {
 *         printf("A button is being held\n");
 *     }
 *     if (pad.GetCntRelease(EPad::BTN_B) > 0) {
 *         printf("B button is released\n");
 *     }
 *     // Check other button states as needed
 *     // ...
 *
 *     // Other game logic and rendering
 *     // ...
 * }
 * @endcode
 */

#pragma once
#include <cobj.h>
#include <nesctrl.h>

class CfgNesCtrl;
class ImplCPadObj;

/**
 * @enum EPad
 * @brief Enum for gamepad buttons.
 */
enum class EPad {
  START, SELECT, LEFT, UP, RIGHT, DOWN, BTN_A, BTN_B , EPAD_MAX
};

/**
 * @class CPadObj
 * @brief Class for managing a gamepad's input and state.
 *
 * The CPadObj class tracks the state of gamepad buttons, integrates with NES controller emulation,
 * and provides drawing functionalities for visual feedback.
 */
class CPadObj : public CObj {
  ImplCPadObj* impl;
  u32   _idx;
private:
  u32   _cnt_hold   [ 8 ] = {0};
  u32   _cnt_release[ 8 ] = {0};
  void  vOnStep() override;
  void  vOnDraw( b8PpuCmd* cmd_ ) override;
public:
  /**
   * @brief Get the hold count for a specific button.
   * @param epad_ The button to check.
   * @return The count of frames the button has been held.
   */
  u32   GetCntHold( EPad epad_ ) const {
    return _cnt_hold[ static_cast< u32 >( epad_ ) ];
  }

  /**
   * @brief Get the release count for a specific button.
   * @param epad_ The button to check.
   * @return The count of frames the button has been released.
   */
  u32   GetCntRelease( EPad epad_ ) const {
    return _cnt_release[ static_cast< u32 >( epad_ ) ];
  }
public:
  /**
   * @brief Construct a new CPadObj object.
   * @param idx_ The index of the gamepad.
   * @param pcfg_ Pointer to the NES controller configuration.
   */
  CPadObj( u32 idx_ , const CfgNesCtrl* pcfg_ = nullptr );

  /**
   * @brief Destroy the CPadObj object.
   */
  virtual ~CPadObj();
};

/**
 * @brief Get the CPadObj instance for a specific index.
 * @param idx_ The index of the gamepad.
 * @return Pointer to the CPadObj instance.
 */
extern  CPadObj*  _CPadObj( u32 idx_ );
