/**
 * @file ppu.h
 * @brief Picture Processing Unit (PPU) definitions for the BEEP-8 system.
 * 
 * This file contains the definitions and function prototypes for the Picture Processing Unit (PPU) 
 * in the BEEP-8 system. It includes register definitions, command codes, color definitions, and 
 * structures for various PPU operations such as drawing rectangles, polygons, sprites, and handling 
 * background tiles.
 * 
 * The PPU is responsible for rendering graphics and handling various graphical operations in the 
 * BEEP-8 system. It supports a range of commands for different drawing and rendering tasks, as well 
 * as configurations for color palettes, view offsets, and scissoring.
 * 
 * For more detailed information, please refer to the BEEP-8 data sheet.
 */
#pragma once

// ppu / Picture Processing Unit
#ifdef  __cplusplus
extern  "C" {
#endif

#include <b8/type.h>
#include <b8/register.h>

// ppu
#define B8_PPU_ADDR         (0xffff8000)
#define B8_PPU_EXEC         _B8_REG(B8_PPU_ADDR + 0x00)
#define B8_PPU_STAT         _B8_REG(B8_PPU_ADDR + 0x04)
#define B8_PPU_INTCTRL      _B8_REG(B8_PPU_ADDR + 0x08)
#define B8_PPU_RESOLUTION   _B8_REG(B8_PPU_ADDR + 0x0c)

#define B8_PPU_CMD_NOP         (0x00)
#define B8_PPU_CMD_FLUSH       (0x01)
#define B8_PPU_CMD_ENABLE      (0x02)
#define B8_PPU_CMD_RECT        (0x10)
#define B8_PPU_CMD_POLY        (0x11)
#define B8_PPU_CMD_SPRITE      (0x12)
#define B8_PPU_CMD_SETPAL      (0x13)
#define B8_PPU_CMD_SETPHYPAL   (0x14)
#define B8_PPU_CMD_BG          (0x15)
#define B8_PPU_CMD_SCISSOR     (0x16)
#define B8_PPU_CMD_VIEWOFFSET  (0x17)
#define B8_PPU_CMD_LOADIMG     (0x20)
#define B8_PPU_CMD_LINE        (0x21)
#define B8_PPU_CMD_JMP         (0xf0)
#define B8_PPU_CMD_HALT        (0xff)

#define B8_PPU_COLOR_BLACK       (0)
#define B8_PPU_COLOR_DARK_BLUE   (1)
#define B8_PPU_COLOR_DARK_PURPLE (2)
#define B8_PPU_COLOR_DARK_GREEN  (3)
#define B8_PPU_COLOR_BROWN       (4)
#define B8_PPU_COLOR_DARK_GREY   (5)
#define B8_PPU_COLOR_LIGHT_GREY  (6)
#define B8_PPU_COLOR_WHITE       (7)
#define B8_PPU_COLOR_RED         (8)
#define B8_PPU_COLOR_ORANGE      (9)
#define B8_PPU_COLOR_YELLOW      (10)
#define B8_PPU_COLOR_GREEN       (11)
#define B8_PPU_COLOR_BLUE        (12)
#define B8_PPU_COLOR_LAVENDER    (13)
#define B8_PPU_COLOR_PINK        (14)
#define B8_PPU_COLOR_LIGHT_PEACH (15)

enum b8PpuColor {
    B8_BLACK       = B8_PPU_COLOR_BLACK,        // 0: Black
    B8_DARK_BLUE   = B8_PPU_COLOR_DARK_BLUE,    // 1: Dark Blue
    B8_DARK_PURPLE = B8_PPU_COLOR_DARK_PURPLE,  // 2: Dark Purple
    B8_DARK_GREEN  = B8_PPU_COLOR_DARK_GREEN,   // 3: Dark Green
    B8_BROWN       = B8_PPU_COLOR_BROWN,        // 4: Brown
    B8_DARK_GREY   = B8_PPU_COLOR_DARK_GREY,    // 5: Dark Grey
    B8_LIGHT_GREY  = B8_PPU_COLOR_LIGHT_GREY,   // 6: Light Grey
    B8_WHITE       = B8_PPU_COLOR_WHITE,        // 7: White
    B8_RED         = B8_PPU_COLOR_RED,          // 8: Red
    B8_ORANGE      = B8_PPU_COLOR_ORANGE,       // 9: Orange
    B8_YELLOW      = B8_PPU_COLOR_YELLOW,       // 10: Yellow
    B8_GREEN       = B8_PPU_COLOR_GREEN,        // 11: Green
    B8_BLUE        = B8_PPU_COLOR_BLUE,         // 12: Blue
    B8_LAVENDER    = B8_PPU_COLOR_LAVENDER,     // 13: Lavender
    B8_PINK        = B8_PPU_COLOR_PINK,         // 14: Pink
    B8_LIGHT_PEACH = B8_PPU_COLOR_LIGHT_PEACH,  // 15: Light Peach
    /* --- */
    B8_TRANSPARENT
};

#define B8_PPU_EXEC_START  (0x1)

/**
 * @brief Background wrapping mode: Clamp.
 * 
 * This mode clamps the background at the edges, preventing any wrapping.
 */
#define B8_PPU_BG_WRAP_CLAMP         (0x00)

/**
 * @brief Background wrapping mode: Clamp to edge.
 * 
 * This mode clamps the background to the edges, with some allowance for wrapping at the very edge.
 */
#define B8_PPU_BG_WRAP_CLAMP_TO_EDGE (0x01)

/**
 * @brief Background wrapping mode: Repeat.
 * 
 * This mode repeats the background texture, allowing it to wrap continuously.
 */
#define B8_PPU_BG_WRAP_REPEAT        (0x02)



#define PACKED_ALIGNED1 __attribute__((__packed__,aligned(1)))
#define PACKED_ALIGNED4 __attribute__((__packed__,aligned(4)))

/**
 * @brief Structure representing a PPU command buffer for the BEEP-8 system.
 * 
 * This structure holds information about the command buffer that the PPU uses for drawing operations.
 */
typedef struct _b8PpuCmd {
  u32*  buff;     /**< Pointer to the buffer storing the PPU commands. */
  u32   bytesize; /**< Size of the buffer in bytes. */
  u32*  sp;       /**< Stack pointer, used to keep track of the current position in the buffer. */
  u32*  tail;     /**< Pointer to the end of the buffer, indicating the last valid command. */

  u32*  ot;       /**< Pointer to the object table, if used. */
  u32*  ot_prev;
  u32   otnum;    /**< Number of objects in the object table. */
  u32*  addr_halt;
} b8PpuCmd;

/**
 * @brief Sets the buffer for PPU commands.
 * 
 * This function initializes the `b8PpuCmd` structure with the given buffer and its size.
 * 
 * Example usage:
 * @code
 * #define PPU_CMD_BUFF_WORDS (8*1024)
 * static u32 _ppu_cmd_buff[PPU_CMD_BUFF_WORDS];
 * b8PpuCmdSetBuff(&_ppu_cmd, _ppu_cmd_buff, sizeof(_ppu_cmd_buff));
 * @endcode
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure to be initialized.
 * @param buff_ Pointer to the buffer where PPU commands will be stored.
 * @param bytesize_ Size of the buffer in bytes.
 */
extern void b8PpuCmdSetBuff(b8PpuCmd* cmd_, u32* buff_, u32 bytesize_);

/**
 * @brief Pushes a command word onto the PPU command buffer.
 * 
 * This function adds a new command word to the current position in the PPU command buffer and then advances the stack pointer.
 * 
 * Example usage:
 * @code
 * b8PpuCmdPush(&_ppu_cmd, some_command_word);
 * @endcode
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param word_ The command word to be pushed onto the buffer.
 */
extern void b8PpuCmdPush(b8PpuCmd* cmd_, u32 word_);

/**
 * @brief Structure representing a rectangle drawing command for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED4 _b8PpuRect {
  unsigned pal  : 4;  /**< Palette number. */
  unsigned na   : 20; /**< Reserved, not used. */
  unsigned code : 8;  /**< Drawing command code. */

  signed y      : 16; /**< Y-coordinate of the top-left corner of the rectangle. */
  signed x      : 16; /**< X-coordinate of the top-left corner of the rectangle. */

  unsigned h    : 16; /**< Height of the rectangle. */
  unsigned w    : 16; /**< Width of the rectangle. */
} b8PpuRect;

