/**
 * file: DBPF_types.cpp
 * author: CatOfEvilGenius
 *
 * type constants such as DBPF_TXTR, DBPF_BHAV, etc,
 * and routines for converting type numbers to strings
**/

#include <cstdlib>
#include <cstdio>
#include "DBPF_types.h"


/**
<pre>
 * input:   rType - hex code for a DBPF resource type
 * output:  str - string, type name, ie. TXMT, ANIM, DIR,
 *          if unknown type, the string will be a hex number,
 *          str must be preallocated and at least 13 characters
 * returns: none
</pre>
**/
void dbpfResourceTypeToString( const unsigned int rType, char * str )
{
  switch( rType )
  {
    case DBPF_DIR:  sprintf( str, "DIR " ); break;
    case DBPF_TXTR: sprintf( str, "TXTR" ); break;
    case DBPF_TXMT: sprintf( str, "TXMT" ); break;
    case DBPF_GZPS: sprintf( str, "GZPS" ); break;
    case DBPF_BINX: sprintf( str, "BINX" ); break;
    case DBPF_3IDR: sprintf( str, "3IDR" ); break;
    case DBPF_XHTN: sprintf( str, "XHTN" ); break;
    case DBPF_STR:  sprintf( str, "STR#" ); break;
    case DBPF_CATS: sprintf( str, "CATS" ); break;
    case DBPF_CTSS: sprintf( str, "CTSS" ); break;
    case DBPF_TTAs: sprintf( str, "TTAs" ); break;
    case DBPF_TTAB: sprintf( str, "TTAB" ); break;
    case DBPF_LIFO: sprintf( str, "LIFO" ); break;
    case DBPF_GMDC: sprintf( str, "GMDC" ); break;
    case DBPF_GMND: sprintf( str, "GMND" ); break;
    case DBPF_SHPE: sprintf( str, "SHPE" ); break;
    case DBPF_CRES: sprintf( str, "CRES" ); break;
    case DBPF_GLOB: sprintf( str, "GLOB" ); break;
    case DBPF_BHAV: sprintf( str, "BHAV" ); break;
    case DBPF_ANIM: sprintf( str, "ANIM" ); break;
    case DBPF_FACE: sprintf( str, "FACE" ); break;
    case DBPF_PDAT: sprintf( str, "PDAT" ); break;
    case DBPF_FAMI: sprintf( str, "FAMI" ); break;
    case DBPF_OBJD: sprintf( str, "OBJD" ); break;
    case DBPF_OBJF: sprintf( str, "OBJF" ); break;
    case DBPF_XOBJ: sprintf( str, "XOBJ" ); break;
    case DBPF_WaFl: sprintf( str, "WaFl" ); break;
    case DBPF_HOUS: sprintf( str, "HOUS" ); break;
    case DBPF_ROOF: sprintf( str, "ROOF" ); break;
    case DBPF_WLAY: sprintf( str, "WLAY" ); break;
    case DBPF_FPL:  sprintf( str, "PFL " ); break;
    case DBPF_POOL: sprintf( str, "POOL" ); break;
    case DBPF_LOT:  sprintf( str, "LOT " ); break;
    case DBPF_DESC: sprintf( str, "DESC" ); break;
    case DBPF_NHTR: sprintf( str, "NHTR" ); break;
    case DBPF_NHTG: sprintf( str, "NHTG" ); break;
    case DBPF_WRLD: sprintf( str, "WRLD" ); break;
    case DBPF_FWAV: sprintf( str, "FWAV" ); break;
    default: sprintf( str, "%x", rType ); break;
  };
}


/**
<pre>
 * given a hex code for a DBPF resource type,
 * print a string with the type's name (no newline)
</pre>
**/
void dbpfPrintResourceType( const unsigned int rType )
{
  char str[20]; str[19] = '\0';
  dbpfResourceTypeToString( rType, str );
  printf( "%s", str );
}


bool operator==( const DBPF_TGIRtype & _left, const DBPF_TGIRtype & _right )
{
  if( _left.muTypeID != _right.muTypeID )
    return false;

  if( _left.muGroupID != _right.muGroupID )
    return false;

  if( _left.muInstanceID != _right.muInstanceID )
    return false;

  if( _left.muResourceID != _right.muResourceID )
    return false;

  return true;
}


bool operator!=( const DBPF_TGIRtype & _left, const DBPF_TGIRtype & _right )
{
  if( _left == _right )
    return false;
  return true;
}
