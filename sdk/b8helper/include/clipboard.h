/**
 * @file clipboard.h
 * @brief Header file for the clipboard library.
 *
 * This header file defines the interface for the clipboard library, which allows
 * writing strings to a clipboard buffer using standard file operations. The clipboard
 * content is updated when the file is closed.
 *
 * Usage:
 * @code
 * #include <clipboard.h>
 * 
 * // Reset clipboard
 * clipboard::Reset();
 * 
 * // Open file for writing to clipboard
 * const char* fname = "/clipboard/con0";
 * FILE* fp_clipboard = fopen(fname, "wt");
 * _ASSERT(fp_clipboard, fname);
 * 
 * fprintf(fp_clipboard, "hello clip\n");
 * 
 * // Nothing is written to the clipboard until fclose() is called
 * fclose(fp_clipboard);
 * @endcode
 */

#pragma once
namespace clipboard {
  void  Reset();
};