/**
 * @brief Allocates and initializes a rectangle drawing command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuRect` structure within the PPU command buffer.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuRect` structure.
 */
extern b8PpuRect* b8PpuRectAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a rectangle drawing command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuRect` structure within the PPU command buffer and sets the Z-order for rendering.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the rectangle, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuRect` structure.
 */
extern b8PpuRect* b8PpuRectAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a rectangle drawing command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuRect` structure within the PPU command buffer and sets the Z-order for rendering. 
 * The rectangle is added to the back of the order table, meaning it will be rendered after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the rectangle, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuRect` structure.
 */
extern b8PpuRect* b8PpuRectAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing a sprite drawing command for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED4 _b8PpuSprite {
  unsigned pal      : 4;  /**< Palette number. */
  unsigned na0      : 20; /**< Reserved, not used. */
  unsigned code     : 8;  /**< Drawing command code. */

  unsigned srchtile : 5;  /**< Source tile height (in tiles). */
  unsigned vfp      : 1;  /**< Vertical flip flag. */
  unsigned na1      : 2;  /**< Reserved, not used. */

  unsigned srcwtile : 5;  /**< Source tile width (in tiles). */
  unsigned hfp      : 1;  /**< Horizontal flip flag. */
  unsigned na2      : 2;  /**< Reserved, not used. */

  unsigned srcytile : 6;  /**< Source Y-coordinate in tiles. */
  unsigned na3      : 2;  /**< Reserved, not used. */

  unsigned srcxtile : 6;  /**< Source X-coordinate in tiles. */
  unsigned na4      : 2;  /**< Reserved, not used. */

  signed y          : 16; /**< Y-coordinate of the sprite's top-left corner. */
  signed x          : 16; /**< X-coordinate of the sprite's top-left corner. */
} b8PpuSprite;

/**
 * @brief Allocates and initializes a sprite drawing command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuSprite` structure within the PPU command buffer.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuSprite` structure.
 */
extern b8PpuSprite* b8PpuSpriteAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a sprite drawing command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuSprite` structure within the PPU command buffer and sets the Z-order for rendering.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the sprite, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuSprite` structure.
 */
extern b8PpuSprite* b8PpuSpriteAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a sprite drawing command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuSprite` structure within the PPU command buffer and sets the Z-order for rendering. 
 * The sprite is added to the back of the order table, meaning it will be rendered after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the sprite, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuSprite` structure.
 */
