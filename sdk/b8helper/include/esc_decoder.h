/**
 * @file esc_decoder.h
 * @brief Module for decoding and handling ANSI escape sequences.
 * 
 * This module is intended for device driver developers and is not meant to be used directly by general users.
 * 
 * This module provides functionality for decoding escape sequences using the
 * `CEscapeSeqDecoder` class and associated functions. Escape sequences are special
 * strings used primarily for terminal display control, such as moving the cursor,
 * clearing the screen, or modifying text appearance.
 * 
 * ### Overview of Features
 * * Supported Escape Sequences:
 * 
 * - Cursor movement:
 *   \e[H       - Moves cursor to the home position (top-left corner).
 *   \e[x;yH    - Moves cursor to position (x, y), where x and y are 1-based coordinates.
 *
 * - Screen clearing:
 *   \e[0J      - Clears screen from the cursor down (ED0).
 *   \e[1J      - Clears screen from the cursor up (ED1).
 *   \e[2J      - Clears the entire screen (ED2).
 * 
 * - Depth (Z-value) control:
 *   \e[3z      - Sets the Z-value (depth) for subsequent text drawing operations only.
 *     This allows controlling the order of text rendering elements based on their depth.
 *     Higher Z-values are drawn behind lower Z-values.
 *     Note: This depth setting applies only to text and does not affect the depth of graphical elements.
 *     Example: \e[5z sets the text depth to 5 for future text operations.
 *
 * - Line clearing:
 *   \e[0K      - Clears line from cursor to the right (EL0).
 *   \e[1K      - Clears line from cursor to the left (EL1).
 *   \e[2K      - Clears the entire line (EL2).
 *
 * - Text attributes and colors:
 *   \e[attr;fg;bgm - Sets text attributes, foreground color, and background color.
 *     Example: \e[1;37;40m - High brightness, white text on a black background.
 *     (attr: 0=Reset, 1=Bold/High intensity; fg=foreground color; bg=background color)
 * 
 * - ANSI Standard Colors (Foreground and Background):
 *   \e[30m - Sets the foreground color to Black.
 *   \e[31m - Sets the foreground color to Red.
 *   \e[32m - Sets the foreground color to Green.
 *   \e[33m - Sets the foreground color to Yellow.
 *   \e[34m - Sets the foreground color to Blue.
 *   \e[35m - Sets the foreground color to Magenta (Purple).
 *   \e[36m - Sets the foreground color to Cyan.
 *   \e[37m - Sets the foreground color to White.
 *   \e[40m - Sets the background color to Black.
 *   \e[41m - Sets the background color to Red.
 *   \e[42m - Sets the background color to Green.
 *   \e[43m - Sets the background color to Yellow.
 *   \e[44m - Sets the background color to Blue.
 *   \e[45m - Sets the background color to Magenta (Purple).
 *   \e[46m - Sets the background color to Cyan.
 *   \e[47m - Sets the background color to White.
 * 
 * - BEEP-8 Custom Colors (Foreground and Background):
 *   \e[50m - Sets the foreground color to BEEP-8 Black.
 *   \e[51m - Sets the foreground color to BEEP-8 Dark Blue.
 *   \e[52m - Sets the foreground color to BEEP-8 Dark Purple.
 *   \e[53m - Sets the foreground color to BEEP-8 Dark Green.
 *   \e[54m - Sets the foreground color to BEEP-8 Brown.
 *   \e[55m - Sets the foreground color to BEEP-8 Dark Grey.
 *   \e[56m - Sets the foreground color to BEEP-8 Light Grey.
 *   \e[57m - Sets the foreground color to BEEP-8 White.
 *   \e[58m - Sets the foreground color to BEEP-8 Red.
 *   \e[59m - Sets the foreground color to BEEP-8 Orange.
 *   \e[60m - Sets the foreground color to BEEP-8 Yellow.
 *   \e[61m - Sets the foreground color to BEEP-8 Green.
 *   \e[62m - Sets the foreground color to BEEP-8 Blue.
 *   \e[63m - Sets the foreground color to BEEP-8 Lavender.
 *   \e[64m - Sets the foreground color to BEEP-8 Pink.
 *   \e[65m - Sets the foreground color to BEEP-8 Light Peach.
 *   \e[70m - Sets the background color to BEEP-8 Black.
 *   \e[71m - Sets the background color to BEEP-8 Dark Blue.
 *   \e[72m - Sets the background color to BEEP-8 Dark Purple.
 *   \e[73m - Sets the background color to BEEP-8 Dark Green.
 *   \e[74m - Sets the background color to BEEP-8 Brown.
 *   \e[75m - Sets the background color to BEEP-8 Dark Grey.
 *   \e[76m - Sets the background color to BEEP-8 Light Grey.
 *   \e[77m - Sets the background color to BEEP-8 White.
 *   \e[78m - Sets the background color to BEEP-8 Red.
 *   \e[79m - Sets the background color to BEEP-8 Orange.
 *   \e[80m - Sets the background color to BEEP-8 Yellow.
 *   \e[81m - Sets the background color to BEEP-8 Green.
 *   \e[82m - Sets the background color to BEEP-8 Blue.
 *   \e[83m - Sets the background color to BEEP-8 Lavender.
 *   \e[84m - Sets the background color to BEEP-8 Pink.
 *   \e[85m - Sets the background color to BEEP-8 Light Peach.

 * 
 * - Shadow control:
 *   \e[101m    - Enables shadow effect.
 *   \e[100m    - Disables shadow effect.
 *
 * - Custom palette selection:
 *   \e[nq      - Selects palette n, where n is a value between 0 and 3.
 *
 * - Cursor visibility control:
 *   \e[>       - Hides the cursor.
 *   \e[<       - Shows the cursor.
 *
 * - Special cursor movement (custom sequences):
 *   \e[O       - Custom escape sequence for cursor movement (defined in the code).
 * 
 * - Delete:
 *   \e[~       - Deletes a character (DEL).
 *
 * #### `CEscapeSeqDecoder` Class
 * 
 * The `CEscapeSeqDecoder` class contains an internal state machine that decodes escape sequences
 * and outputs the resulting operation and parameters using the `EscapeOut` structure.
 *
 * ### Sample Usage
 * 
 * Here is a comprehensive set of examples demonstrating the use of various escape sequences
 * supported by this module:
 * 
 * @code
 * #include "esc_decoder.h"
 * int main() {
 *     FILE* fp = stdout;
 *
 *     // Cursor movement
 *     fprintf(fp, "\e[H");         // Move cursor to home (top-left)
 *     fprintf(fp, "\e[10;20H");    // Move cursor to position (10, 20)
 *
 *     // Screen clearing
 *     fprintf(fp, "\e[2J");        // Clear the entire screen
 *     fprintf(fp, "\e[0J");        // Clear from cursor down
 *     fprintf(fp, "\e[1J");        // Clear from cursor up
 *
 *     // Depth (Z-value) control
 *     fprintf(fp, "\e[5z");        // Set Z-value (depth) to 5
 *
 *     // Line clearing
 *     fprintf(fp, "\e[2K");        // Clear entire line
 *     fprintf(fp, "\e[0K");        // Clear from cursor to right
 *     fprintf(fp, "\e[1K");        // Clear from cursor to left
 *
 *     // Text attributes and colors
 *     fprintf(fp, "\e[1;37;40mBold white text on black background\n");
 *     fprintf(fp, "\e[0;33;41mYellow text on red background\n");
 *
 *     // ANSI Standard Colors (Foreground and Background)
 *     fprintf(fp, "\e[31mRed foreground\n");
 *     fprintf(fp, "\e[44mBlue background\n");
 *     fprintf(fp, "\e[0mReset to default colors\n");
 *
 *     // BEEP-8 Custom Colors (Foreground and Background)
 *     fprintf(fp, "\e[58;78mBEEP-8 Red foreground with BEEP-8 Red background\n");
 *     fprintf(fp, "\e[62;80mBEEP-8 Blue foreground with BEEP-8 Yellow background\n");
 *     fprintf(fp, "\e[0mReset to default colors\n");
 *     fprintf(fp, "\e[1;37;41mA_WHITE / A_RED\n");    // White text, red background
 *     fprintf(fp, "\e[1;30;47mA_BLACK / A_WHITE\n");  // Black text, white background
 *     fprintf(fp, "\e[1;31;40mA_RED / A_BLACK\n");    // Red text, black background
 *     fprintf(fp, "\e[1;32;44mA_GREEN / A_BLUE\n");   // Green text, blue background
 *     fprintf(fp, "\e[1;33;45mA_YELLOW / A_MAGENTA\n"); // Yellow text, magenta background
 *     fprintf(fp, "\e[1;34;43mA_BLUE / A_YELLOW\n");  // Blue text, yellow background
 *     fprintf(fp, "\e[1;35;42mA_MAGENTA / A_GREEN\n"); // Magenta text, green background
 *     fprintf(fp, "\e[1;36;46mA_CYAN / A_CYAN\n");    // Cyan text, cyan background
 *     fprintf(fp, "\e[1;90;41mA_BRIGHT_BLACK / A_RED\n"); // Bright black text, red background
 *     fprintf(fp, "\e[1;97;100mA_BRIGHT_WHITE / A_BRIGHT_BLACK\n"); // Bright white text, bright black background
 * 
 *     // Test combinations of foreground and background colors with shorthand notation
 *     fprintf(fp, "\e[57;72mWhite/Purple\n"); // Foreground: White, Background: Dark Purple
 *     fprintf(fp, "\e[59;73mOrange/Green\n"); // Foreground: Orange, Background: Dark Green
 *     fprintf(fp, "\e[60;74mYellow/Brown\n"); // Foreground: Yellow, Background: Brown
 *     fprintf(fp, "\e[61;75mGreen/DarkGrey\n"); // Foreground: Green, Background: Dark Grey
 *     fprintf(fp, "\e[62;76mBlue/LightGrey\n"); // Foreground: Blue, Background: Light Grey
 *     fprintf(fp, "\e[63;77mLavender/White\n"); // Foreground: Lavender, Background: White
 *     fprintf(fp, "\e[64;78mPink/Red\n"); // Foreground: Pink, Background: Red
 *     fprintf(fp, "\e[65;79mPeach/Orange\n"); // Foreground: Light Peach, Background: Orange
 *     fprintf(fp, "\e[50;71mBlack/Blue\n"); // Foreground: Black, Background: Dark Blue
 *     fprintf(fp, "\e[51;70mDarkBlue/Black\n"); // Foreground: Dark Blue, Background: Black
 *
 *     // Shadow control
 *     fprintf(fp, "\e[101mShadow enabled\n");
 *     fprintf(fp, "\e[100mShadow disabled\n");
 *
 *     // Custom palette selection
 *     fprintf(fp, "\e[2qSelected custom palette 2\n");
 *
 *     // Cursor visibility control
 *     fprintf(fp, "\e[>Cursor hidden\n");
 *     fprintf(fp, "\e[<Cursor shown\n");
 *
 *     // Special cursor movement
 *     fprintf(fp, "\e[OCustom cursor movement\n");
 *
 *     // Delete character
 *     fprintf(fp, "\e[~Deleted character\n");
 * 
 *     return 0;
 * }
 *
 * @endcode
 *
 * ### Escape Sequence Format
 * - **Cursor Movement**: \e[x;yH moves the cursor to the specified position `(x, y)`.
 * - **Screen and Line Clearing**: Various `J` and `K` commands control clearing of the screen or lines.
 * - **Text Attributes**: The `m` command modifies text attributes and colors (bold, foreground, background).
 * 
 * ### Detailed Description of `CEscapeSeqDecoder` Class
 * 
 * The `CEscapeSeqDecoder` class manages the decoding process through a state machine defined by the `EscState` enumeration.
 * It tracks the current state of the decoder and processes each character in the escape sequence to determine the appropriate action.
 * The parsed operations are stored in the `EscapeOut` structure.
 * 
 * - **State Machine Implementation**: The `Stream` method processes each character and handles transition between states
 *   such as `ES_0x1b` (Escape), `ES_0x5b` (CSI), and other custom states for handling sequences like palette selection.
 * - **Color Handling**: The decoder parses color codes in the format `\e[attr;fg;bgm` where:
 *   - `attr`: Text attributes (e.g., `0=Reset`, `1=Bold`).
 *   - `fg`: Foreground color (e.g., `37=White`).
 *   - `bg`: Background color (e.g., `44=Blue`).
 * - **Cursor Movement**: The decoder recognizes sequences like `\e[x;yH` for moving the cursor to a specific position.
 * 
 * ### Operations and Parameters (`EscapeOut`)
 * 
 * The `EscapeOut` structure stores the result of decoding an escape sequence:
 * 
 * - `_code`: The last processed character.
 * - `_Ope`: The operation determined by the escape sequence.
 * - `_fg`: The foreground color.
 * - `_bg`: The background color.
 * - `_x`, `_y`: Cursor positions for movement commands.
 * - `_EscapePAL`: The selected palette for custom palette handling.
 * 
 * ### Additional Helper Functions
 * 
 * - `EscClearEntireScreen`: Clears the entire screen.
 * - `EscMoveCursor`: Moves the cursor to a specific position `(x, y)`.
 * 
 * ### Summary
 * 
 * This module provides a robust system for decoding and handling ANSI escape sequences in terminal environments. It supports cursor movement,
 * text color and attribute manipulation, screen and line clearing, and custom palette selection. The state machine-based parser efficiently
 * processes complex sequences and outputs the necessary operations in a structured format.
 * 
 * @version 1.0
 * @date 2024
 * 
 * @note This module depends on the b8/type.h header for type definitions.
 */

