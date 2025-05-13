#include <stdio.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <b8/assert.h>
#include "esc_decoder.h"
#include <cstr.h>
#include <b8/sys.h>
#include <b8/misc.h>
using namespace std;

#define ATTR_RESET  (0)

enum	EscState
{
  ES_IDLE,
  ES_0x1b,  // esc start (=0x1b)
  ES_0x5b,  // '['
  ES_0x3e,  // '>' remove cursor
  ES_0x3f,  // '?' Private CSI
  ES_0x4f,  // '0'
};

class ImplCEscapeSeqDecoder{
  friend  class CEscapeSeqDecoder;
  EscState  _EscState = ES_IDLE;
  string    _Str_0x5b;
  bool      _bReverse = false;
  AnsiColor   _FrontColor = ANSI_COLOR_WHITE;
  AnsiColor   _BackColor  = ANSI_COLOR_BLACK;
  vector<int> params;
  EscapeOut eout;

  const vector<int>& parseParams(const char* x_) {
    params.clear();
    int currentParam = 0;
    char prevChar = '\0';

    while (*x_) {
      char c = *x_++;
      switch (c) {
        case '0' ... '9':
          currentParam = currentParam * 10 + (c - '0');
          break;
        case ';':
          params.push_back(currentParam);
          currentParam = 0;
          break;
        default:
          break;
      }
      prevChar = c;
    }

    if (currentParam != 0 || prevChar == '0') {
        params.push_back(currentParam);
    }
    return params;
  }

  ImplCEscapeSeqDecoder(){
    params.reserve(5);
    _Str_0x5b.reserve(32);
  }
};

CEscapeSeqDecoder::CEscapeSeqDecoder(){
  _impl = new ImplCEscapeSeqDecoder;
}

CEscapeSeqDecoder::~CEscapeSeqDecoder(){
  delete _impl;
}