extern b8PpuSprite* b8PpuSpriteAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing a palette setting command for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED4 _b8PpuSetpal {
  unsigned palsel     : 4;  /**< Palette selection index. */
  unsigned wmask      : 16; /**< write mask */
  unsigned reserved   : 4; /**< write mask */
  unsigned code   : 8;  /**< Command code for setting the palette. */

  unsigned pidx0  : 4;  /**< Palette index 0. */
  unsigned pidx1  : 4;  /**< Palette index 1. */
  unsigned pidx2  : 4;  /**< Palette index 2. */
  unsigned pidx3  : 4;  /**< Palette index 3. */
  unsigned pidx4  : 4;  /**< Palette index 4. */
  unsigned pidx5  : 4;  /**< Palette index 5. */
  unsigned pidx6  : 4;  /**< Palette index 6. */
  unsigned pidx7  : 4;  /**< Palette index 7. */

  unsigned pidx8  : 4;  /**< Palette index 8. */
  unsigned pidx9  : 4;  /**< Palette index 9. */
  unsigned pidx10 : 4;  /**< Palette index 10. */
  unsigned pidx11 : 4;  /**< Palette index 11. */
  unsigned pidx12 : 4;  /**< Palette index 12. */
  unsigned pidx13 : 4;  /**< Palette index 13. */
  unsigned pidx14 : 4;  /**< Palette index 14. */
  unsigned pidx15 : 4;  /**< Palette index 15. */
} b8PpuSetpal;

/**
 * @brief Allocates and initializes a SETPAL command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuSetpal` structure within the PPU command buffer 
 * and initializes the palette selection, write mask, and default palette indices. 
 * 
 * The command sets the `palsel` (palette selection index) to 0 by default and applies the write 
 * mask `wmask` to allow updates to all 16 palette entries. Only the palette entries corresponding 
 * to bits set to `1` in `wmask` will be updated. The palette entries (`pidx0` to `pidx15`) 
 * are initialized sequentially from 0 to 15, mapping to the respective color values in the selected palette.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated and initialized `b8PpuSetpal` structure.
 */
extern b8PpuSetpal* b8PpuSetpalAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a palette setting command in the PPU command buffer with a specified Z-order and cache flush option.
 * 
 * This function allocates memory for a `b8PpuSetpal` structure within the PPU command buffer, sets the Z-order for rendering, and optionally flushes the palette cache.
 * 
 * **Note:** Modifying the contents of the palette can incur significant runtime costs. 
 * It is recommended to use this API with caution. Since there are 16 palettes available, it is advisable to initialize their contents only during application startup to minimize performance impact during runtime.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz Z-order value for the palette, determining its rendering order.
 * @param flush Flag to control whether the palette image cache should be flushed. 
 *              Set to 1 to ensure the palette change is applied immediately. 
 *              If setting multiple palettes, you can set `flush` to 0 for all but the final palette setting to minimize the number of cache flushes. 
 *              However, use this with caution.
 * @return Pointer to the newly allocated `b8PpuSetpal` structure.
 */
extern b8PpuSetpal* b8PpuSetpalAllocZ(b8PpuCmd* cmd_, u32 otz, u8 flush);

/**
 * @brief Allocates and initializes a palette setting command in the PPU command buffer with a specified Z-order and cache flush option, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuSetpal` structure within the PPU command buffer, sets the Z-order for rendering, and optionally flushes the palette cache.
 * The palette setting command is added to the back of the order table, meaning it will be processed after previously inserted elements with the same Z-order.
 * 
 * **Note:** Modifying the contents of the palette can incur significant runtime costs. 
 * It is recommended to use this API with caution. Since there are 16 palettes available, it is advisable to initialize their contents only during application startup to minimize performance impact during runtime.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz Z-order value for the palette, determining its rendering order.
 * @param flush Flag to control whether the palette image cache should be flushed. 
 *              Set to 1 to ensure the palette change is applied immediately. 
 *              If setting multiple palettes, you can set `flush` to 0 for all but the final palette setting to minimize the number of cache flushes. 
 *              However, use this with caution.
 * @return Pointer to the newly allocated `b8PpuSetpal` structure.
 */
extern b8PpuSetpal* b8PpuSetpalAllocZPB(b8PpuCmd* cmd_, u32 otz, u8 flush);


/**
 * @brief Structure representing a background tile for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED1 _b8PpuBgTile {
  unsigned YTILE : 6; /**< Y-coordinate of the tile in the background map (in tiles). */
  unsigned XTILE : 6; /**< X-coordinate of the tile in the background map (in tiles). */
  unsigned VFP   : 1; /**< Vertical flip flag. */
  unsigned HFP   : 1; /**< Horizontal flip flag. */
  unsigned PAL   : 2; /**< Palette selection for the tile. */
} b8PpuBgTile;

/**
 * @brief Defines the maximum width of the background map in tiles.
 * 
 * This macro specifies the maximum number of horizontal tiles (`XTILE`) 
 * that can be used in the background map for the PPU.
 */
#define B8_PPU_MAX_WTILE  (64)

/**
 * @brief Defines the maximum height of the background map in tiles.
 * 
 * This macro specifies the maximum number of vertical tiles (`YTILE`) 
 * that can be used in the background map for the PPU.
 */
#define B8_PPU_MAX_HTILE  (64)

/**
 * @brief Structure representing a background configuration command for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED4 _b8PpuBg {
  unsigned htile  : 4; /**< Number of tiles in the vertical direction, specified as a power of 2. */
  unsigned na0    : 8; /**< Reserved, not used. */
  unsigned wtile  : 4; /**< Number of tiles in the horizontal direction, specified as a power of 2. */
  unsigned na1    : 8; /**< Reserved, not used. */
  unsigned code   : 8; /**< Command code for background configuration. */

  b8PpuBgTile* cpuaddr; /**< Pointer to the background tile data in CPU memory. */

  signed vpix     : 16; /**< Vertical offset for the background. */
  signed upix     : 16; /**< Horizontal offset for the background. */

  unsigned vwrap :  2;  /**< Vertical wrapping mode (see B8_PPU_BG_WRAP_*). */
  unsigned uwrap :  2;  /**< Horizontal wrapping mode (see B8_PPU_BG_WRAP_*). */
  unsigned na2   : 28;  /**< Reserved, not used. */
} b8PpuBg;