#pragma once
#include <b8/type.h>

class ImplCEscapeSeqDecoder;

/**
 * @brief Enumeration for selecting a palette in the terminal.
 * 
 * The `EscapePAL` enumeration represents the available palettes that can be selected
 * using escape sequences. The values range from `PAL_0` to `PAL_3`, corresponding
 * to different palettes in the system.
 */
enum EscapePAL {
    PAL_0,  ///< Palette 0
    PAL_1,  ///< Palette 1
    PAL_2,  ///< Palette 2
    PAL_3,  ///< Palette 3
};

/**
 * @brief Enumeration for escape sequence operations.
 * 
 * The `EscapeSeqOpe` enumeration defines the various operations that can be parsed
 * and executed from escape sequences, such as moving the cursor, clearing the screen,
 * or setting text colors and attributes.
 */
enum EscapeSeqOpe {
    ESO_ONE_CHAR,                        ///< Processes a single character
    ESO_UP,                              ///< Moves the cursor up
    ESO_DOWN,                            ///< Moves the cursor down
    ESO_RIGHT,                           ///< Moves the cursor right
    ESO_LEFT,                            ///< Moves the cursor left
    ESO_DEL,                             ///< Deletes a character
    ESO_MOVE_CURSOR,                     ///< Moves the cursor to a specified position
    ESO_SEL_PAL,                         ///< Selects a palette
    ESO_SET_COLOR,                       ///< Sets the foreground and background color
    ESO_CLEAR_SCREEN_FROM_CURSOR_DOWN,   ///< Clears the screen from the cursor down
    ESO_CLEAR_SCREEN_FROM_CURSOR_UP,     ///< Clears the screen from the cursor up
    ESO_CLEAR_ENTIRE_SCREEN,             ///< Clears the entire screen
    ESO_CLEAR_LINE_FROM_CURSOR_RIGHT,    ///< Clears the line from the cursor to the right
    ESO_CLEAR_LINE_FROM_CURSOR_LEFT,     ///< Clears the line from the cursor to the left
    ESO_CLEAR_ENTIRE_LINE,               ///< Clears the entire line
    ESO_ENABLE_SHADOW,                   ///< Enables the shadow effect
    ESO_DISABLE_SHADOW,                  ///< Disables the shadow effect
    ESO_SET_Z,                           ///< Sets the z value
    ESO_NONE                             ///< No operation
};

