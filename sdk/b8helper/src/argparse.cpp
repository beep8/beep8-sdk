// MIT License

// Copyright (c) 2022 ZooK

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <argparse.h>

using namespace std;
using namespace argparse;

static  bool IsNumber(const string& s)
{
  return std::ranges::all_of(s.begin(), s.end(),
    [](char c){ return isdigit(c) != 0 || c == '-'; });
}

static  bool IsString(const string& s)
{
  return std::ranges::all_of(s.begin(), s.end(),
    [](char c){
      switch( c ){
        case  'a' ... 'z':
        case  'A' ... 'Z':
        case  '_':
        case  '-':
          return true;
        default:
          return false;
      }
    });
}

void  Argument::ShowHelp() const {
  printf( "  %s, %-16s%s\n" , _short_alias.c_str() , _name.c_str() , _help.c_str() );
}

ArgumentParser::Result ArgumentParser::ParseArgs( const std::vector< std::string >& args ){
    _showed_help = false;
    _cur_name.clear();
    _vars.clear();
    for( const auto& it : args ){
      // name ?
      if( it.size() >= 3 && it.at(0) == '-' && it.at(1) == '-' ){
        Argument dest;
        if( _SearchName( it.c_str() ,dest ) ){
          auto result = _Action( dest , it );
          if( result != OK )  return result;
        } else {
          fprintf( stderr , "unknown option %s\n" , it.c_str() );
          return UNKNOWN_FLAG;
        }

      // short_alias_ ?
      } else if ( it.size() == 2 && it.at(0) == '-' && std::isdigit( it.at(1) ) == false ){
        Argument dest;
        if( _SearchShortAlias( it.c_str() ,dest ) ){
          auto result = _Action( dest, dest._name.c_str() );
          if( result != OK )  return result;
        } else {
          fprintf( stderr , "unknown option %s\n" , it.c_str() );
          return UNKNOWN_FLAG;
        }

      // number ?
      } else if( IsNumber( string( it.c_str() ) ) ){
        if( _vars.find( _cur_name ) != _vars.end() ){
          _vars[ _cur_name ]._val = std::atoi( it.c_str() );
        }

      // string ?
      } else if ( IsString( string( it.c_str() ) ) ){
        if( _vars.find( _cur_name ) != _vars.end() ){
          _vars[ _cur_name ]._str = it.c_str();
        }
      }
    }
    return OK;

}

void  ArgumentParser::ShowHelp(){
  if( _showed_help )  return;
  printf( "optional arguments:\n" );
  for( const auto& it : _args ){
    it.ShowHelp();
  }
  _showed_help = true;
}

int ArgumentParser::GetVarInt( std::string name_ ,  int if_not_found_ ){
  if( _vars.find( name_ ) == _vars.end() )  return if_not_found_;
  return  _vars[ name_ ]._val;
}

std::string ArgumentParser::GetVarStr( std::string name_ ,  std::string if_not_found_ ){
  if( _vars.find( name_ ) == _vars.end() )  return if_not_found_;
  return  _vars[ name_ ]._str;
}

bool  ArgumentParser::_SearchName( std::string name_ , Argument& dest_ ){
  for( const auto& it : _args ){
    if( it._name == name_ ){
      dest_ = it;
      return true;
    }
  }
  return false;
}

bool  ArgumentParser::_SearchShortAlias( std::string short_alias_ , Argument& dest_ ){
  for( const auto& it : _args ){
    if( it._short_alias == short_alias_ ){
      dest_ = it;
      return true;
    }
  }
  return false;
}

int   ArgumentParser::AddArgument(
  std::string name_,
  std::string short_alias_,
  std::string help_,
  Argument::Action action_
){
  if( name_.size() < 4 )    return BAD_PARAM;
  if( name_.at(0) != '-' )  return BAD_PARAM;
  if( name_.at(1) != '-' )  return BAD_PARAM;
  if( std::isdigit( name_.at(2) ) ) return BAD_PARAM;

  if( short_alias_.size() != 2 )  return BAD_PARAM;
  if( short_alias_.at(0) != '-' ) return BAD_PARAM;
  if( std::isdigit( short_alias_.at(1) ) )  return BAD_PARAM;

  Argument arg;
  arg._name         = name_;
  arg._short_alias  = short_alias_;
  arg._help         = help_;
  arg._action       = action_;
  _args.push_back( arg );
  return OK;
}

ArgumentParser::Result  ArgumentParser::_Action( const Argument& dest , const std::string& it_ ){
  _cur_name = it_.c_str();
  Var var;
  if( dest._action == Argument::store_true ){
    var._val = 1;
  } else if( dest._action == Argument::store_false ){
    var._val = 0;
  } else if( dest._action == Argument::show_help ){
    ShowHelp();
    return SHOWED_HELP;
  }
  _vars[ _cur_name ] = var;

  return OK;
}

ArgumentParser::ArgumentParser(){
  AddArgument( "--help", "-h", "show this help message and exit" , Argument::show_help );
}