/**
 * @brief Allocates and initializes a background drawing command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuBg` structure within the PPU command buffer.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuBg` structure.
 */
extern b8PpuBg* b8PpuBgAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a background drawing command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuBg` structure within the PPU command buffer and sets the Z-order for rendering.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the background, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuBg` structure.
 */
extern b8PpuBg* b8PpuBgAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a background drawing command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuBg` structure within the PPU command buffer and sets the Z-order for rendering. 
 * The background command is added to the back of the order table, meaning it will be rendered after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the background, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuBg` structure.
 */
extern b8PpuBg* b8PpuBgAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing a polygon drawing command for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED4 _b8PpuPoly {
  unsigned pal   : 4;  /**< Palette number. */
  unsigned na0   : 20; /**< Reserved, not used. */
  unsigned code  : 8;  /**< Drawing command code. */

  signed y0      : 16; /**< First vertex x-coordinate of the polygon. */
  signed x0      : 16; /**< First vertex y-coordinate of the polygon. */

  signed y1      : 16; /**< Second vertex x-coordinate of the polygon. */
  signed x1      : 16; /**< Second vertex y-coordinate of the polygon. */

  signed y2      : 16; /**< Third vertex x-coordinate of the polygon. */
  signed x2      : 16; /**< Third vertex y-coordinate of the polygon. */
} b8PpuPoly;
/**
 * @brief Allocates and initializes a polygon drawing command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuPoly` structure within the PPU command buffer.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuPoly` structure.
 */
extern b8PpuPoly* b8PpuPolyAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a polygon drawing command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuPoly` structure within the PPU command buffer and sets the Z-order for rendering.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the polygon, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuPoly` structure.
 */
extern b8PpuPoly* b8PpuPolyAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a polygon drawing command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuPoly` structure within the PPU command buffer and sets the Z-order for rendering. 
 * The polygon command is added to the back of the order table, meaning it will be rendered after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the polygon, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuPoly` structure.
 */
extern b8PpuPoly* b8PpuPolyAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing a line drawing command for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED4 _b8PpuLine {
  unsigned pal   : 4;  /**< Palette number. */
  unsigned width : 4;  /**< Line width, specified in 0.5 increments (e.g., 0:0, 1:0.5, 2:1.0, 3:1.5 ... 15:7.5). */
  unsigned na0   : 16; /**< Reserved, not used. */
  unsigned code  : 8;  /**< Drawing command code. */

  signed y0      : 16; /**< Starting x-coordinate of the line segment. */
  signed x0      : 16; /**< Starting y-coordinate of the line segment. */

  signed y1      : 16; /**< Ending x-coordinate of the line segment. */
  signed x1      : 16; /**< Ending y-coordinate of the line segment. */
} b8PpuLine;

/**
 * @brief Allocates and initializes a line drawing command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuLine` structure within the PPU command buffer.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuLine` structure.
 */
extern b8PpuLine* b8PpuLineAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a line drawing command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuLine` structure within the PPU command buffer and sets the Z-order for rendering.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the line, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuLine` structure.
 */
extern b8PpuLine* b8PpuLineAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a line drawing command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuLine` structure within the PPU command buffer and sets the Z-order for rendering. 
 * The line command is added to the back of the order table, meaning it will be rendered after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the line, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuLine` structure.
 */
extern b8PpuLine* b8PpuLineAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing a scissor (clipping) command for the PPU (Pixel Processing Unit).
 */
typedef struct PACKED_ALIGNED4 _b8PpuScissor {
  unsigned na0   : 24; /**< Reserved, not used. */
  unsigned code  :  8; /**< Command code for the scissor (clipping) operation. */

  signed y       : 16; /**< Y-coordinate of the top-left corner of the clipping region. */
  signed x       : 16; /**< X-coordinate of the top-left corner of the clipping region. */

  signed h       : 16; /**< Height of the clipping region. */
  signed w       : 16; /**< Width of the clipping region. */
} b8PpuScissor;

/**
 * @brief Allocates and initializes a scissor (clipping) command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuScissor` structure within the PPU command buffer.
 * The scissor command defines a rectangular area within which rendering is allowed. 
 * Anything outside this area will be clipped and not rendered.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuScissor` structure.
 */
extern b8PpuScissor* b8PpuScissorAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a scissor (clipping) command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuScissor` structure within the PPU command buffer and sets the Z-order for rendering.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the scissor rectangle, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuScissor` structure.
 */
extern b8PpuScissor* b8PpuScissorAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a scissor (clipping) command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuScissor` structure within the PPU command buffer and sets the Z-order for rendering.
 * The scissor command is added to the back of the order table, meaning it will be applied after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the scissor rectangle, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuScissor` structure.
 */
extern b8PpuScissor* b8PpuScissorAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing a view offset command for the PPU (Pixel Processing Unit).
 * 
 * The `ViewOffset` command adds an offset to all drawing coordinates, effectively shifting the entire rendered view.
 * If this command is not set, the offset defaults to (0,0), meaning no offset is applied.
 */
typedef struct PACKED_ALIGNED4 _b8PpuViewoffset {
  unsigned na0   : 24; /**< Reserved, not used. */
  unsigned code  :  8; /**< Command code for the view offset operation. */

  signed y       : 16; /**< Y-coordinate offset applied to all drawing commands. */
  signed x       : 16; /**< X-coordinate offset applied to all drawing commands. */
} b8PpuViewoffset;

/**
 * @brief Allocates and initializes a view offset command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuViewoffset` structure within the PPU command buffer.
 * The view offset adds the specified X and Y offsets to all subsequent drawing coordinates.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuViewoffset` structure.
 */
