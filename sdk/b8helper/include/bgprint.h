/**
 * @file bgprint.h
 * @brief Header file for the bgprint library.
 *
 * This library provides functionality to display strings on the BG layer 
 * through interfaces such as fprintf for writing strings to files. It includes 
 * features for cursor movement, palette selection, and context management for 
 * buffer output. This allows for formatted text output and manipulation of the 
 * display on a BG layer in a flexible and efficient manner.
 *
 * Usage:
 * Please refer to the sample for "bgprint" located at "./beep8/sdk/app/bgprint/"
 *
 * // Example Usage:
 * {
 *   bgprint::Context ctx;
 *   FILE* fp_bgprint = bgprint::Open(
 *     bgprint::CH3,
 *     nullptr, 256,
 *     ctx
 *   );
 *
 *   // printf
 *   fprintf(fp_bgprint, "Hello!\n" );
 *
 *   // moving cursor
 *   fprintf(fp_bgprint, "\e[2;2H" );
 *
 *   // select bg palette
 *   fprintf(fp_bgprint, "\e[3q SelectPal");
 * 
 *   // clear the entire screen 
 *   fprintf(fp_bgprint,"\e[2J" );
 * }
 */
#include <b8/ppu.h>
#pragma once
namespace bgprint {
  enum EnCmd{
    SET_SLOT_CONTEXT,
    SET_UV_SCROLL,
    EXPORT_PPU_CMD,
    GET_INFO
  };

  enum EnCh {
    CH0,
    CH1,
    CH2,
    CH3,
    CH4,
    CH5,
    CH6,
    CH7,
    /* --- */
    CHMAX
  };

  struct UvScroll {
    int _u = 0;
    int _v = 0;
  };

  /**
   * \brief This structure is used to manage the context of the buffer output.
   * 
   */
  struct Context {
    /**
     * \brief Please specify the beginning of the b8PpuBgTile array for text output.
     *
     * If nullptr is assigned, memory will be automatically allocated from the heap.
     */
    b8PpuBgTile* cpuaddr = nullptr;

    /**
     * \brief Specify the width of the b8PpuBgTile array in powers of 2. 
     * For example, if you specify 4, it will be 2^4=16. Recommended value is 4.
     */
    u16 _w_pow2   = 4;

    /**
     * \brief Specify the height of the b8PpuBgTile array in powers of 2.
     * For example, if you specify 5, it will be 2^5=32. Recommended value is 5.
     */
    u16 _h_pow2   = 5;

    /**
     * \brief Specifies the display area height in decimal units.
     *
     * This value determines the height of the actual visible area on the screen. 
     * It is defined in decimal format, allowing fine control over the display area. 
     * Recommended value is 30.
     */
    u16 _h_disp   = 30;

    /**
     * \brief Indicates whether scrolling is enabled when the text exceeds the display height.
     *
     * If set to true, the text will scroll upward when it reaches the bottom of the display area.
     */
    bool  _scroll = true;
  };

  struct ExportPpuCmd {
    b8PpuCmd* _cmd = nullptr;
    u32 _otz = 0;
  };

  void  Reset();

  /**
   * \brief Open a specified channel, buffer it up to a specified size, and use the context provided
   * 
   * \param ch_     Specifies the channel
   * 
   * \param buff    Specifies the address at the beginning of the stream buffer for FILE*, 
   *                which is secured on the user side of this function. 
   *                Specify nullptr if you want it to be automatically secured.
   *                The size is specified by buffsize.
   * 
   * \param buffsize Buffers the results of fputs(), fwrite(), etc., up to the specified byte size.
   *                 Recommended size is 256. At the very least, please specify a size larger than 1.
   * 
   * \param ctx      Specifies the context. For more information, refer to the description of bgprint::Context.
   * 
   * \return A pointer to FILE structure.
   * 
   */
  FILE* Open(
    EnCh ch_,
    char* buff,
    size_t buffsize,
    bgprint::Context& ctx
  );

  void  SetUvScroll(
    FILE* fp_,
    const bgprint::UvScroll& uv_
  );

  void  Export(
    FILE* fp_,
    const bgprint::ExportPpuCmd& epc
  );

  void  Locate(FILE* fp_ ,s16 lx_,s16 ly_ );
  void  Pal(FILE* fp_, u8 pal_ );
  struct Info {
    s16 _x_locate = 0;   ///< X location.
    s16 _y_locate = 0;   ///< Y location.
    u8  _pal;
  };
  int GetInfo(FILE* fp_, Info& dest);
}