/**
 * @brief Enumeration for ANSI color codes.
 * 
 * The `AnsiColor` enumeration defines the standard and high-intensity colors used
 * in ANSI escape sequences. The values correspond to the codes used to set foreground
 * and background colors in terminal environments.
 */
enum AnsiColor {
  ANSI_NULL = 0,

  ANSI_COLOR_BLACK   = 30,  // Black   (0, 0, 0)
  ANSI_COLOR_RED,           // Red     (255, 0, 0)
  ANSI_COLOR_GREEN,         // Green   (0, 255, 0)
  ANSI_COLOR_YELLOW,        // Yellow  (255, 255, 0)
  ANSI_COLOR_BLUE,          // Blue    (0, 0, 255)
  ANSI_COLOR_MAGENTA,       // Magenta (255, 0, 255)  // Magenta can also be seen as purple
  ANSI_COLOR_CYAN,          // Cyan    (0, 255, 255)
  ANSI_COLOR_WHITE,         // White   (255, 255, 255)

  ANSI_COLOR_BLACK_BG = 40,
  ANSI_COLOR_RED_BG,
  ANSI_COLOR_GREEN_BG,
  ANSI_COLOR_YELLOW_BG,
  ANSI_COLOR_BLUE_BG,
  ANSI_COLOR_MAGENTA_BG,
  ANSI_COLOR_CYAN_BG,
  ANSI_COLOR_WHITE_BG,