extern b8PpuViewoffset* b8PpuViewoffsetAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a view offset command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuViewoffset` structure within the PPU command buffer and sets the Z-order for rendering.
 * The view offset adds the specified X and Y offsets to all subsequent drawing coordinates.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the view offset, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuViewoffset` structure.
 */
extern b8PpuViewoffset* b8PpuViewoffsetAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a view offset command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuViewoffset` structure within the PPU command buffer and sets the Z-order for rendering.
 * The view offset adds the specified X and Y offsets to all subsequent drawing coordinates. 
 * The command is added to the back of the order table, meaning it will be applied after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the view offset, determining its rendering order.
 * @return Pointer to the newly allocated `b8PpuViewoffset` structure.
 */
extern b8PpuViewoffset* b8PpuViewoffsetAllocZPB(b8PpuCmd* cmd_, u32 otz_);

typedef struct PACKED_ALIGNED4 _b8PpuNop {
  unsigned  na0       : 24;       // N/A
  unsigned  code      :  8;
} b8PpuNop;

/**
 * @brief Allocates and initializes a no-operation (NOP) command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuNop` structure within the PPU command buffer. 
 * The NOP command is used as a placeholder or synchronization point in the command sequence 
 * and does not perform any rendering operations.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuNop` structure.
 */
extern b8PpuNop* b8PpuNopAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a no-operation (NOP) command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuNop` structure within the PPU command buffer 
 * and adds it to the front of the order table (OT) with a specified Z-order. 
 * The NOP command does not perform any rendering operations but will be processed 
 * before previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the NOP command, determining its position in the rendering order.
 * @return Pointer to the newly allocated `b8PpuNop` structure.
 */
extern b8PpuNop* b8PpuNopAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a no-operation (NOP) command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuNop` structure within the PPU command buffer 
 * and adds it to the back of the order table (OT) with a specified Z-order. 
 * The NOP command does not perform any rendering operations but will be processed 
 * after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the NOP command, determining its position in the rendering order.
 * @return Pointer to the newly allocated `b8PpuNop` structure.
 */
extern b8PpuNop* b8PpuNopAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing a flush command for the PPU (Pixel Processing Unit).
 * 
 * The `Flush` command ensures that any changes made to VRAM or the palette through drawing commands 
 * are fully committed from the cache to the actual VRAM. This operation is known as cache flushing.
 * 
 * **Note:** Frequent use of the flush command can introduce performance overhead due to the cost of cache flushing. 
 * It is recommended to minimize the number of flush operations by grouping drawing commands efficiently. 
 * Only flush when necessary, especially when working with palette or VRAM updates.
 */
typedef struct PACKED_ALIGNED4 _b8PpuFlush {
  unsigned pal  :  1; /**< Flag to flush palette changes. */
  unsigned img  :  1; /**< Flag to flush image (VRAM) changes. */
  unsigned na0  : 22; /**< Reserved, not used. */
  unsigned code :  8; /**< Command code for the flush operation. */
} b8PpuFlush;

/**
 * @brief Allocate and initialize a flush command in the PPU command buffer.
 *
 * This function allocates space for a |b8PpuFlush| structure within the provided
 * PPU command buffer, sets its command code to `B8_PPU_CMD_FLUSH`, and initializes
 * both the palette and image flush flags to 0.
 *
 * @param cmd_ Pointer to the PPU command buffer structure.
 * @return Pointer to the newly allocated |b8PpuFlush| command.
 */
extern b8PpuFlush* b8PpuFlushAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocate and initialize a flush command in the PPU command buffer with Z-order.
 *
 * This function behaves like |b8PpuFlushAlloc| but also inserts the flush command
 * into the Ordering Table (OT) at the front (highest priority) for the specified Z-layer.
 *
 * @param cmd_ Pointer to the PPU command buffer structure.
 * @param otz_ Z-layer index at which to insert the flush command.
 * @return Pointer to the newly allocated |b8PpuFlush| command.
 */
extern b8PpuFlush* b8PpuFlushAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocate and initialize a flush command in the PPU command buffer with Z-order and back-of-OT insertion.
 *
 * This function behaves like |b8PpuFlushAlloc| but also appends the flush command
 * to the back (lowest priority) of the Ordering Table (OT) for the specified Z-layer.
 *
 * @param cmd_ Pointer to the PPU command buffer structure.
 * @param otz_ Z-layer index at which to append the flush command.
 * @return Pointer to the newly allocated |b8PpuFlush| command.
 */
extern b8PpuFlush* b8PpuFlushAllocZPB(b8PpuCmd* cmd_, u32 otz_);


/**
 * @brief Structure representing a halt command for the PPU (Pixel Processing Unit).
 * 
 * This command is used to signal the completion of the drawing commands.
 */
typedef struct PACKED_ALIGNED4 _b8PpuHalt {
  unsigned na0   : 24; /**< Reserved, not used. */
  unsigned code  :  8; /**< Command code for the halt operation. */
} b8PpuHalt;

/**
 * @brief Allocates and initializes a halt command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuHalt` structure within the PPU command buffer.
 * The halt command signals the end of the current set of drawing commands, allowing the PPU 
 * to complete processing.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuHalt` structure.
 */
extern b8PpuHalt* b8PpuHaltAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes a halt command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuHalt` structure within the PPU command buffer and sets 
 * the Z-order for the halt operation.
 * 
 * The halt command signals the end of the current set of drawing commands, allowing the PPU 
 * to complete processing.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the halt operation, determining its execution order relative to other commands.
 * @return Pointer to the newly allocated `b8PpuHalt` structure.
 */
