#include <tokenizer.h>
#include <b8/assert.h>

void  MacroDict::DumpAll()  const {
  printf("MacroDict::DumpAll(){\n");
  for( std::pair<str8, const char*> it : _dict ){
    printf("  [%s]=[%s]\n",it.first.c_str(),it.second);
  }
  printf("}\n");
}

const char* MacroDict::Get( const char* key ) const {
  decltype( _dict )::const_iterator it = _dict.find( key );
  if( it == _dict.end() ){
    printf( "unknown macro [%s]\n",key );
    DumpAll();
    _ASSERT( it != _dict.end() , "not found macro" );
  }
  return it->second;
}

MacroDict::MacroDict( size_t num_of_macro , const Macro* macros ){
  for( size_t nn=0 ; nn<num_of_macro ; ++nn,++macros ){
    _dict[ str8(macros->_key) ] = macros->_data;
  }
}

str16 CTokenizer::GetString( const char* lhs_ , const char* def_ ){
  str16 lhs( lhs_ );
  for( CFormula formula : _formulas ){
    if( lhs == formula._lhs ){
      return formula._rhs;
    }
  }
  return str16( def_ );
}

s32   CTokenizer::GetNumber( const char* lhs_ , s32 def_ ){
  str16 lhs( lhs_ );
  for( CFormula formula : _formulas ){
    if( lhs == formula._lhs ){
      if( formula._rhs.empty() )  return def_;
      return std::atoi( formula._rhs.c_str() );
    }
  }
  return def_;
}

CTokenizer::CTokenizer( const char* sz , const MacroDict* dict_ ) {
  CFormula formula;
  CTokenizer::State sts = CTokenizer::STS_LHS;

  if( dict_ && sz && *sz=='$' ){  // sz       = "$alias_name"
    const char* szmacro = sz+1;   // szmacro  = "alias_name"
    sz = dict_->Get( szmacro );
  }

  while( *sz ){
    if( *sz == ' '){
      ++sz;
      continue;
    }

    switch( sts ){
      case  STS_LHS:{
        if( isalpha( *sz ) || *sz == '_' ){
          formula._lhs.push_back( *sz );
        } else if ( *sz == '=' ){
          sts = STS_RHS;
        }
      }break;

      case  STS_RHS:{
        if( isdigit( *sz ) || isalpha( *sz ) || *sz == '.' || *sz == '_' || *sz == '-' || *sz == '+' ){
          formula._rhs.push_back( *sz );
        } else if ( *sz == ';' || *sz == ',' ){
          _formulas.push_back( formula );
          formula.Clear();
          sts = STS_LHS;
        }
      }break;
    }
    ++sz;
  }

  if( formula._lhs.empty() == false && formula._rhs.empty() == false ){
    _formulas.push_back( formula );
  }
}