  ANSI_COLOR_B8_BLACK       = 50, // BEEP-8 Black
  ANSI_COLOR_B8_DARK_BLUE   ,     // BEEP-8 Dark Blue
  ANSI_COLOR_B8_DARK_PURPLE ,     // BEEP-8 Dark Purple
  ANSI_COLOR_B8_DARK_GREEN  ,     // BEEP-8 Dark Green
  ANSI_COLOR_B8_BROWN       ,     // BEEP-8 Brown
  ANSI_COLOR_B8_DARK_GREY   ,     // BEEP-8 Dark Grey
  ANSI_COLOR_B8_LIGHT_GREY  ,     // BEEP-8 Light Grey
  ANSI_COLOR_B8_WHITE       ,     // BEEP-8 White
  ANSI_COLOR_B8_RED         ,     // BEEP-8 Red
  ANSI_COLOR_B8_ORANGE      ,     // BEEP-8 Orange
  ANSI_COLOR_B8_YELLOW      ,     // BEEP-8 Yellow
  ANSI_COLOR_B8_GREEN       ,     // BEEP-8 Green
  ANSI_COLOR_B8_BLUE        ,     // BEEP-8 Blue
  ANSI_COLOR_B8_LAVENDER    ,     // BEEP-8 Lavender
  ANSI_COLOR_B8_PINK        ,     // BEEP-8 Pink
  ANSI_COLOR_B8_LIGHT_PEACH ,     // BEEP-8 Light Peach

