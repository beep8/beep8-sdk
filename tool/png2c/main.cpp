#include <stdio.h>
#include <ctype.h>
#include <string>
#include <map>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#define PNG_DEBUG 3
#include "png.h"
using namespace std;

static  string _png_filename;
static  int width, height;
static  png_byte color_type;
static  png_byte bit_depth;
static  png_structp png_ptr;
static  png_infop info_ptr;
static  int number_of_passes;
static  png_bytep* row_pointers;

static  map< uint32_t , uint8_t > _map_pal;
static  uint32_t  _tbl_pal[16] = {
  0x000000, 0x1D2B53, 0x7E2553, 0x008751,
  0xAB5236, 0x5F574F, 0xC2C3C7, 0xFFF1E8,
  0xFF004D, 0xFFA300, 0xFFEC27, 0x00E436,
  0x29ADFF, 0x83769C, 0xFF77A8, 0xFFCCAA
};
static  void  init_pal(){
  for( uint8_t ii=0 ; ii<sizeof( _tbl_pal ) / sizeof( _tbl_pal[0] ) ; ++ii ){
    _map_pal[ _tbl_pal[ii] ] = ii;
  }
}

void read_png_file(const char* file_name)
{
  png_byte header[8];

  /* open file and test for it being a png */
  FILE *fp = fopen(file_name, "rb");
  if (!fp)
          throw string("[read_png_file] File" ) + file_name + "could not be opened for reading";
  fread(header, 1, 8, fp);
  if (png_sig_cmp(header, 0, 8))
          throw string("[read_png_file] File ") + file_name + " is not recognized as a PNG file";

  /* initialize stuff */
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
          throw string("[read_png_file] png_create_read_struct failed");

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
          throw string("[read_png_file] png_create_info_struct failed");

  if (setjmp(png_jmpbuf(png_ptr)))
          throw string("[read_png_file] Error during init_io");

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);
  width = png_get_image_width(png_ptr, info_ptr);
  height = png_get_image_height(png_ptr, info_ptr);
  color_type = png_get_color_type(png_ptr, info_ptr);
  bit_depth = png_get_bit_depth(png_ptr, info_ptr);
  number_of_passes = png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);

  /* read file */
  if (setjmp(png_jmpbuf(png_ptr)))
          throw string("[read_png_file] Error during read_image");

  row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (int y=0; y<height; y++)
          row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

  png_read_image(png_ptr, row_pointers);
  fclose(fp);
}

int main(int argc, char *argv[]){
  if( argc < 2){
    printf( "%s test.png\n" , argv[0] );
    return 0;
  }
  init_pal();

  try {
    _png_filename = std::string( argv[1] );
    read_png_file( _png_filename.c_str() );
    size_t pos_sep = _png_filename.find_last_of("/\\");
    std::string filename_with_ext = (pos_sep == std::string::npos)
        ? _png_filename
        : _png_filename.substr(pos_sep + 1);
    size_t pos_dot = filename_with_ext.find_last_of('.');
    std::string filename = (pos_dot == std::string::npos)
        ? filename_with_ext
        : filename_with_ext.substr(0, pos_dot);

    if( width  & 7 )   throw string( "width isn't multipe of 8" );
    if( height & 7 )  throw string( "height isn't multipe of 8" );

    if( width  > 512 ) throw string( "width is greater than 512" );
    if( height > 512 ) throw string( "height is greater than 512" );

    if( width  < 8 )  throw string( "width is less than 8");
    if( height < 8 )  throw string( "height is less than 8");

    if( bit_depth != 8 )  throw string( "bit width isn't 8");

    switch( color_type ){
      case  PNG_COLOR_TYPE_GRAY:
        //(bit depths 1, 2, 4, 8, 16)
        throw string("PNG_COLOR_TYPE_GRAY not supported");
        break;
      case  PNG_COLOR_TYPE_GRAY_ALPHA:
        //(bit depths 8, 16)
        throw string("PNG_COLOR_TYPE_GRAY_ALPHA not supported");
        break;
      case  PNG_COLOR_TYPE_PALETTE:
        //(bit depths 1, 2, 4, 8)
        throw string("PNG_COLOR_TYPE_PALETTE not supported");
        break;
      case  PNG_COLOR_TYPE_RGB:
      case  PNG_COLOR_TYPE_RGB_ALPHA:
        // pass
        break;
    }

    printf( "// exported by png2c %s\n", _png_filename.c_str() );
    printf( "#include <stdint.h>\n" );
    printf( "extern	const uint16_t b8_image_%s_width =%d;\n",filename.c_str(),width);
    printf( "extern	const uint16_t b8_image_%s_height=%d;\n",filename.c_str(),height);
    printf( "extern	const uint8_t  b8_image_%s[ (%d*%d)>>1 ] = {\n" , filename.c_str(), width , height);

    for(int yy=0 ; yy < height ; ++yy){
      uint8_t pix[2];
      png_bytep row = row_pointers[ yy ];
      for( int xx=0 ; xx<width ; ++xx ){

        uint32_t rgb32 = 0x000000;
        rgb32 |= *row++; rgb32 <<= 8;
        rgb32 |= *row++; rgb32 <<= 8;
        rgb32 |= *row++;

        switch( color_type ){
          case  PNG_COLOR_TYPE_RGB:
          break;
          case  PNG_COLOR_TYPE_RGB_ALPHA:
            ++row;
          break;
          default:
            throw string("unknown color type");
            break;
        }

        if( _map_pal.find( rgb32 ) != _map_pal.end() ){
          pix[xx&1] = _map_pal[ rgb32 ];
        } else {
          throw string( "found unknown 32bpp color" );
        }
        if( xx&1 ) printf( "0x%02x," , (pix[0]<<4)|pix[1] );
      }
      printf("\n");
    }

    printf("};\n");
    printf(
      "extern \"C\" const uint8_t* b8_image_%s_get( uint16_t* width, uint16_t* height ){\n"
      "  *width  = b8_image_%s_width;"
      "  *height = b8_image_%s_height;"
      "  return  b8_image_%s;\n"
      "}\n",
      filename.c_str(),
      filename.c_str(),
      filename.c_str(),
      filename.c_str()
    );
  } catch( string str ){
    fprintf( stderr, "%s error=%s\n",argv[0],str.c_str());
    return EXIT_FAILURE;
  }
  return 0;
}
