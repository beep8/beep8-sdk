#include "shell.h"
#include <b8/hif.h>
#include <trace.h>
#include <vector>
#include <cstr.h>
#include <map>
#include "shell.h"
#include <b8/hif.h>
#include <b8/misc.h>
using namespace std;

#define N_LINEBUFF  (0x80)
class CHelpShellCmd : public CShellCmd{
  CShellObj* _shell;
  int vOnMain( const std::vector< str16 >& tokens ){
    (void)tokens;
    _shell->_no_history_registration = true;
    vector< str16 > commands = _shell->GetAvailableCommands();
    for( const str16& command : commands ){
      if( command == "help" ) continue;

      printf( "\n%s:\n",command.c_str() );

      cstr< N_LINEBUFF > linebuff;

      linebuff = command.c_str();
      linebuff += " --help";

      _shell->_SetLineBuff( linebuff.c_str() );
      _shell->_Parse();
    }
    printf( "\n" );
    return 0;
  }
public:
  CHelpShellCmd( CShellObj* shell_ )
  : CShellCmd( "help" )
  {
    _shell = shell_;
  }
};

class CClearShellCmd : public CShellCmd{
  int  vOnMain( const std::vector< str16 >& tokens ){
    if( ShowHelp( tokens , "clear console\n") ){
      return 0;
    }
    printf("%c",ASCII_FF);
    return 0;
  }
public:
  CClearShellCmd()
  : CShellCmd( "clear" )
  {}
};

class ImplCShellObj{
  friend  class CShellObj;
  cstr< N_LINEBUFF > linebuff_history;
  cstr< N_LINEBUFF > linebuff;
  map< str16 , CShellCmd* , cmp_str > _map_cmd;

  void  ClearLineBuff(){
    linebuff.clear();
  }

  ImplCShellObj(){
    ClearLineBuff();
  }

  ~ImplCShellObj(){
    for (const auto& [name, value] : _map_cmd ){
      delete value;
    }
  }
};

void  CShellObj::Prompt(){
  printf( "bp8> ");
}

void  CShellObj::vOnStep(){
    int kcode = getchar();
  if( EOF  != kcode ){
    Input( kcode );
  }
}

void  CShellObj::ExecShellScript( std::shared_ptr< std::string > script_ ){
  string line;
  for( const auto& it : *script_ ){
    if( it == B8_HIF_KB_CODE_ENTER ){
      if( ! line.empty() ){
        _SetLineBuff( line.c_str() );
        _Parse();
      }
      line.clear();
    } else {
      line.push_back( it );
    }
  }
}

void  CShellObj::Input( u32 keycode_ ){
  keycode_ &= 0xffff;
  switch( keycode_ ){
    case  B8_HIF_KB_CODE_ENTER:{
      printf("\n");
      int retval = _Parse();
      if( 0 == retval && _no_history_registration == false ){
        if( ! _impl->linebuff.empty() ) _impl->linebuff_history = _impl->linebuff;
      }
      _no_history_registration = false;
      Prompt();
      _impl->ClearLineBuff();
    }break;
    case  B8_HIF_KB_CODE_ARROW_UP:
      printf("%c\n",ASCII_BS);
      _impl->linebuff = _impl->linebuff_history;
      Prompt();
      printf( "%s" , _impl->linebuff.c_str());
      break;

    case  '"': case  '!': case  '#': case  '$': case  '%':
    case  '&': case  '\'':
    case  '(': case  ')': case  '=': case  '-':
    case  '|': case  '^': case  '~': case  '/': case  '_':
    case  '[': case  ']':
    case  ';': case  ':':
    case  '{': case  '}':
    case  '*': case  '@': case  '?':
    case  '<': case  '>':
    case  '0'...'9':
    case  'a'...'z':
    case  'A'...'Z':
    case  ' ':
    case  '.':
    {
      if( _impl->linebuff.size()+1 < N_LINEBUFF ){
        _impl->linebuff.push_back( keycode_ );
        printf("%c",(int)keycode_);
      }
    }break;

    case  ASCII_FF:{
      printf("%c",ASCII_FF);
      Prompt();
    }break;

    case  ASCII_ETX:{
      printf("\n");
      Prompt();
    }break;

    case  ASCII_BS:{
      printf("%c",ASCII_BS);
      _impl->linebuff.pop_back();
      printf("\n");
      Prompt();
      printf("%s", _impl->linebuff.c_str());
    }break;
    default:
      //printf("shell keycode_ = 0x%08x",keycode_);
      break;
  }
}

std::vector< str16 >  CShellObj::GetAvailableCommands(){
  std::vector< str16 > commands;
  for (const auto& [name, value] : _impl->_map_cmd ){
    commands.push_back( name );
  }
  return  commands;
}

CShellObj::CShellObj(){
  _impl = new ImplCShellObj;
  Register( new CHelpShellCmd( this ) );
  Register( new CClearShellCmd() );
  Prompt();
}

CShellObj::~CShellObj(){
  delete _impl;
}

void  CShellObj::_SetLineBuff( const char* sz ){
  _impl->linebuff = sz;
}

int  CShellObj::_Parse(){
  str16 token;
  vector< str16 > tokens;

  const char* pp = _impl->linebuff.c_str();

  while( true ){
    while( *pp == ' ' ) ++pp;
    if( *pp == 0x00 ) break;

    while( *pp && *pp != ' ' ){
      token.push_back(*pp++);
    }
    if( token.size()>0 ){
      tokens.push_back( token );
      token.clear();
    }
    if( *pp == 0x00 ) break;
  }

  if( tokens.empty() )  return 0;

  if( _impl->_map_cmd.find( tokens[0] ) == _impl->_map_cmd.end() ){
    printf( "command not found : %s\n" , tokens[0].c_str() );
    return 1;
  }

  CShellCmd* cmd = _impl->_map_cmd[ tokens[0] ];
  int retval = cmd->Main( tokens );
  if( retval > 0 ){
    printf( "'%s' returns err code : %d\n" , tokens[0].c_str(), retval );
  }
  return  retval;
}

void  CShellObj::Register( CShellCmd* cmd_ ){
  if( !cmd_ ) return;

  if( _impl->_map_cmd.find( cmd_->_name ) != _impl->_map_cmd.end() ){
    printf( "[%s] has already been registered.\n",cmd_->_name.c_str() );
    return;
  }

  _impl->_map_cmd[ cmd_->_name ] = cmd_;
  cmd_->_pShellObj = this;
}

CShellCmd::CShellCmd( const char* name_ ){
  _name = name_;
}

bool  CShellCmd::ShowHelp( const std::vector< str16 >& tokens , const char* help ){
  for( size_t nn=1 ; nn<tokens.size() ; ++nn ){
    if( tokens.at(nn) == "--help" ){
      printf( help );
      return true;
    }
  }
  return false;
}