  ANSI_COLOR_B8_BLACK_BG    = 70,
  ANSI_COLOR_B8_DARK_BLUE_BG    ,   
  ANSI_COLOR_B8_DARK_PURPLE_BG  , 
  ANSI_COLOR_B8_DARK_GREEN_BG   ,  
  ANSI_COLOR_B8_BROWN_BG        ,       
  ANSI_COLOR_B8_DARK_GREY_BG    ,    
  ANSI_COLOR_B8_LIGHT_GREY_BG   ,
  ANSI_COLOR_B8_WHITE_BG        ,       
  ANSI_COLOR_B8_RED_BG          ,         
  ANSI_COLOR_B8_ORANGE_BG       ,      
  ANSI_COLOR_B8_YELLOW_BG       ,      
  ANSI_COLOR_B8_GREEN_BG        ,       
  ANSI_COLOR_B8_BLUE_BG         ,        
  ANSI_COLOR_B8_LAVENDER_BG     ,    
  ANSI_COLOR_B8_PINK_BG         ,        
  ANSI_COLOR_B8_LIGHT_PEACH_BG  ,

  // High-intensity (Bright) colors
  ANSI_COLOR_BRIGHT_BLACK   = 90, // Bright Black (Gray)     (128, 128, 128)
  ANSI_COLOR_BRIGHT_RED     ,     // Bright Red              (255, 85, 85)
  ANSI_COLOR_BRIGHT_GREEN   ,     // Bright Green            (85, 255, 85)
  ANSI_COLOR_BRIGHT_YELLOW  ,     // Bright Yellow           (255, 255, 85)
  ANSI_COLOR_BRIGHT_BLUE    ,     // Bright Blue             (85, 85, 255)
  ANSI_COLOR_BRIGHT_MAGENTA ,     // Bright Magenta (Purple) (255, 85, 255)
  ANSI_COLOR_BRIGHT_CYAN    ,     // Bright Cyan             (85, 255, 255)
  ANSI_COLOR_BRIGHT_WHITE   ,     // Bright White            (255, 255, 255)