extern b8PpuHalt* b8PpuHaltAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes a halt command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuHalt` structure within the PPU command buffer 
 * and sets the Z-order for the halt operation. The halt command signals the end of the current set of 
 * drawing commands, allowing the PPU to complete processing. The halt command is added to the back 
 * of the order table, meaning it will be processed after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the halt operation, determining its execution order relative to other commands.
 * @return Pointer to the newly allocated `b8PpuHalt` structure.
 */
extern b8PpuHalt* b8PpuHaltAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing an enable/disable command for the PPU (Pixel Processing Unit).
 * 
 * This command is used to enable or disable back-face culling during rendering. 
 * When `cul` is set to 1, back-face culling is enabled, meaning that certain surfaces 
 * or elements will not be rendered based on their orientation. When `cul` is set to 0, 
 * back-face culling is disabled, allowing all surfaces to be rendered regardless of orientation.
 */
typedef struct PACKED_ALIGNED4 _b8PpuEnable {
  unsigned cul  :  1; /**< Flag to enable (1) or disable (0) back-face culling. */
  unsigned na0  : 23; /**< Reserved, not used. */
  unsigned code :  8; /**< Command code for the enable/disable operation. */
} b8PpuEnable;

/**
 * @brief Command code for enabling back-face culling of polygons in the PPU.
 * 
 * This macro defines the bitmask used to enable back-face culling of polygons in the PPU.
 * When this bit is set in the `b8PpuEnable` structure, back-face culling will be enabled, 
 * meaning that polygons facing away from the camera or view will not be rendered. 
 * This helps improve performance by skipping the rendering of unnecessary surfaces.
 */
#define B8_PPU_CMD_ENABLE_CULLING_OF_POLYGONS  (1<<0)

/**
 * @brief Allocates and initializes an enable/disable command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuEnable` structure within the PPU command buffer.
 * The enable command configures the PPU to either enable or disable back-face culling based on the `cul` flag.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuEnable` structure.
 */
extern b8PpuEnable* b8PpuEnableAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes an enable/disable command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuEnable` structure within the PPU command buffer and sets the Z-order 
 * for the enable/disable operation.
 * 
 * The enable command configures the PPU to either enable or disable back-face culling based on the `cul` flag.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the enable/disable operation, determining its execution order relative to other commands.
 * @return Pointer to the newly allocated `b8PpuEnable` structure.
 */
