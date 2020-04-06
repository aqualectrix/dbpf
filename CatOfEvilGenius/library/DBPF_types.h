/**
 * file: DBPF_types.h
 * author: CatOfEvilGenius
 *
 * type constants such as DBPF_TXTR, DBPF_BHAV, etc,
 * and routines for converting type numbers to strings
**/

#ifndef DBPF_TYPES_H_CAT
#define DBPF_TYPES_H_CAT


// DBPF Resource Types
// ===================
// 
// directory of compressed files (appears as CLST in SimPE)
#define DBPF_DIR 0xE86B1EEF 
// text string, catalog description
#define DBPF_STR  0x53545223 
// catalog string
#define DBPF_CATS 0x43415453 
// catalog description
#define DBPF_CTSS 0x43545353 
// pie menu strings
#define DBPF_TTAs 0x54544173 
// pie menu functions
#define DBPF_TTAB 0x54544142 
// material definition
#define DBPF_TXMT 0x49596978
// texture (can be bump map)
#define DBPF_TXTR 0x1c4a276c
// large image file (sometimes used by TXTR)
#define DBPF_LIFO 0xED534136
// hair tone XML
#define DBPF_XHTN 0x8C1580B5
// texture overlay XML
#define DBPF_XTOL 0x2C1FD8A1
// property set
#define DBPF_GZPS 0xEBCF3E27 
// binary index BINX, can also be OBJX
#define DBPF_BINX 0x0C560F39 
// 3D ID referencing file
#define DBPF_3IDR 0xAC506764 
// gmdc - part of mesh, geometric data container, vertices, uv, bone weights, normals
#define DBPF_GMDC 0xAC4F8687
// gmnd - part of mesh, geometric node
#define DBPF_GMND 0x7BA3838C
// shpe - part of a mesh, shape
#define DBPF_SHPE 0xFC6EB1F7 
// cres - part of a mesh, resource node, (skeleton)
#define DBPF_CRES 0xE519C933 
// semi global
#define DBPF_GLOB 0x474C4F42 
// animation
#define DBPF_ANIM 0xFB00791E 
// logic code, behavior function
#define DBPF_BHAV 0x42484156
// face properties
#define DBPF_FACE 0x46414345
// sim data
#define DBPF_PDAT 0xAACE2EFB 
// family information
#define DBPF_FAMI 0x46414D49
// object data
#define DBPF_OBJD 0x4F424A44 
// object functions
#define DBPF_OBJF 0x4F424A66 
// wall or floor thingie, doesn't have an EA resource name,
// so I named it WaFl (SimPE calls it an XOBJ, but its not)
#define DBPF_WaFl 0xCCA8E925 
// object class dump
#define DBPF_XOBJ 0x584F424A 
// house descriptor
#define DBPF_HOUS 0x484F5553 
// roof
#define DBPF_ROOF 0xAB9406AA 
// wall layer
#define DBPF_WLAY 0x8A84D7B0 
// fence post layer
#define DBPF_FPL  0xAB4BA572
// pool surface
#define DBPF_POOL 0x0C900FDB 
// lot definition
#define DBPF_LOT  0x6C589723 
// lot description
#define DBPF_DESC 0x0BF999E7 
// neighborhood terrain
#define DBPF_NHTR 0xABD0DC63 
// neighborhood terrain geometry
#define DBPF_NHTG 0xABCB5DA4 
// world database
#define DBPF_WRLD 0x49FF7D76 
// audio data
#define DBPF_FWAV 0x46574156 


void dbpfResourceTypeToString( const unsigned int rType, char * str );
void dbpfPrintResourceType( const unsigned int rType );


class DBPF_TGIRtype
{
public:
  DBPF_TGIRtype()
    : muGroupID(0), muInstanceID(0), muResourceID(0), muTypeID(0)
  {}
  ~DBPF_TGIRtype() {}

  unsigned int muGroupID;
  unsigned int muInstanceID;
  unsigned int muResourceID; // not always present
  unsigned int muTypeID;

};

bool operator==( const DBPF_TGIRtype & _left, const DBPF_TGIRtype & _right );
bool operator!=( const DBPF_TGIRtype & _left, const DBPF_TGIRtype & _right );


// DBPF_TYPES_H_CAT
#endif