  ANSI_COLOR_BRIGHT_BLACK_BG   = 100,
  ANSI_COLOR_BRIGHT_RED_BG     ,
  ANSI_COLOR_BRIGHT_GREEN_BG   ,
  ANSI_COLOR_BRIGHT_YELLOW_BG  ,
  ANSI_COLOR_BRIGHT_BLUE_BG    ,
  ANSI_COLOR_BRIGHT_MAGENTA_BG ,
  ANSI_COLOR_BRIGHT_CYAN_BG    ,
  ANSI_COLOR_BRIGHT_WHITE_BG
};

struct EscapeOut{
  u16 _code;
  EscapePAL     _EscapePAL;
	EscapeSeqOpe	_Ope;
  s16 _x;
  s16 _y;
  u8  _attr : 4;  // Holds values 0-15 (commonly used range is 0-9 for terminal attributes)
  AnsiColor _fg;  // Supports ANSI standard colors (0-15 for basic colors)
  AnsiColor _bg;
  u16 _otz;

  void  ResetSetColor(){
    _Ope = ESO_SET_COLOR;
    _fg = ANSI_COLOR_WHITE;
    _bg = ANSI_NULL;
  }

  void  ResetMoveCursor(){
    _Ope = ESO_MOVE_CURSOR;
    _x = _y = 0;
  }

  void  ResetZ(){
    _Ope = ESO_SET_Z;
    _otz = 0;
  }

  inline  void  ResetAll(){
    _code = 0x0000;
    _EscapePAL = PAL_0;
    _Ope = ESO_NONE;
    _x = 0;
    _y = 0;
    _attr = 0;
    _fg = ANSI_COLOR_WHITE;
    _bg = ANSI_NULL;
    _otz = 0;
  }

  EscapeOut(){
    ResetAll();
  }
};

class CEscapeSeqDecoder{
  ImplCEscapeSeqDecoder* _impl;
public:
  EscapeOut& Stream( s32 code_ );
  CEscapeSeqDecoder();
  ~CEscapeSeqDecoder();
};

/**
 * @brief Clears the entire screen.
 * 
 * This function sends an escape sequence to clear the entire screen in the terminal.
 * It uses the `\033[2J` escape sequence to perform this operation.
 * 
 * @param fp A pointer to a FILE stream (typically stdout).
 */
extern void EscClearEntireScreen(FILE* fp);

/**
 * @brief Moves the cursor to a specific position.
 * 
 * This function sends an escape sequence to move the cursor to a specific position
 * in the terminal. It uses the `\033[y;xH` escape sequence where y and x are the
 * row and column coordinates, respectively.
 * 
 * @param fp A pointer to a FILE stream (typically stdout).
 * @param x_ The column number (1-based).
 * @param y_ The row number (1-based).
 */
extern void EscMoveCursor(FILE* fp, s32 x_, s32 y_);
