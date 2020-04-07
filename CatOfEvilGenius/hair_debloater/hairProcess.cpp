// hairProcess.cpp

#include <cstdlib>
#include <cstdio>

#include "DBPF.h"
#include "DBPFcompress.h"
#include "DBPF_types.h"
#include "DBPF_3IDR.h"
#include "DBPF_GZPS.h"
#include "DBPF_XHTN.h"
#include "DBPF_TXMT.h"
#include "DBPF_TXTR.h"
#include "DBPF_STR.h"

void removeNullResources( vector< DBPF_resourceType * > & resources )
{
  if( true == resources.empty() )
    return;

  vector< DBPF_resourceType * > tmp;
  DBPF_resourceType * pResource = NULL;

  int nullCount = 0;
  for( int i = 0; i < (int)resources.size(); ++i )
  {
    pResource = resources[i];
    if( pResource != NULL )
    {
      tmp.push_back( pResource );
    }
    else
      ++nullCount;
  }

  resources.clear();

  for( int i = 0; i < (int)tmp.size(); ++i )
    resources.push_back( tmp[i] );

#ifdef _DEBUG
  printf( "\nremoved %i resources\n\n", nullCount );
#endif
}



bool hairProcess( const char * filename, const int hairColor, const char * strFamily, const char * strHatTXTR )
{
//  bool bFileWasChanged = false;

  // open package file, read resources

  DBPFtype package;
  vector< DBPF_resourceType * > resources;

  vector< unsigned int > typesToInit;
  typesToInit.push_back( DBPF_GZPS );
  typesToInit.push_back( DBPF_XHTN );
  typesToInit.push_back( DBPF_TXMT );
  typesToInit.push_back( DBPF_TXTR );
  typesToInit.push_back( DBPF_3IDR );

  if( false == readPackage( filename, package, typesToInit, resources ) )
    return false;

// -----------------------------------------------------------------------------

  // name of Maxis reference hair texture, without the color at the end
  //   and number of hair alpha groups
//  char strMaxisHairTexture[256]; strcpy( strMaxisHairTexture, "ufhairlongsimple-" );
//  const int numAlphaGroups = 2;  // hairlongsimple
  char strMaxisHairTexture[256]; strcpy( strMaxisHairTexture, "ufhairlong-" );
  const int numAlphaGroups = 8;  // hairlong

  // bin hair,
  // familify hair
  // -------------------------

  unsigned int itemCount = (unsigned int)(resources.size());
  DBPF_resourceType * pResource = NULL;

  for( unsigned int k = 0; k < itemCount; ++k )
  {
    pResource = resources[k];
    if( NULL == pResource )
      continue;

    if( DBPF_XHTN == pResource->getType() )
    {
      ((DBPF_XHTNtype*)pResource)->setHairColor( hairColor );
      ((DBPF_XHTNtype*)pResource)->setFamily( strFamily );

      DBPF_CPFitemType item;
      item.miType = CPF_INT;
      item.miValue = 0x5e; // age groups, all but toddler
      ((DBPF_XHTNtype*)pResource)->setPropertyValue( "age", item );
    }
    else if( DBPF_GZPS == pResource->getType() )
    {
      // grey hair should be set to grey
      int color = hairColor;
      unsigned int age = ((DBPF_GZPStype*)pResource)->getAge();
      if( 0x10 == age )
        color = HAIR_GREY;

      ((DBPF_GZPStype*)pResource)->setHairColor( color );
      ((DBPF_GZPStype*)pResource)->setFamily( strFamily );
    }
  }


  // remove unnecessary TXMTs
  // ----------------------------
  // keep these:
  //   afhair_Casual1~hair_txmt
  //   afhair_Casual1~hairalpha11_txmt
  //   afhair_Casual1~frame_txmt
  //   afhair_Casual1~lens_txmt
  // and for black hair, keep greys
  //   efhair_Casual1~hair_txmt
  //   efhair_Casual1~hairalpha11_txmt

  DBPF_TGIRtype tgirHairA, tgirHairAlphaA, // adult hair TXMT
                tgirHairE, tgirHairAlphaE, // elder hair TXMT
                tgirFrame, tgirLens;       // hat TXMT

  bool bHaveHairA = false, bHaveHairAlphaA = false,
       bHaveHairE = false, bHaveHairAlphaE = false,
       bHaveFrame = false, bHaveLens = false;

  string resName;
  bool bKeepThisTXMT = false;
  bool bGreyHairTXMT = false;
  char strHairTXTRname[256];

  for( unsigned int k = 0; k < itemCount; ++k )
  {
    pResource = resources[k];
    if( NULL == pResource )
      continue;

    // Is this a TXMT?
    if( DBPF_TXMT == pResource->getType() )
    {
      bKeepThisTXMT = false;
      bGreyHairTXMT = false;

      // What is the resource name?
      resName = pResource->getName();



      // Is it adult female hair?
      if( false == bHaveHairA  ||  false == bHaveHairAlphaA
       || false == bHaveFrame  ||  false == bHaveLens )
      {
        if( NULL != strstr( resName.c_str(), "afhair" ) )
        {
          if( false == bHaveFrame  &&  NULL != strstr( resName.c_str(), "~frame" ) )
          {
            pResource->getTGIR( tgirFrame );
            bHaveFrame = true;
            bKeepThisTXMT = true;
          }
          else if( false == bHaveLens  &&  NULL != strstr( resName.c_str(), "~lens" ) )
          {
            pResource->getTGIR( tgirLens );
            bHaveLens = true;
            bKeepThisTXMT = true;
          }
          else if( false == bHaveHairAlphaA  &&  NULL != strstr( resName.c_str(), "alpha" ) )
          {
            pResource->getTGIR( tgirHairAlphaA );
            bHaveHairAlphaA = true;
            bKeepThisTXMT = true;
          }
          else if( false == bHaveHairA  &&  NULL != strstr( resName.c_str(), "~hair" ) )
          {
            pResource->getTGIR( tgirHairA );
            bHaveHairA = true;
            bKeepThisTXMT = true;
          }
        }
      }

      // Is it elder hair? (black only)
      if( (HAIR_BLACK == hairColor) &&
              ( false == bHaveHairE  ||  false == bHaveHairAlphaE ) )
      {
        if( NULL != strstr( resName.c_str(), "efhair" ) )
        {
          bGreyHairTXMT = true;

          if( false == bHaveHairAlphaE  &&  NULL != strstr( resName.c_str(), "alpha" ) )
          {
            pResource->getTGIR( tgirHairAlphaE );
            bHaveHairAlphaE = true;
            bKeepThisTXMT = true;
          }
          else if( false == bHaveHairE  &&  NULL != strstr( resName.c_str(), "~hair" ) )
          {
            pResource->getTGIR( tgirHairE );
            bHaveHairE = true;
            bKeepThisTXMT = true;
          }
        }
      }

      // TXMT to keep, alter it.
      // - proper hair, hairalpha TXTR reference
      // - proper frame/lens TXTR reference
      if( true == bKeepThisTXMT )
      {
        // hair or hairalpha TXMT, af or ef
        if( NULL != strstr( resName.c_str(), "~hair" ) )
        {
          strcpy( strHairTXTRname, strMaxisHairTexture );
          if( bGreyHairTXMT == true )
            strcat( strHairTXTRname, "grey" );
          else
          {
            if(      HAIR_BLACK == hairColor )  strcat( strHairTXTRname, "black" );
            else if( HAIR_BROWN == hairColor )  strcat( strHairTXTRname, "brown" );
            else if( HAIR_BLOND == hairColor )  strcat( strHairTXTRname, "blond" );
            else if( HAIR_RED == hairColor   )  strcat( strHairTXTRname, "red"   );
          }
        }
        // frame or lens TXMT
        else
          strcpy( strHairTXTRname, strHatTXTR );

        // change TXTR ref in TXMT
        ((DBPF_TXMTtype*)pResource)->setPropertyValue( "stdMatBaseTextureName", string( strHairTXTRname ) );
      }

      // It's an unnecessary TXMT, nuke it.
      if( false == bKeepThisTXMT )
      {
        delete pResource;
        pResource = NULL;
        resources[k] = NULL;
#ifdef _DEBUG
        printf( "nuke TXMT: %s\n", resName.c_str() );
#endif
      }
#ifdef _DEBUG
      else
      { printf( "keep TXMT: %s\n", resName.c_str() );
      }
#endif
    }
  }


  // remove unneeded age GZPS
  // ----------------------------

  unsigned int instanceToddler = 0;
  unsigned int instanceElder1 = 0;
  unsigned int instanceElder2 = 0;

  unsigned int age = 0;

  for( unsigned int k = 0; k < itemCount; ++k )
  {
    pResource = resources[k];
    if( NULL == pResource )
      continue;

    // Is this a GZPS?
    if( DBPF_GZPS == pResource->getType() )
    {
      // age
      age = ((DBPF_GZPStype*)pResource)->getAge();

      // toddler
      if( 1 == age )
      {
        instanceToddler = pResource->getInstance();

        // remove toddler
        delete pResource; pResource = NULL;
        resources[k] = NULL;
#ifdef _DEBUG
        printf( "nuke GZPS: toddler, instance %u\n", instanceToddler );
#endif
      }

      // elder
      else if( 0x10 == age )
      {
        unsigned int foo = pResource->getInstance();
        if( 0 == instanceElder1 )
          instanceElder1 = foo;
        else
          instanceElder2 = foo;

        // remove elder if hair not black
        if( hairColor != HAIR_BLACK )
        {
          delete pResource; pResource = NULL;
          resources[k] = NULL;
#ifdef _DEBUG
          printf( "nuke GZPS: elder, instance %u\n", foo );
#endif
        }
#ifdef _DEBUG
        else // keep elder if hair is black
          printf( "keep GZPS: elder, instance %u\n", foo );
#endif
      }
    }
  }


  // remove unneeded TXTRs
  // ----------------------------
  // don't need hair texture, use Maxis,
  // non-black hair doesn't need hat texture, use from black (passed in)

  bool bKeepTXTR = false;

  for( unsigned int k = 0; k < (unsigned int)(resources.size()); ++k )
  {
    pResource = resources[k];
    if( NULL == pResource )
      continue;

    // Is this a TXTR?
    if( DBPF_TXTR == pResource->getType() )
    {
      bKeepTXTR = false;

      // black hair, keep hat texture
      // non-black hair, remove all textures
      if( HAIR_BLACK == hairColor )
      {
        string foo, bar;
        foo = pResource->getName();
        bar = string( strHatTXTR ) + "_txtr";
        if( 0 == strcmp( foo.c_str(), bar.c_str() ) )
          bKeepTXTR = true;
      }

      if( false == bKeepTXTR )
      {
#ifdef _DEBUG
        printf( "nuke TXTR: %s\n", pResource->getName().c_str() );
#endif
        delete pResource; pResource = NULL;
        resources[k] = NULL;
      }
#ifdef _DEBUG
      else
      { printf( "keep TXTR: %s\n", pResource->getName().c_str() );
      }
#endif
    }
  }


  // fix up 3IDRs
  // ----------------------------
  // change TXMT refs to existing TXMTs

  DBPF_3IDRtype * p3IDR = NULL;
  bool bUseElderTXMTs = false;

  for( unsigned int k = 0; k < (unsigned int)(resources.size()); ++k )
  {
    pResource = resources[k];
    if( NULL == pResource )
      continue;

    // Is this a 3IDR?
    if( DBPF_3IDR == pResource->getType() )
    {
      p3IDR = (DBPF_3IDRtype*)pResource;

      // instance number
      unsigned int inst;
      inst = p3IDR->getInstance();

      // elder, in black hair
      bUseElderTXMTs = false;
      if( HAIR_BLACK == hairColor && ( inst == instanceElder1 || inst == instanceElder2 ) )
        bUseElderTXMTs = true;

      // 3IDR TGIR loop
      DBPF_TGIRtype entry;
      int TXMTentryCount = 0;
      for( unsigned int t = 0; t < p3IDR->getTGIRcount(); ++t )
      {
        p3IDR->getTGIRentry( t, entry );
        if( DBPF_TXMT == entry.muTypeID )
        {
          // which TXMT are we at?
          ++TXMTentryCount;

          // hair
          if( 1 == TXMTentryCount )
          { if( false == bUseElderTXMTs )
              p3IDR->setTGIRentry( t, tgirHairA );
            else
              p3IDR->setTGIRentry( t, tgirHairE );
          }

          // hair alpha
          else if( TXMTentryCount <= (numAlphaGroups+1) )
          { if( false == bUseElderTXMTs )
              p3IDR->setTGIRentry( t, tgirHairAlphaA );
            else
              p3IDR->setTGIRentry( t, tgirHairAlphaE );
          }

          // hat, frame
          else if( TXMTentryCount == (numAlphaGroups+2) )
          { p3IDR->setTGIRentry( t, tgirFrame );
          }

          // hat, lens
          else
          { p3IDR->setTGIRentry( t, tgirLens );
          }

        }
      }
    }
  }



  // remove NULL resources
  // ----------------------------

  removeNullResources( resources );


// -----------------------------------------------------------------------------


  // write out file
  // --------------------------

  string filenameOut;
  makeOutputFileName( filenameOut, filename, "_HAIR" );
  printf( "writing file: %s\n", filenameOut.c_str() );
  bool bWriteSuccess = writeCompressedPackage( filenameOut.c_str(), package, resources );



  // clean up memory
  // ---------------

  if( resources.empty() == false )
  {
    size_t vecSize = resources.size();
    for( size_t k = 0; k < vecSize; ++k )
    {
      if( resources[k] != NULL )
      { delete resources[k]; resources[k] = NULL; }
    }
    resources.clear();
  }


  // all done
  // ---------------

	return bWriteSuccess;
} // hairProcess
