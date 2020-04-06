// hairMain.cpp : Defines the entry point for the console application.

#include <cstdlib>
#include <cstdio>

#include <DBPF.h>
#include <DBPFcompress.h>
#include <DBPF_types.h>
#include <DBPF_3IDR.h>
#include <DBPF_GZPS.h>
#include <DBPF_XHTN.h>
#include <DBPF_TXMT.h>
#include <DBPF_TXTR.h>
#include <DBPF_STR.h>


extern bool hairProcess( const char * filename, const int hairColor, const char * strFamily, const char * strHatTXTR );


int main( int argc, char * argv[])
{
  printf( "argc %i\n", argc );
  for( int i = 0; i < argc; ++i )
    printf( "argv[%i] %s\n", i, argv[i] );

  // incorrect parameters
  if( argc < 5 )
  { fprintf( stderr, "usage: %s filename hairColor family hatTXTRref\n", argv[0] );
    return 1;
  }

  // hair color

  int hairColor = 0;
  if( 0 == strcmp( "black", argv[2] ) )
    hairColor = HAIR_BLACK;
  else if( 0 == strcmp( "brown", argv[2] ) )
    hairColor = HAIR_BROWN;
  else if( 0 == strcmp( "blond", argv[2] ) )
    hairColor = HAIR_BLOND;
  else if( 0 == strcmp( "red", argv[2] ) )
    hairColor = HAIR_RED;
  else
  { fprintf( stderr, "ERROR: invalid hair color, %s\n", argv[2] );
    return 1;
  }

  // process file

  hairProcess( argv[1], hairColor, argv[3], argv[4] );

	return 0;
}