extern b8PpuEnable* b8PpuEnableAllocZ(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Allocates and initializes an enable/disable command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuEnable` structure within the PPU command buffer 
 * and sets the Z-order for the enable/disable operation. The enable command configures the PPU 
 * to either enable or disable back-face culling based on the `cul` flag. 
 * The command is added to the back of the order table, meaning it will be processed after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz_ Z-order value for the enable/disable operation, determining its execution order relative to other commands.
 * @return Pointer to the newly allocated `b8PpuEnable` structure.
 */
extern b8PpuEnable* b8PpuEnableAllocZPB(b8PpuCmd* cmd_, u32 otz_);

/**
 * @brief Structure representing an image loading command for the PPU (Pixel Processing Unit).
 * 
 * The BEEP-8 system uses tiles of size 8x8 pixels as the basic unit for BG (background) and SPRITE references.
 * BEEP-8 has VRAM located in a different space from CPU memory, with a format of 64 tiles by 64 tiles.
 * Both sprites and backgrounds can only reference VRAM.
 * 
 * The `b8PpuLoadimg` structure is used to transfer tile compositions stored in CPU memory to VRAM.
 */
typedef struct PACKED_ALIGNED4 _b8PpuLoadimg {
  unsigned na0      : 24; /**< Reserved, not used. */
  unsigned code     :  8; /**< Command code for the image loading operation. */

  const u8* cpuaddr;      /**< Pointer to the tile data in CPU memory. */

  unsigned na1      :  8; /**< Reserved, not used. */
  unsigned srcwtile :  6; /**< Width of the source image in tiles. */
  unsigned na2      :  2; /**< Reserved, not used. */
  unsigned srcytile :  6; /**< Y-coordinate in tiles of the source image. */
  unsigned na3      :  2; /**< Reserved, not used. */
  unsigned srcxtile :  6; /**< X-coordinate in tiles of the source image. */
  unsigned na4      :  2; /**< Reserved, not used. */

  unsigned trnhtile :  6; /**< Height of the transfer region in tiles. */
  unsigned na5      :  2; /**< Reserved, not used. */
  unsigned trnwtile :  6; /**< Width of the transfer region in tiles. */
  unsigned na6      :  2; /**< Reserved, not used. */
  unsigned dstytile :  6; /**< Y-coordinate in tiles of the destination in VRAM. */
  unsigned na7      :  2; /**< Reserved, not used. */
  unsigned dstxtile :  6; /**< X-coordinate in tiles of the destination in VRAM. */
  unsigned na8      :  2; /**< Reserved, not used. */
} b8PpuLoadimg;

/**
 * @brief Allocates and initializes an image loading command in the PPU command buffer.
 * 
 * This function allocates memory for a `b8PpuLoadimg` structure within the PPU command buffer.
 * The image loading command is responsible for transferring tile data from CPU memory to VRAM.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @return Pointer to the newly allocated `b8PpuLoadimg` structure.
 */
extern b8PpuLoadimg* b8PpuLoadimgAlloc(b8PpuCmd* cmd_);

/**
 * @brief Allocates and initializes an image loading command in the PPU command buffer with a specified Z-order.
 * 
 * This function allocates memory for a `b8PpuLoadimg` structure within the PPU command buffer and sets the Z-order 
 * for the image loading operation.
 * 
 * The image loading command is responsible for transferring tile data from CPU memory to VRAM.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz Z-order value for the image loading operation, determining its execution order relative to other commands.
 * @return Pointer to the newly allocated `b8PpuLoadimg` structure.
 */
extern b8PpuLoadimg* b8PpuLoadimgAllocZ(b8PpuCmd* cmd_, u32 otz);

/**
 * @brief Allocates and initializes an image loading command in the PPU command buffer with a specified Z-order, adding it to the back of the order table.
 * 
 * This function allocates memory for a `b8PpuLoadimg` structure within the PPU command buffer 
 * and sets the Z-order for the image loading operation. The image loading command is responsible 
 * for transferring tile data from CPU memory to VRAM. The command is added to the back of the order table, 
 * meaning it will be processed after previously inserted elements with the same Z-order.
 * 
 * @param cmd_ Pointer to the `b8PpuCmd` structure representing the PPU command buffer.
 * @param otz Z-order value for the image loading operation, determining its execution order relative to other commands.
 * @return Pointer to the newly allocated `b8PpuLoadimg` structure.
 */
extern b8PpuLoadimg* b8PpuLoadimgAllocZPB(b8PpuCmd* cmd_, u32 otz);

/**
 * @brief Structure representing a jump command for the PPU (Pixel Processing Unit).
 * 
 * In the BEEP-8 system, the PPU parses drawing commands located at consecutive addresses.
 * Similar to the CPU's JMP instruction, this command allows the PPU to jump to a different address.
 */
typedef struct PACKED_ALIGNED4 _b8PpuJmp {
  unsigned cpuaddr4 : 24; /**< Target CPU address in words (4 bytes). */
  unsigned code     :  8; /**< Command code for the jump operation. */
} b8PpuJmp;

/**
 * @brief Allocates and initializes a PPU jump command.
 * 
 * @param cmd_ Pointer to the current PPU command.
 * @param cpuaddr_ Pointer to the CPU address where the jump should occur.
 * @return Pointer to the newly allocated `b8PpuJmp` structure.
 */
extern b8PpuJmp* b8PpuJmpAlloc(b8PpuCmd* cmd_, u32* cpuaddr_);

/**
 * @brief Allocates and initializes an Ordering Table (OT) for the PPU commands.
 *
 * This function allocates memory for an Ordering Table and sets up the necessary
 * jump commands and a halt command to create a linked list structure for PPU commands.
 * The OT is used to manage the order of rendering commands based on their Z-values.
 *
 * The function links the OT entries in reverse order, where each entry points to the
 * previous one, with the last entry being a halt command. This reverse linking ensures
 * that the PPU processes commands in the correct Z-order.
 *
 * @param cmd_ A pointer to the PPU command structure.
 * @param ot_ A pointer to the memory allocated for the Ordering Table, which must be the
 *            first element of a `u32` array of size `num_`. This array holds the OT entries.
 * @param ot_prev_ A pointer to the memory allocated for the previous Ordering Table entries,
 *                 also the first element of a `u32` array of size `num_`. This is used to link
 *                 the current OT with the previous OT.
 * @param num_ The number of entries in the Ordering Table, which defines the size of the `ot_` and `ot_prev_` arrays.
 *
 * The function will initialize the OT entries with jump commands that link each entry
 * to the previous one, with the last entry linked to a halt command. This setup ensures
 * the correct rendering order based on Z-values.
 *
 * Note: `ot_` and `ot_prev_` must both point to the first element of a `u32` array with at least `num_` entries. 
 * The function will modify these arrays as part of the linking process to manage the order of rendering commands.
 * 
 * Example:
 * @code
 *  ot_[ num_ - 1 ] -> HALT
 *  ot_[ num_ - 2 ] -> &ot_[ num_ - 1 ]
 *  ot_[ num_ - 3 ] -> &ot_[ num_ - 2 ]
 *    ...
 *  ot_[ 1 ] -> &ot_[ 0 ]
 *  ot_[ 0 ] -> HALT
 * @endcode
 */
extern void b8PpuClearOT(b8PpuCmd* cmd_, u32* ot_, u32* ot_prev_, u32 num_);

/**
 * @brief Adds a primitive to the front of the Ordering Table (OT) at the specified Z-value.
 *
 * The Ordering Table (OT) is used in the BEEP-8 PPU (Pixel Processing Unit) to manage
 * the rendering order of graphical primitives. This function inserts a primitive command
 * to the front of the OT at the position corresponding to the provided Z-value (otz_).
 *
 * Unlike `b8PpuPushBackOT`, which appends the primitive to the end of the OT list for a
 * given Z-value, this function inserts the primitive at the front of the OT, ensuring
 * that it is rendered before any existing primitives at the same Z-value.
 *
 * The function first validates the provided Z-value. If the Z-value is greater than or
 * equal to the total number of Z-layers (cmd_->otnum), it returns an error code indicating
 * an invalid value. If the Z-value is valid, the function proceeds to insert the primitive
 * at the specified Z-layer.
 *
 * The function works by updating both the OT and the jump pointers that manage the
 * rendering order. The new primitive is inserted before the first primitive in the same
 * Z-layer, maintaining the correct rendering sequence with depth sorting.
 *
 * **Note:** It is prohibited to use both `b8PpuPushFrontOT` and `b8PpuPushBackOT` on the
 * same Z-value (`otz_`). Doing so will result in undefined behavior.
 *
 * The following diagram illustrates how the OT is updated when a primitive is pushed
 * to the front of a Z-layer that already contains multiple primitives:
 *
 *        OT Index (Z-value)
 *          [0] -> Primitive A, Primitive F
 *          [1] -> Primitive B
 *          [2] -> Primitive G
 *          [3] -> Primitive C, Primitive H     ----> PushFront Primitive K at Z=3
 *          [4] -> Primitive D
 *          [5] -> Primitive E, Primitive I, Primitive J
 *
 * After insertion:
 *
 *        OT Index (Z-value)
 *          [0] -> Primitive A, Primitive F
 *          [1] -> Primitive B
 *          [2] -> Primitive G
 *          [3] -> Primitive K, Primitive C, Primitive H  ----> Updated to insert Primitive K at the front
 *          [4] -> Primitive D
 *          [5] -> Primitive E, Primitive I, Primitive J
 *
 * @param cmd_ A pointer to the PPU command structure, which holds the OT and other related data.
 * @param otz_ The Z-value at which the primitive should be inserted in the OT. This value
 *             determines the rendering priority of the primitive relative to others.
 * @param prim_ A pointer to the primitive command to be added to the OT. This command
 *              represents a graphical element that will be rendered by the PPU.
 *
 * If the specified Z-value (otz_) is greater than or equal to the number of entries
 * in the OT, the function will return an error code (-B8_INVALID_VALUE) indicating an
 * invalid value. Otherwise, it inserts the primitive to the OT, updating the jump commands
 * to maintain the correct rendering order based on depth.
 */
extern void b8PpuPushFrontOT(b8PpuCmd* cmd_, u32 otz_, void* prim_);

/**
 * @brief Adds a primitive to the end of the Ordering Table (OT) at the specified Z-value.
 *
 * The Ordering Table (OT) is used in the BEEP-8 PPU (Pixel Processing Unit) for managing
 * the rendering order of graphical primitives. This function adds a primitive command
 * to the back of the OT at the position corresponding to the provided Z-value (otz_).
 *
 * Unlike `b8PpuPushFrontOT`, which inserts the primitive at the beginning of the OT for a
 * given Z-value, this function appends the primitive to the end of the OT list for that
 * Z-value. This ensures that the new primitive is rendered after any existing primitives
 * at the same Z-value.
 *
 * The function first validates the provided Z-value. If the Z-value is greater than or
 * equal to the total number of Z-layers (cmd_->otnum), it returns an error code indicating
 * an invalid value. If the Z-value is valid, the function proceeds to append the primitive
 * at the specified Z-layer.
 *
 * The function works by updating both the OT and the jump pointers that manage the
 * rendering order. The new primitive is inserted after the last primitive in the same Z-layer,
 * maintaining the correct rendering sequence.
 *
 * **Note:** It is prohibited to use both `b8PpuPushFrontOT` and `b8PpuPushBackOT` on the
 * same Z-value (`otz_`). Doing so will result in undefined behavior.
 *
 * The following diagram illustrates how the OT is updated when a primitive is pushed
 * to the back of a Z-layer that already contains multiple primitives:
 *
 *        OT Index (Z-value)
 *          [0] -> Primitive A, Primitive F
 *          [1] -> Primitive B
 *          [2] -> Primitive G
 *          [3] -> Primitive C, Primitive H     ----> PushBack Primitive K at Z=3
 *          [4] -> Primitive D
 *          [5] -> Primitive E, Primitive I, Primitive J
 *
 * After insertion:
 *
 *        OT Index (Z-value)
 *          [0] -> Primitive A, Primitive F
 *          [1] -> Primitive B
 *          [2] -> Primitive G
 *          [3] -> Primitive C, Primitive H, Primitive K  ----> Updated to append Primitive K at the end
 *          [4] -> Primitive D
 *          [5] -> Primitive E, Primitive I, Primitive J
 *
 * @param cmd_ A pointer to the PPU command structure, which holds the OT and other related data.
 * @param otz_ The Z-value at which the primitive should be appended in the OT. This value
 *             determines the rendering priority of the primitive relative to others.
 * @param prim_ A pointer to the primitive command to be added to the OT. This command
 *              represents a graphical element that will be rendered by the PPU.
 *
 * If the specified Z-value (otz_) is greater than or equal to the number of entries
 * in the OT, the function will return an error code (-B8_INVALID_VALUE) indicating an
 * invalid value. Otherwise, it appends the primitive to the OT, updating the jump commands
 * to maintain the correct rendering order based on depth.
 */
extern void b8PpuPushBackOT(b8PpuCmd* cmd_, u32 otz_, void* prim_);

/**
 * @brief Executes the PPU commands stored in the buffer.
 *
 * This function triggers the execution of the PPU commands that have been
 * stored in the command buffer.
 *
 * @param cmd_ A pointer to the PPU command structure containing the commands to be executed.
 *
 * This function sets the PPU execution register to start processing the
 * commands from the buffer.
 */
extern  void  b8PpuExec( b8PpuCmd* cmd_ );

/**
 * @brief Enables the V-blank interrupt for the PPU.
 *
 * This function enables the V-blank interrupt, allowing the PPU to generate
 * an interrupt at the start of the vertical blanking interval.
 */
extern  void  b8PpuEnableVblankInterrupt( void );

/**
 * @brief Waits for the next V-blank interval.
 *
 * This function blocks the current thread until the next vertical blanking
 * interval occurs, ensuring synchronization with the display refresh rate.
 */
extern  void  b8PpuVsyncWait( void );

/**
 * @brief Gets the current screen resolution.
 *
 * This function retrieves the current screen width and height in pixels.
 *
 * @param ww A pointer to a variable where the screen width will be stored.
 * @param hh A pointer to a variable where the screen height will be stored.
 */
extern  void  b8PpuGetResolution( u32* ww, u32* hh );

/**
 * @brief Resets the PPU system.
 *
 * This function resets the PPU system and sets up the V-blank interrupt.
 * Note: This API is reserved for the BEEP-8 system. Users should not call this function directly.
 */
extern  void  b8PpuReset( void );

#ifdef  __cplusplus
}
#endif