EscapeOut& CEscapeSeqDecoder::Stream( s32 code_ ){
  EscapeOut& eout = _impl->eout;
  eout._code = 0x0000;
  eout._Ope = ESO_NONE;

  if( ASCII_ESC == code_ ) {
    _impl->_EscState = ES_0x1b;
  }

	switch( _impl->_EscState ) {
    case	ES_IDLE:{
      _impl->_Str_0x5b.clear();
      eout._code = code_;
      eout._Ope = ESO_ONE_CHAR;
    }break;
    case	ES_0x1b:{// [ESC]
      switch( code_ ) {
        case	'[':	// '['
          _impl->_EscState = ES_0x5b;
          _impl->_Str_0x5b.clear();
          break;
        case	'H':
          _impl->_EscState = ES_IDLE;
          break;
        case	0x4f:	// cursor moving
          _impl->_EscState = ES_0x4f;
          break;
        // The escape sequences are defined in the HP-71B Reference Manual on page 328, and in the 82163A manual on page 10
        case	'<':  // turns the cursor off
        case	'>':  // turns the cursor on
        case	'Q':  // sets the "insert cursor", a blinking left arrow
        case	'R':  // sets the "replace cursor", a blinking block
          _impl->_EscState = ES_IDLE;
          break;
      }
    }break;
    case  ES_0x3f:{ // '?' Private CSI
      switch( code_ ) {
        case 'm':{
          const auto& params = _impl->parseParams( _impl->_Str_0x5b.c_str() );
          if( ! params.empty() ){
            switch( params[0] ){
              case 101: eout._Ope = ESO_ENABLE_SHADOW;  break;
              case 100: eout._Ope = ESO_DISABLE_SHADOW; break;
              default:break;
            }
          } else {
            eout._Ope = ESO_DISABLE_SHADOW;
          }

          _impl->_EscState = ES_IDLE;
					_impl->_Str_0x5b.clear();
        }break;
				default:
					_impl->_Str_0x5b.push_back( code_ );
					break;
      }
    }break;
    case	ES_0x5b:{ // '['
      switch( code_ ) {
        case	'>':
          _impl->_EscState = ES_0x3e;
          break;

        /*
          Esc[0q 	select PAL0
          Esc[1q 	select PAL1
          Esc[2q 	select PAL2
          Esc[3q 	select PAL3
        */
        case	'q':{
          char* endptr;
          int pal = strtol(_impl->_Str_0x5b.c_str(), &endptr,0);
          _ASSERT( pal>=0 && pal<= 3, "INVALID PAL");
          eout._Ope = ESO_SEL_PAL;
          eout._EscapePAL = (EscapePAL)pal;
          _impl->_EscState = ES_IDLE;
        }break;

        case	'J':
          if( 1 == _impl->_Str_0x5b.size() ){
            switch( _impl->_Str_0x5b.at(0) ){
              //Esc[0J 	Clear screen from cursor down 	ED0
              case '0':
                eout._Ope = ESO_CLEAR_SCREEN_FROM_CURSOR_DOWN;
                break;
              //Esc[1J 	Clear screen from cursor up 	ED1
              case '1':
                eout._Ope = ESO_CLEAR_SCREEN_FROM_CURSOR_UP;
                break;
              //Esc[2J 	Clear entire screen 	ED2
              case '2':
                eout._Ope = ESO_CLEAR_ENTIRE_SCREEN;
                break;
            }
          }
          _impl->_EscState = ES_IDLE;
          break;

				case	'K':
          if( 1 == _impl->_Str_0x5b.size() ){
            switch( _impl->_Str_0x5b.at(0) ){
		          // Esc[0K 	Clear line from cursor right 	EL0
              case '0':
                eout._Ope = ESO_CLEAR_LINE_FROM_CURSOR_RIGHT;
                break;
		          // Esc[1K 	Clear line from cursor left 	EL1
              case '1':
                eout._Ope = ESO_CLEAR_LINE_FROM_CURSOR_LEFT;
                break;
              // Esc[2K 	Clear entire line 	EL2
              case '2':
                eout._Ope = ESO_CLEAR_ENTIRE_LINE;
                break;
            }
          }
					_impl->_EscState = ES_IDLE;
					break;

        case	'?':	// '?'
          _impl->_EscState = ES_0x3f;
          _impl->_Str_0x5b.clear();
          break;

        case	'r':
          _impl->_EscState = ES_IDLE;
          break;

        // .ex "[5z"
        case	'z':{
          eout.ResetZ();
          const auto& params = _impl->parseParams( _impl->_Str_0x5b.c_str() );
          if( ! params.empty() )  eout._otz = params[0];

          _impl->_EscState = ES_IDLE;
					_impl->_Str_0x5b.clear();
        }break;

        case	'm':{
          const auto& params = _impl->parseParams( _impl->_Str_0x5b.c_str() );
          if( params.empty() ){
            eout.ResetSetColor();
          } else {
            for( auto param : params ){
              switch( param ){
                case  0:
                  eout.ResetSetColor();
                  break;
                case  ANSI_COLOR_BLACK     ... ANSI_COLOR_WHITE:
                case  ANSI_COLOR_B8_BLACK  ... ANSI_COLOR_B8_LIGHT_PEACH:
                case  ANSI_COLOR_BRIGHT_BLACK ... ANSI_COLOR_BRIGHT_WHITE:
                  eout._Ope = ESO_SET_COLOR;
                  eout._fg = static_cast< AnsiColor >( param );
                  break;

                case  ANSI_COLOR_BLACK_BG         ... ANSI_COLOR_WHITE_BG:
                case  ANSI_COLOR_BRIGHT_BLACK_BG  ... ANSI_COLOR_BRIGHT_WHITE_BG:
                  eout._Ope = ESO_SET_COLOR;
                  eout._bg = static_cast< AnsiColor >( param-10 );
                  break;

                case  ANSI_COLOR_B8_BLACK_BG      ...  ANSI_COLOR_B8_LIGHT_PEACH_BG:
                  eout._Ope = ESO_SET_COLOR;
                  eout._bg = static_cast< AnsiColor >( param-20 );
                  break;
              }
            }
          }
          
          _impl->_EscState = ES_IDLE;
					_impl->_Str_0x5b.clear();
				}break;	// ';' or 'm'

				case	'H': {
          eout.ResetMoveCursor();
          const auto& params = _impl->parseParams( _impl->_Str_0x5b.c_str() );
          if( params.size() >= 1 )  eout._y = params[0];
          if( params.size() >= 2 )  eout._x = params[1];
					_impl->_EscState = ES_IDLE;
					_impl->_Str_0x5b.clear();
				}break;

				case	ASCII_DEL:	// = 0x7e = '~'
					_impl->_EscState = ES_IDLE;
					_impl->_Str_0x5b.clear();
					eout._Ope = ESO_DEL;
					break;

				default:
					_impl->_Str_0x5b.push_back( code_ );
					break;
			}
		}break;

    default: break;
  }
  return eout;
}

void  EscClearEntireScreen( FILE* fp ){
  if(!fp) return;
  fprintf(fp, "\033[2J" );
}

void  EscMoveCursor(FILE*fp,s32 x_,s32 y_){
  if(!fp) return;
  fprintf(fp,"\033[%d;%dH" ,y_,x_);
}
