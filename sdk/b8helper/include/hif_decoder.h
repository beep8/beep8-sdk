/**
 * @file hif_decoder.h
 * @brief Module for decoding and managing HIF events in the BEEP-8 system.
 * 
 * This module provides functionality for decoding and managing human interface events (`b8HifEvent`)
 * from the Hardware Interface (HIF) system of the BEEP-8 system. The `CHifDecoder` class fetches
 * and manages events, storing them in an internal map, and allows retrieval of the current status
 * of input points such as touch and mouse events.
 * 
 * **Note:**
 * The `hif_decoder.h` is a utility library, and the use of this module is not mandatory. If you wish
 * to implement mouse control and multi-touch event handling yourself, or if you already have the
 * necessary expertise, you can use the APIs provided directly in `<b8/hif.h>`.
 * 
 * ### Features
 * 
 * - **HifPoint**: Structure representing a single input point (touch or mouse).
 * - **CHifDecoder**: Class for decoding and managing HIF events.
 * 
 * ### Usage Example
 * 
 * Here is an example of how to use this module to fetch and process events:
 * 
 * @code
    #include "hif_decoder.h"

    static CHifDecoder decoder;

    void some_func() {
      const auto& status = decoder.GetStatus();
      for (const auto& [key, value] : status) {
        switch (value->ev.type) {
          case B8_HIF_EV_TOUCH_START:
            printf(fp_bgprint, "%d t start\n", (int)value->hdl);
            break;
          case B8_HIF_EV_TOUCH_MOVE:
            printf(fp_bgprint, "%d t move\n", (int)value->hdl);
            break;
          case B8_HIF_EV_TOUCH_CANCEL:
            printf(fp_bgprint, "%d t cancel\n", (int)value->hdl);
            break;
          case B8_HIF_EV_TOUCH_END:
            printf(fp_bgprint, "%d t end\n", (int)value->hdl);
            break;
          case B8_HIF_EV_MOUSE_DOWN:
            printf(fp_bgprint, "%d m down %d %d\n", (int)value->hdl, value->ev.xp >> 4, value->ev.yp >> 4);
            break;
          case B8_HIF_EV_MOUSE_MOVE:
            printf(fp_bgprint, "%d m move %d %d\n", (int)value->hdl, value->ev.xp >> 4, value->ev.yp >> 4);
            break;
          case B8_HIF_EV_MOUSE_HOVER_MOVE:
            printf(fp_bgprint, "%d m hover %d %d\n", (int)value->hdl, value->ev.xp >> 4, value->ev.yp >> 4);
            break;
          case B8_HIF_EV_MOUSE_UP:
            printf(fp_bgprint, "%d m up %d %d\n", (int)value->hdl, value->ev.xp >> 4, value->ev.yp >> 4);
            break;
        }
      }
    }
 * @endcode
 * 
 * @version 1.0
 * @date 2024
 */

#pragma once

#include <map>
#include <b8/hif.h>

/**
 * @brief Structure representing a single input point (touch or mouse).
 * 
 * The `HifPoint` structure represents an active input point from the HIF system,
 * such as a touch or mouse event. It contains information about the event type,
 * a unique handle, and the event data itself.
 * 
 * @note
 * - The `ev.xp` and `ev.yp` members of the `b8HifEvent` structure are 16-bit signed fixed-point numbers
 *   with 4 fractional bits. To obtain the integer part of the coordinates, shift right by 4 bits
 *   (i.e., `ev.xp >> 4` and `ev.yp >> 4`).
 */
struct HifPoint {
  /**
   * @brief Enumeration of input point types.
   */
  enum PointType {
    None,   /**< No input */
    Mouse,  /**< Mouse input */
    Touch,  /**< Touch input */
  };

  u32 hdl = 0;            /**< Unique handle for the input point */
  PointType ptype = None; /**< Type of the input point (Mouse or Touch) */
  b8HifEvent ev;          /**< Event data associated with the input point */

  /**
   * @brief Constructs a `HifPoint` with the given event.
   * 
   * @param ev_ The HIF event associated with this input point.
   */
  HifPoint(const b8HifEvent& ev_)
    : ev(ev_) {}
};

class ImplCHifDecoder;

/**
 * @brief Class for decoding and managing HIF events.
 * 
 * The `CHifDecoder` class fetches and manages human interface events from the
 * Hardware Interface (HIF) system. It maintains an internal map of active input
 * points (`HifPoint` instances), identified by their event identifiers.
 */
class CHifDecoder {
  ImplCHifDecoder* impl = nullptr;

public:
  /**
   * @brief Constructs a `CHifDecoder` object.
   * 
   * Initializes the internal implementation of the decoder.
   */
  CHifDecoder();

  /**
   * @brief Destructs the `CHifDecoder` object.
   * 
   * Cleans up the internal implementation of the decoder.
   */
  ~CHifDecoder();

  /**
   * @brief Retrieves the current mouse or touch panel status.
   * 
   * This method provides access to the current state of the mouse or touch panel
   * by returning a pointer to a `b8HifMouseStatus` structure. The returned structure
   * includes information about the current position and dragging state of the mouse
   * or touch input.
   * 
   * **Usage Example:**
   * ```cpp
   * const b8HifMouseStatus* status = decoder.GetMouseStatus();
   * if (status->is_dragging) {
   *     int x = status->mouse_x >> 4; // Convert fixed-point to integer
   *     int y = status->mouse_y >> 4;
   *     // Handle dragging at position (x, y)
   * }
   * ```
   * 
   * @note
   * - The `mouse_x` and `mouse_y` values are 16-bit signed fixed-point numbers
   *   with 4 fractional bits. To obtain the integer part of the coordinates, shift
   *   right by 4 bits (i.e., `mouse_x >> 4`).
   * 
   * @return A pointer to the current `b8HifMouseStatus` structure.
   */
  const b8HifMouseStatus* GetMouseStatus();

  /**
   * @brief Retrieves the current status of active input points.
   * 
   * This function processes new HIF events and updates the internal state.
   * It returns a constant reference to a map containing pointers to
   * `HifPoint` instances, keyed by their event identifiers (`u8`).
   * The map represents the current active input points, such as ongoing
   * touch or mouse interactions.
   * 
   * **Usage Example:**
   * ```cpp
   * const auto& status = decoder.GetStatus();
   * for (const auto& [id, point] : status) {
   *     // Use point->ev.xp and point->ev.yp (shifted right by 4 bits for integer part)
   *     int x = point->ev.xp >> 4;
   *     int y = point->ev.yp >> 4;
   *     // Handle different event types
   *     switch (point->ev.type) {
   *         // ...
   *     }
   * }
   * ```
   * 
   * @return A constant reference to a map of active input points.
   */
  const std::map<u8, HifPoint*>& GetStatus();
};
