/*
 * Implementation of the interface documented in dbpf.h.
 * Version 20070601.
 *
 * This file (with the exception of some parts adapted from zlib) is
 * Copyright 2007 Ben Rudiak-Gould. Anyone may use it under the terms of
 * the GNU General Public License, version 2 or (at your option) any
 * later version. This code comes with NO WARRANTY. Make backups!
 */

#include "dbpf.h"

#include <string.h>  // for memcpy and memset
#include <stdlib.h>

//#include <assert.h>
#define assert(expr) do{}while(0)


// datatype assumptions: 8-bit bytes; sizeof(int) >= 4


typedef unsigned char byte;
struct word { byte lo,hi; };
struct dword { word lo,hi; };

#ifdef _X86_   // little-endian and no alignment restrictions
  static inline unsigned get(const word& w)     { return *(const unsigned short*)&w; }
  static inline unsigned get(const dword& dw)   { return *(const unsigned*)&dw; }
  static inline void put(word& w, unsigned x)   { *(unsigned short*)&w = x; }
  static inline void put(dword& dw, unsigned x) { *(unsigned*)&dw = x; }
#else
  static inline unsigned get(const word& w)     { return w.lo + w.hi * 256; }
  static inline unsigned get(const dword& dw)   { return get(dw.lo) + get(dw.hi) * 65536; }
  static inline void put(word& w, unsigned x)   { w.lo = x; w.hi = x >> 8; }
  static inline void put(dword& dw, unsigned x) { put(dw.lo, x); put(dw.hi, x >> 16); }
#endif

struct word3be { byte hi,mid,lo; };

static inline unsigned get(const word3be& w3)   { return w3.hi * 65536 + w3.mid * 256 + w3.lo; }
static inline void put(word3be& w3, unsigned x) { w3.hi = x >> 16; w3.mid = x >> 8; w3.lo = x; }


struct dbpf_header    // 96 bytes
{
    dword magic;  // == DBPF_MAGIC
    dword version_major, version_minor;    // 1.0 SC4, 1.1 TS2
    byte reserved[12];
    dword creation_date, modification_date;    // what format???
    dword index_major;            // == 7
    dword index_entry_count;
    dword index_offset, index_size;
    dword hole_entry_count;
    dword hole_offset, hole_size;
    dword index_minor;            // version 1.1+ only
    byte reserved2[32];
};

#define DBPF_MAGIC 0x46504244  // "DBPF"

struct dbpf_index_1    // 20 bytes
{
    dword type_id, group_id;
    dword instance_id;
    dword offset, size;
};

struct dbpf_index_2    // 24 bytes
{
    dword type_id, group_id;
    dword instance_id, instance_id_2;
    dword offset, size;
};


struct dbpf_compressed_dir_1    // 16 bytes
{
    dword type_id, group_id;
    dword instance_id;
    dword decompressed_size;
};


struct dbpf_compressed_dir_2    // 20 bytes
{
    dword type_id, group_id;
    dword instance_id, instance_id_2;
    dword decompressed_size;
};


struct dbpf_hole    // 8 bytes
{
    dword offset, size;
};


struct range { int ofs,len; };


struct DBPF
{
    void* ctx;
    int (*read)(void* ctx, int start, int length, void* buf, const char** error);
    int (*write)(void* ctx, int start, int length, const void* buf, const char** error);
    int (*close)(void* ctx);

    int entry_count;
    dbpf_entry* entries;

    range index_range, hole_range, dir_range;  // for writing
};


static inline
int call_read(DBPF* dbpf, int start, int length, void* buf, const char** error)
{
    return length ? dbpf->read(dbpf->ctx, start, length, buf, error) : 0;
}

static inline
int call_write(DBPF* dbpf, int start, int length, const void* buf, const char** error)
{
    return length ? dbpf->write(dbpf->ctx, start, length, buf, error) : 0;
}

static inline
int call_truncate(DBPF* dbpf, int pos, const char** error)
{
    return dbpf->write(dbpf->ctx, pos, 0, 0, error);
}

// Note that mynew and mydelete don't call the constructor or destructor (we don't have any)
template<class T>
static inline
T* mynew(int n)
{
    int size = n * sizeof(T);
    size += !size;  // don't depend on behavior of malloc(0)
    return (T*)malloc(size);
}

static inline
void mydelete(void* p) { if (p) free(p); }


static bool decompress(const byte* src, int compressed_size, byte* dst, int uncompressed_size, bool truncate);
static byte* compress(const byte* src, const byte* srcend, byte* dst, byte* dstend, bool pad);
static byte* try_compress(const byte* src, int srclen, int* dstlen);


static const int MAX_FILE_SIZE = 0x40000000;


// helper for dbpf_open

static
int dbpf_set_decompressed_size(
    DBPF* dbpf, int last_match_pos,
    unsigned type_id, unsigned group_id,
    unsigned instance_id, unsigned instance_id_2,
    int decompressed_size,
    const char** error)
{
    // This is optimized for the (hopefully common) case that
    // the compressed directory entries are in the same order
    // as the main directory. Otherwise we'd have to construct
    // some sort of lookup table for the index to avoid
    // inefficiency.

    int entry_count = dbpf->entry_count;
    dbpf_entry* entries = dbpf->entries;

    for (int i = 1; i <= entry_count; ++i) {
        int j = last_match_pos + i;
        if (j >= entry_count) j -= entry_count;
        if (1    && entries[j].type_id == type_id
            && entries[j].group_id == group_id
            && entries[j].instance_id == instance_id
            && entries[j].instance_id_2 == instance_id_2)
        {
            if (entries[j].compressed_in_file) {
                *error = "bad DBPF file (duplicate entry in compressed directory)";
                return -1;
            }
            entries[j].compressed_in_file = 1;
            entries[j].size = decompressed_size;
            return j;
        }
    }
    *error = "bad DBPF file (spurious entry in compressed directory)";
    return -1;
}


#define DBPF_TYPE_COMPRESSED_FILE_DIRECTORY 0xE86B1EEF
#define DBPF_INSTANCE_COMPRESSED_FILE_DIRECTORY 0x286B1F03


static inline
int find_dir(DBPF* dbpf, range* dir_range)
{
    dir_range->ofs = 0;
    dir_range->len = 0;
    int i, j = 0;
    for (i = 0; i < dbpf->entry_count; ++i) {
        if (dbpf->entries[i].type_id == DBPF_TYPE_COMPRESSED_FILE_DIRECTORY) {
            dir_range->ofs = dbpf->entries[i].offset_in_file;
            dir_range->len = dbpf->entries[i].size_in_file;
        } else {
            if (j != i) dbpf->entries[j] = dbpf->entries[i];
            ++j;
        }
    }
    dbpf->entry_count = j;
    return i-j;
}


extern "C"
DBPF* dbpf_open(
    void* ctx,
    int (*read)(void* ctx, int start, int length, void* buf, const char** error),
    int (*write)(void* ctx, int start, int length, const void* buf, const char** error),
    int (*close)(void* ctx),
    const char** error)
{
#define ERROR(msg)    do { *error = msg; dbpf_close(dbpf); return 0; } while (0)
#define MAYFAIL(expr) do { if ((expr) < 0) { dbpf_close(dbpf); return 0; } } while (0)
#define ALLOC(expr)   do { if ((expr) == 0) ERROR("allocation failure"); } while (0)

    DBPF* dbpf;
    ALLOC(dbpf = mynew<DBPF>(1));
    dbpf->ctx = ctx;
    dbpf->read = read;
    dbpf->write = write;
    dbpf->close = close;
    dbpf->entry_count = 0;
    dbpf->entries = 0;

    dbpf_header hdr;
    MAYFAIL(call_read(dbpf, 0, sizeof(hdr), &hdr, error));

    if (get(hdr.magic) != DBPF_MAGIC)
        ERROR("not a DBPF file (missing signature)");

    unsigned version_minor = get(hdr.version_minor);
    unsigned index_minor = version_minor ? get(hdr.index_minor) : 1;
    if (get(hdr.version_major) != 1 || version_minor > 2  /* what's version 1.2? */
            || get(hdr.index_major) != 7 || index_minor < 1 || index_minor > 2)
        ERROR("not a supported DBPF file (unrecognized version)");

    dbpf->entry_count = get(hdr.index_entry_count);

    dbpf->index_range.ofs = get(hdr.index_offset);
    dbpf->index_range.len = get(hdr.index_size);
    dbpf->hole_range.ofs = get(hdr.hole_offset);
    dbpf->hole_range.len = get(hdr.hole_size);

    // read the index

    if (dbpf->entry_count) {
        ALLOC(dbpf->entries = mynew<dbpf_entry>(dbpf->entry_count));
        if (index_minor == 1) {
            if (dbpf->index_range.len != dbpf->entry_count * (int)sizeof(dbpf_index_1))
                ERROR("bad DBPF file (index size mismatch)");
            dbpf_index_1* idx;
            ALLOC(idx = mynew<dbpf_index_1>(dbpf->entry_count));
            MAYFAIL(call_read(dbpf, dbpf->index_range.ofs, dbpf->index_range.len, idx, error));
            int i;
            for (i = 0; i < dbpf->entry_count; ++i) {
                dbpf->entries[i].type_id = get(idx[i].type_id);
                dbpf->entries[i].group_id = get(idx[i].group_id);
                dbpf->entries[i].instance_id = get(idx[i].instance_id);
                dbpf->entries[i].instance_id_2 = 0;
                dbpf->entries[i].offset_in_file = get(idx[i].offset);
                dbpf->entries[i].size_in_file = get(idx[i].size);
            }
            mydelete(idx);
        } else {
            if (dbpf->index_range.len != dbpf->entry_count * (int)sizeof(dbpf_index_2))
                ERROR("bad DBPF file (index size mismatch)");
            dbpf_index_2* idx;
            ALLOC(idx = mynew<dbpf_index_2>(dbpf->entry_count));
            MAYFAIL(call_read(dbpf, dbpf->index_range.ofs, dbpf->index_range.len, idx, error));
            int i;
            for (i = 0; i < dbpf->entry_count; ++i) {
                dbpf->entries[i].type_id = get(idx[i].type_id);
                dbpf->entries[i].group_id = get(idx[i].group_id);
                dbpf->entries[i].instance_id = get(idx[i].instance_id);
                dbpf->entries[i].instance_id_2 = get(idx[i].instance_id_2);
                dbpf->entries[i].offset_in_file = get(idx[i].offset);
                dbpf->entries[i].size_in_file = get(idx[i].size);
            }
            mydelete(idx);
        }
        int i;
        for (i = 0; i < dbpf->entry_count; ++i) {
			dbpf_entry* e = &dbpf->entries[i];
            e->size = e->size_in_file;   // until we hear otherwise
            e->write_disposition = dbpf_write_keep_existing;
            e->compressed_in_file = 0;   // until we hear otherwise
			// check for overflow (careful, most algebraic laws don't hold)
			if (e->offset_in_file < 0 || e->offset_in_file >= MAX_FILE_SIZE
					|| e->size_in_file < 0 || e->size_in_file >= MAX_FILE_SIZE - e->offset_in_file)
				ERROR("bad DBPF file (bad data offset in index)");
        }
    }

    // read the compressed file index (what a waste of space and every implementor's time...)

    int num_dirs;
    num_dirs = find_dir(dbpf, &dbpf->dir_range);
    if (num_dirs > 1) {
        ERROR("bad DBPF file (more than one compressed file directory");
    } else if (num_dirs == 1) {
        if (index_minor == 1) {
            if (dbpf->dir_range.len % sizeof(dbpf_compressed_dir_1) != 0)
                ERROR("bad DBPF file (bad compressed directory size)");
            int dir_entry_count = dbpf->dir_range.len / sizeof(dbpf_compressed_dir_1);
            dbpf_compressed_dir_1* dir;
            ALLOC(dir = mynew<dbpf_compressed_dir_1>(dir_entry_count));
            MAYFAIL(call_read(dbpf, dbpf->dir_range.ofs, dbpf->dir_range.len, dir, error));
            int pos = 0;
            int i;
            for (i = 0; i < dir_entry_count; ++i) {
                MAYFAIL(pos = dbpf_set_decompressed_size(
                                dbpf, pos,
                                get(dir[i].type_id), get(dir[i].group_id),
                                get(dir[i].instance_id), 0,
                                get(dir[i].decompressed_size),
                                error));
            }
            mydelete(dir);
        } else {
            if (dbpf->dir_range.len % sizeof(dbpf_compressed_dir_2) != 0)
                ERROR("bad DBPF file (bad compressed directory size)");
            int dir_entry_count = dbpf->dir_range.len / sizeof(dbpf_compressed_dir_2);
            dbpf_compressed_dir_2* dir;
            ALLOC(dir = mynew<dbpf_compressed_dir_2>(dir_entry_count));
            MAYFAIL(call_read(dbpf, dbpf->dir_range.ofs, dbpf->dir_range.len, dir, error));
            int pos = 0;
            int i;
            for (i = 0; i < dir_entry_count; ++i) {
                MAYFAIL(pos = dbpf_set_decompressed_size(
                                dbpf, pos,
                                get(dir[i].type_id), get(dir[i].group_id),
                                get(dir[i].instance_id), get(dir[i].instance_id_2),
                                get(dir[i].decompressed_size),
                                error));
            }
            mydelete(dir);
        }
    }

    return dbpf;

#undef ERROR
#undef MAYFAIL
#undef ALLOC
}


extern "C"
int dbpf_get_entry_count(const DBPF* dbpf) { return dbpf->entry_count; }

extern "C"
const struct dbpf_entry* dbpf_get_entries(const DBPF* dbpf) { return dbpf->entries; }


extern "C"
int dbpf_read(DBPF* dbpf, int entry_index, byte* buf, const char** error)
{
    const dbpf_entry* e = &dbpf->entries[entry_index];
    if (e->compressed_in_file) {
        byte* rawbuf = mynew<byte>(e->size_in_file);
        if (!rawbuf) {
            *error = "allocation failure";
            return -1;
        }
        int rtn = 0;
        if (call_read(dbpf, e->offset_in_file, e->size_in_file, rawbuf, error) < 0) {
            rtn = -1;
        } else if (!decompress(rawbuf, e->size_in_file, buf, e->size, false)) {
            *error = "bad DBPF file (invalid compressed data)";
            rtn = -1;
        }
        mydelete(rawbuf);
        return rtn;
    } else {
        int result = call_read(dbpf, e->offset_in_file, e->size, buf, error);
        return (result < 0) ? -1 : 0;
    }
}


extern "C"
int dbpf_read_64bytes(DBPF* dbpf, int entry_index, byte* buf, const char** error)
{
    const dbpf_entry* e = &dbpf->entries[entry_index];
    if (e->size < 64) {
        *error = "bad DBPF file (missing file name)";
        return -1;
    }
    if (e->compressed_in_file) {
        // what's the most that 64 bytes could ever compress to? 150 bytes should be safe.
        byte rawbuf[150];
        int rawcount = e->size_in_file < 150 ? e->size_in_file : 150;
        int rtn = 0;
        if (call_read(dbpf, e->offset_in_file, rawcount, rawbuf, error) < 0) {
            rtn = -1;
        } else if (!decompress(rawbuf, rawcount, buf, 64, true)) {
            *error = "bad DBPF file (invalid compressed data)";
            rtn = -1;
        }
        return rtn;
    } else {
        int result = call_read(dbpf, e->offset_in_file, 64, buf, error);
        return (result < 0) ? -1 : 0;
    }
}


extern "C"
int dbpf_close(DBPF* dbpf)
{
    if (dbpf) {
        int rtn = dbpf->close ? dbpf->close(dbpf->ctx) : 0;
        mydelete(dbpf->entries);
        mydelete(dbpf);
        return rtn;
    } else {
        return 0;
    }
}


static
void make_header(
    dbpf_header* hdr,
    int index_entry_count, range index_range,
    int hole_entry_count, range hole_range)
{
    memset(hdr, 0, sizeof(*hdr));
    put(hdr->magic, DBPF_MAGIC);
    put(hdr->version_major, 1);
    put(hdr->version_minor, 1);
    put(hdr->index_major, 7);
    put(hdr->index_entry_count, index_entry_count);
    put(hdr->index_offset, index_range.ofs);
    put(hdr->index_size, index_range.len);
    put(hdr->hole_entry_count, hole_entry_count);
    put(hdr->hole_offset, hole_range.ofs);
    put(hdr->hole_size, hole_range.len);
    put(hdr->index_minor, 2);
}


extern "C"
void dbpf_create(byte* buf)
{
    range none = {0,0};
    make_header((dbpf_header*)buf, 0, none, 0, none);
}


/************************** file writing routines ***************************/


extern "C"
int dbpf_update_in_place(DBPF* dbpf, int entry_index, const byte* buf, const char** error)
{
    dbpf_entry* e = &dbpf->entries[entry_index];
    byte* cbuf = 0;
    if (e->compressed_in_file) {
        cbuf = mynew<byte>(e->size_in_file);
        if (!compress(buf, buf + e->size, cbuf, cbuf + e->size_in_file, true)) {
            mydelete(cbuf);
            return 0;
        }
    }
    int result = call_write(dbpf, e->offset_in_file, e->size_in_file, e->compressed_in_file ? cbuf : buf, error);
    mydelete(cbuf);
    return (result < 0) ? -1 : 1;
}


static inline
int CompareUnsigned(unsigned a, unsigned b) { return (a>b)-(a<b); }

static
int CompareUnsigned(const void* a, const void* b)
{
    return CompareUnsigned(*(const unsigned*)a, *(const unsigned*)b);
}

extern "C"
int dbpf_compare_entries(const dbpf_entry* a, const dbpf_entry* b)
{
    int cmp;
    if ((cmp = CompareUnsigned(a->type_id, b->type_id)) != 0) return cmp;
    if ((cmp = CompareUnsigned(a->group_id, b->group_id)) != 0) return cmp;
    if ((cmp = CompareUnsigned(a->instance_id_2, b->instance_id_2)) != 0) return cmp;
    return CompareUnsigned(a->instance_id, b->instance_id);
}


static
int free_of_used(const range* src, int count, range* dst)
{
    unsigned* boundaries = mynew<unsigned>(count*2+2);
    int i;
    for (i=0; i<count; ++i) {
        boundaries[i * 2] = src[i].ofs * 2;
        boundaries[i * 2 + 1] = (src[i].ofs + src[i].len) * 2 + 1;
    }
    boundaries[count*2] = 1;
    boundaries[count*2+1] = unsigned(MAX_FILE_SIZE) * 2;
    qsort(boundaries, count*2+2, sizeof(unsigned), CompareUnsigned);

    int used = 1;
    int k = 0;
    i = 0;
    while (i < count*2+2) {
        int j = i;
        int change = 0;
        unsigned pos = boundaries[i] >> 1;
        do {
            change -= (boundaries[j] & 1) * 2 - 1;
            ++j;
        } while (j < count*2+2 && pos == (boundaries[j] >> 1));
        if (used > 0 && used+change == 0) {
            dst[k].ofs = pos;
        } else if (used == 0 && used+change > 0) {
            dst[k].len = pos - dst[k].ofs;
            ++k;
        }
        used += change;
        i = j;
    }
    assert(used == 1);
    mydelete(boundaries);
    return k;
}


static
range* find_holes(
    dbpf_entry* entries, int entry_count,
    range index_range, range hole_range, range dir_range,
    int* num_holes)
{
    range* used = mynew<range>(entry_count + 4);
    if (!used) return false;
    range* holes = mynew<range>(entry_count + 5);
    if (!holes) {
        mydelete(used);
        return false;
    }

    for (int i = 0; i < entry_count; ++i) {
        used[i].ofs = entries[i].offset_in_file;
        used[i].len = entries[i].size_in_file;
    }
    used[entry_count].ofs = 0;
    used[entry_count].len = sizeof(dbpf_header);
    used[entry_count+1] = index_range;
    used[entry_count+2] = hole_range;
    used[entry_count+3] = dir_range;
    *num_holes = free_of_used(used, entry_count + 4, holes);
    mydelete(used);
    return holes;
}


static
int file_alloc(int size, range* holes, int num_holes)
{
    // Put the file in the smallest hole that's large enough to accommodate it
    int best_len = MAX_FILE_SIZE;
    int hole = -1;
    for (int i=0; i<num_holes; ++i) {
        if (holes[i].len >= size) {
            if (holes[i].len < best_len) {
                hole = i;
                best_len = holes[i].len;
            }
        }
    }
    if (hole < 0) {
        return -1;
    } else {
        int result = holes[hole].ofs;
        holes[hole].ofs += size;
        holes[hole].len -= size;
        return result;
    }
}


static int CompareEntryPtrs(const void* p, const void* q)
{
    return dbpf_compare_entries(*(const dbpf_entry*const*)p, *(const dbpf_entry*const*)q);
}

static bool duplicate_entries(const dbpf_entry* entries, int entry_count)
{
    const dbpf_entry** e = mynew<const dbpf_entry*>(entry_count);
    int i;
    for (i = 0; i < entry_count; ++i)
        e[i] = &entries[i];
    qsort(e, entry_count, sizeof(const dbpf_entry*), CompareEntryPtrs);
    for (i = 1; i < entry_count; ++i)
        if (dbpf_compare_entries(e[i-1], e[i]) == 0)
            break;
    mydelete(e);
    return (i < entry_count);
}


template<class T>
class auto_mydelete {
public:
    T* val;
    auto_mydelete() { val = 0; }
    ~auto_mydelete() { mydelete(val); }
    T* operator=(T* newval) { return val = newval; }
    bool operator!() { return !val; }
    T& operator[](int i) { return val[i]; }
    operator T*() { return val; }
    T* keep() { T* temp = val; val = 0; return temp; }
};


extern "C"
int dbpf_write(
    DBPF* dbpf,
    const struct dbpf_entry* client_new_entries,
    int client_new_entry_count,
    void* ctx,
    const byte* (*get_data)(void* ctx, int entry_index, const char** error),
    const char** error)
{
#define ERROR(msg)      do { *error = msg; return -1; } while (0)
#define FAILMINUS(expr) do { if ((expr) < 0) return -1; } while (0)
#define FAILZERO(expr)  do { if ((expr) == 0) return -1; } while (0)
#define FILEALLOC(expr) do { if ((expr) < 0) ERROR("DBPF file too large"); } while (0)
#define ALLOC(expr)     do { if ((expr) == 0) ERROR("allocation failure"); } while (0)

    // We ignore the existing hole table and make our own
    int num_holes;
    auto_mydelete<range> holes;
    ALLOC(holes = find_holes(
            dbpf->entries, dbpf->entry_count,
            dbpf->index_range, dbpf->hole_range, dbpf->dir_range,
            &num_holes));

    // Find a location for each entry in the new index (including
    // the compressed file directory) plus the index and table of holes.

    auto_mydelete<dbpf_entry> new_entries;
    ALLOC(new_entries = mynew<dbpf_entry>(client_new_entry_count + 1));

    int new_entry_count = 0;
    {for (int i = 0; i < client_new_entry_count; ++i) {
        if (client_new_entries[i].write_disposition != dbpf_write_skip)
            new_entries[new_entry_count++] = client_new_entries[i];
    }}

    if (duplicate_entries(new_entries, new_entry_count))
        ERROR("duplicate entry in index (same type, group and instance)");

    int num_compressed = 0;

    {for (int i = 0; i < new_entry_count; ++i) {

        dbpf_entry* e = &new_entries[i];
        if (e->type_id == DBPF_TYPE_COMPRESSED_FILE_DIRECTORY)
            ERROR("invalid type ID (bug)");

        byte* compressed = 0;
        const byte* data_to_write = 0;

        switch (e->write_disposition)
        {
        case dbpf_write_keep_existing:
            break;
        case dbpf_write_uncompressed:
            FAILZERO(data_to_write = get_data(ctx, i, error));
            e->compressed_in_file = 0;
            e->size_in_file = e->size;
            break;
        case dbpf_write_compressed:
            FAILZERO(data_to_write = get_data(ctx, i, error));
            compressed = try_compress(data_to_write, e->size, &e->size_in_file);
            if (compressed) {
                data_to_write = compressed;
                e->compressed_in_file = 1;
            } else {
                e->compressed_in_file = 0;
                e->size_in_file = e->size;
            }
            break;
        case dbpf_write_compressed_raw:
            FAILZERO(data_to_write = get_data(ctx, i, error));
            e->compressed_in_file = 1;
            break;
        default:
            ERROR("invalid write_disposition (bug)");
        }
        if (e->write_disposition != dbpf_write_keep_existing) {
            FILEALLOC(e->offset_in_file = file_alloc(e->size_in_file, holes, num_holes));
            int io = call_write(dbpf, e->offset_in_file, e->size_in_file, data_to_write, error);
            mydelete(compressed);
            FAILMINUS(io);
            e->write_disposition = dbpf_write_keep_existing;
        }
        num_compressed += !!e->compressed_in_file;
    }}

    range new_dir_range;
    new_dir_range.len = num_compressed * sizeof(dbpf_compressed_dir_2);
    new_dir_range.ofs = 0;

    if (new_dir_range.len > 0) {
        // add a compressed file directory

        FILEALLOC(new_dir_range.ofs = file_alloc(new_dir_range.len, holes, num_holes));

        dbpf_entry* e = &new_entries[new_entry_count++];
        e->type_id = DBPF_TYPE_COMPRESSED_FILE_DIRECTORY;
        e->group_id = DBPF_TYPE_COMPRESSED_FILE_DIRECTORY;
        e->instance_id = DBPF_INSTANCE_COMPRESSED_FILE_DIRECTORY;
        e->instance_id_2 = 0;
        e->size = new_dir_range.len;
        e->write_disposition = dbpf_write_keep_existing;
        e->compressed_in_file = 0;
        e->offset_in_file = new_dir_range.ofs;
        e->size_in_file = new_dir_range.len;

        dbpf_compressed_dir_2* dir;
        ALLOC(dir = mynew<dbpf_compressed_dir_2>(num_compressed));
        int j = 0;
        for (int i = 0; i < new_entry_count; ++i) {
            dbpf_entry* e2 = &new_entries[i];
            if (e2->compressed_in_file) {
                put(dir[j].type_id, e2->type_id);
                put(dir[j].group_id, e2->group_id);
                put(dir[j].instance_id, e2->instance_id);
                put(dir[j].instance_id_2, e2->instance_id_2);
                put(dir[j].decompressed_size, e2->size);
                ++j;
            }
        }
        int io = call_write(dbpf, new_dir_range.ofs, new_dir_range.len, dir, error);
        mydelete(dir);
        FAILMINUS(io);
    }

    // Write the new index to the file
    range new_index_range;
    new_index_range.len = new_entry_count * sizeof(dbpf_index_2);
    new_index_range.ofs = 0;
    if (new_index_range.len) {
        FILEALLOC(new_index_range.ofs = file_alloc(new_index_range.len, holes, num_holes));
        dbpf_index_2* index;
        ALLOC(index = mynew<dbpf_index_2>(new_entry_count));
        for (int i = 0; i < new_entry_count; ++i) {
            dbpf_entry* e = &new_entries[i];
            dbpf_index_2* x = &index[i];
            put(x->type_id, e->type_id);
            put(x->group_id, e->group_id);
            put(x->instance_id, e->instance_id);
            put(x->instance_id_2, e->instance_id_2);
            put(x->offset, e->offset_in_file);
            put(x->size, e->size_in_file);
        }
        int io = call_write(dbpf, new_index_range.ofs, new_index_range.len, index, error);
        mydelete(index);
        FAILMINUS(io);
    }

    // Almost done! Make and write a new hole list
    mydelete(holes);
    ALLOC(holes = find_holes(
            new_entries, new_entry_count,
            new_index_range, new_index_range, new_index_range,
            &num_holes));

    range new_hole_range;
    new_hole_range.len = (num_holes-1) * sizeof(dbpf_hole);
    new_hole_range.ofs = 0;

    if (new_hole_range.len) {
        // There's a silly catch-22 here: if the hole list exactly fills a hole,
        // then there's one less hole, so the hole list gets shorter, which means
        // it doesn't fill the hole... I hope a hole length of zero is okay.
        FILEALLOC(new_hole_range.ofs = file_alloc(new_hole_range.len, holes, num_holes));
        dbpf_hole* hole_list;
        ALLOC(hole_list = mynew<dbpf_hole>(num_holes-1));  // omit the final hole
        for (int i = 0; i < num_holes-1; ++i) {
            put(hole_list[i].offset, holes[i].ofs);
            put(hole_list[i].size, holes[i].len);
        }
        int io = call_write(dbpf, new_hole_range.ofs, new_hole_range.len, hole_list, error);
        mydelete(hole_list);
        FAILMINUS(io);
    }

    // Write the header
    dbpf_header hdr;
    make_header(&hdr, new_entry_count, new_index_range, num_holes-1, new_hole_range);
    FAILMINUS(call_write(dbpf, 0, sizeof(dbpf_header), &hdr, error));

    // Truncate the file
    FAILMINUS(call_truncate(dbpf, holes[num_holes-1].ofs, error));

    // Done!
    dbpf->entry_count = client_new_entry_count;
    mydelete(dbpf->entries);
    dbpf->entries = new_entries.keep();
    dbpf->index_range = new_index_range;
    dbpf->hole_range = new_hole_range;
    dbpf->dir_range = new_dir_range;

    return 0;

#undef ERROR
#undef FAILMINUS
#undef FAILZERO
#undef FILEALLOC
#undef ALLOC
}


/********************** low-level compression routines **********************/


struct dbpf_compressed_file_header  // 9 bytes
{
    dword compressed_size;
    word compression_id;       // DBPF_COMPRESSION_QFS
    word3be uncompressed_size;
};

#define DBPF_COMPRESSION_QFS (0xFB10)


static
bool decompress(const byte* src, int compressed_size, byte* dst, int uncompressed_size, bool truncate)
{
    const byte* src_end = src + compressed_size;
    byte* dst_end = dst + uncompressed_size;
    byte* dst_start = dst;

    if (compressed_size < (int)sizeof(dbpf_compressed_file_header) + 1)
        return false;
    const dbpf_compressed_file_header* hdr = (const dbpf_compressed_file_header*)src;

    if (get(hdr->compression_id) != DBPF_COMPRESSION_QFS)
        return false;

    int hdr_c_size = get(hdr->compressed_size), hdr_uc_size = get(hdr->uncompressed_size);
    if (truncate) {
        if (hdr_c_size < compressed_size || hdr_uc_size < uncompressed_size)
            return false;
    } else {
        if (hdr_c_size != compressed_size || hdr_uc_size != uncompressed_size)
            return false;
    }

    src += sizeof(dbpf_compressed_file_header);

    unsigned b0;
    do {
        int lit, copy, offset;
        b0 = *src++;
        if (b0 < 0x80) {
            if (src == src_end) return false;
            unsigned b1 = *src++;
            lit = b0 & 0x03;                        // 0..3
            copy = ((b0 & 0x1C) >> 2) + 3;          // 3..10
            offset = ((b0 & 0x60) << 3) + b1 + 1;   // 1..1024
        } else if (b0 < 0xC0) {
            if (src+2 > src_end) return false;
            unsigned b1 = *src++;
            unsigned b2 = *src++;
            lit = (b1 & 0xC0) >> 6;                 // 0..3
            copy = (b0 & 0x3F) + 4;                 // 4..67
            offset = ((b1 & 0x3F) << 8) + b2 + 1;   // 1..16384
        } else if (b0 < 0xE0) {
            if (src+3 > src_end) return false;
            unsigned b1 = *src++;
            unsigned b2 = *src++;
            unsigned b3 = *src++;
            lit = b0 & 0x03;                        // 0..3
            copy = ((b0 & 0x0C) << 6) + b3 + 5;     // 5..1028
            offset = ((b0 & 0x10) << 12) + (b1 << 8) + b2 + 1;  // 1..131072
        } else if (b0 < 0xFC) {
            lit = (b0 - 0xDF) * 4;                  // 4..112
            copy = 0;
            offset = 0;
        } else {
            lit = b0 - 0xFC;
            copy = 0;
            offset = 0;
        }
        if (src + lit > src_end || dst + lit + copy > dst_end) {
            if (!truncate)
                return false;
            if (lit > dst_end - dst)
                lit = dst_end - dst;
            if (copy > dst_end - dst - lit)
                copy = dst_end - dst - lit;
            if (src + lit > src_end)
                return false;
        }
        if (lit) {
            memcpy(dst, src, lit);
            dst += lit; src += lit;
        }
        if (copy) {
            if (offset > dst - dst_start)
                return false;
            if (offset == 1) {
                memset(dst, dst[-1], copy);
                dst += copy;
            } else {
                do {
                    *dst = *(dst-offset);
                    ++dst;
                } while (--copy);
            }
        }
    } while (src < src_end && dst < dst_end);

    if (truncate) {
        return (dst == dst_end);
    } else {
        while (src < src_end && *src == 0xFC)
            ++src;
        return (src == src_end && dst == dst_end);
    }
}


/*
 * Try to compress the data and return the result in a buffer (which the
 * caller must delete). If it's uncompressable, return NULL.
 */
static
byte* try_compress(const byte* src, int srclen, int* dstlen)
{
    // There are only 3 byte for the uncompressed size in the header,
    // so I guess we can only compress files larger than 16MB...
    if (srclen < 14 || srclen >= 16777216) return 0;

    // We only want the compressed output if it's smaller than the
    // uncompressed.
    byte* dst = mynew<byte>(srclen-1);
    if (!dst) return 0;

    byte* dstend = compress(src, src+srclen, dst, dst+srclen-1, false);
    if (dstend) {
        *dstlen = dstend - dst;
        return dst;
    } else {
        *dstlen = 0;
        mydelete(dst);
        return 0;
    }
}



#define MAX_MATCH 1028
#define MIN_MATCH 3

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)

// corresponds to zlib compression level 9
#define GOOD_LENGTH 32
#define MAX_LAZY    258
#define NICE_LENGTH 258
#define MAX_CHAIN   4096

#define HASH_BITS 16
#define HASH_SIZE 65536
#define HASH_MASK 65535
#define HASH_SHIFT 6

#define W_SIZE 131072
#define MAX_DIST W_SIZE
#define W_MASK (W_SIZE-1)


class Hash
{
private:
    unsigned hash;
    int *head, *prev;
public:
    Hash() {
        hash = 0;
        head = mynew<int>(HASH_SIZE);
        for (int i=0; i<HASH_SIZE; ++i)
            head[i] = -1;
        prev = mynew<int>(W_SIZE);
    }
    ~Hash() {
        mydelete(head);
        mydelete(prev);
    }

    int getprev(unsigned pos) const { return prev[pos & W_MASK]; }

    void update(unsigned c) {
        hash = ((hash << HASH_SHIFT) ^ c) & HASH_MASK;
    }

    int insert(unsigned pos) {
        int match_head = prev[pos & W_MASK] = head[hash];
        head[hash] = pos;
        return match_head;
    }
};

class CompressedOutput
{
private:

    byte* dstpos;
    byte* dstend;
    const byte* src;
    unsigned srcpos;

public:

    CompressedOutput(const byte* src_, byte* dst, byte* dstend_) {
        dstpos = dst; dstend = dstend_; src = src_;
        srcpos = 0;
    }

    byte* get_end() { return dstpos; }

    bool emit(unsigned from_pos, unsigned to_pos, unsigned count)
    {
        if (count)
            assert(memcmp(src + from_pos, src + to_pos, count) == 0);

        unsigned lit = to_pos - srcpos;

        while (lit >= 4) {
            unsigned amt = lit>>2;
            if (amt > 28) amt = 28;
            if (dstpos + amt*4 >= dstend) return false;
            *dstpos++ = 0xE0 + amt - 1;
            memcpy(dstpos, src + srcpos, amt*4);
            dstpos += amt*4;
            srcpos += amt*4;
            lit -= amt*4;
        }

        unsigned offset = to_pos - from_pos - 1;

        if (count == 0) {
            if (dstpos+1+lit > dstend) return false;
            *dstpos++ = 0xFC + lit;
        } else if (offset < 1024 && 3 <= count && count <= 10) {
            if (dstpos+2+lit > dstend) return false;
            *dstpos++ = ((offset >> 3) & 0x60) + ((count-3) * 4) + lit;
            *dstpos++ = offset;
        } else if (offset < 16384 && 4 <= count && count <= 67) {
            if (dstpos+3+lit > dstend) return false;
            *dstpos++ = 0x80 + (count-4);
            *dstpos++ = lit * 0x40 + (offset >> 8);
            *dstpos++ = offset;
        } else /* if (offset < 131072 && 5 <= count && count <= 1028) */ {
            if (dstpos+4+lit > dstend) return false;
            *dstpos++ = 0xC0 + ((offset >> 12) & 0x10) + (((count-5) >> 6) & 0x0C) + lit;
            *dstpos++ = offset >> 8;
            *dstpos++ = offset;
            *dstpos++ = (count-5);
        }

        for (; lit; --lit) *dstpos++ = src[srcpos++];
        srcpos += count;

        return true;
    }
};


/*
 * The following two functions (longest_match and compress) are loosely
 * adapted from zlib 1.2.3's deflate.c, and are probably still covered by
 * the zlib license, which carries this notice:
 */
/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 1.2.3, July 18th, 2005

  Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files http://www.ietf.org/rfc/rfc1950.txt
  (zlib format), rfc1951.txt (deflate format) and rfc1952.txt (gzip format).
*/


static inline
unsigned longest_match(
    int cur_match,
    const Hash& hash,
    const byte* const src,
    const byte* const srcend,
    unsigned const pos,
    unsigned const remaining,
    unsigned const prev_length,
    unsigned* pmatch_start)
{
    unsigned chain_length = MAX_CHAIN;         /* max hash chain length */
    int best_len = prev_length;                /* best match length so far */
    int nice_match = NICE_LENGTH;              /* stop if match long enough */
    int limit = pos > MAX_DIST ? pos - MAX_DIST + 1 : 0;
    /* Stop when cur_match becomes < limit. */

    const byte* const scan = src+pos;

    /* This is important to avoid reading past the end of the memory block */
    if (best_len >= (int)remaining)
        return remaining;

    const int max_match = (remaining < MAX_MATCH) ? remaining : MAX_MATCH;
    byte scan_end1  = scan[best_len-1];
    byte scan_end   = scan[best_len];

    /* Do not waste too much time if we already have a good match: */
    if (prev_length >= GOOD_LENGTH) {
        chain_length >>= 2;
    }
    /* Do not look for matches beyond the end of the input. This is necessary
     * to make deflate deterministic.
     */
    if ((unsigned)nice_match > remaining) nice_match = remaining;

    do {
        assert(cur_match < pos);
        const byte* match = src + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2.
         */
        if (match[best_len]   != scan_end  ||
            match[best_len-1] != scan_end1 ||
            match[0]          != scan[0]   ||
            match[1]          != scan[1])      continue;

        /* It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        assert(scan[2] == match[2]);

        int len = 2;
        do { ++len; } while (len < max_match && scan[len] == match[len]);

        if (len > best_len) {
            *pmatch_start = cur_match;
            best_len = len;
            if (len >= nice_match || scan+len >= srcend) break;
            scan_end1  = scan[best_len-1];
            scan_end   = scan[best_len];
        }
    } while ((cur_match = hash.getprev(cur_match)) >= limit
             && --chain_length > 0);

    return best_len;
}


/* Returns the end of the compressed data if successful, or NULL if we overran the output buffer */

static
byte* compress(const byte* src, const byte* srcend, byte* dst, byte* dstend, bool pad)
{
    unsigned match_start = 0;
    unsigned match_length = MIN_MATCH-1;           /* length of best match */
    bool match_available = false;         /* set if previous match exists */

    unsigned pos = 0, remaining = srcend - src;

    if (remaining >= 16777216) return 0;

    CompressedOutput compressed_output(src, dst+sizeof(dbpf_compressed_file_header), dstend);

    Hash hash;
    hash.update(src[0]);
    hash.update(src[1]);

    while (remaining) {

        unsigned prev_length = match_length;
        unsigned prev_match = match_start;
        match_length = MIN_MATCH-1;

        int hash_head = -1;

        if (remaining >= MIN_MATCH) {
            hash.update(src[pos + MIN_MATCH-1]);
            hash_head = hash.insert(pos);
        }

        if (hash_head >= 0 && prev_length < MAX_LAZY && pos - hash_head <= MAX_DIST) {

            match_length = longest_match (hash_head, hash, src, srcend, pos, remaining, prev_length, &match_start);

            /* If we can't encode it, drop it. */
            if ((match_length <= 3 && pos - match_start > 1024) || (match_length <= 4 && pos - match_start > 16384))
                match_length = MIN_MATCH-1;
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (prev_length >= MIN_MATCH && match_length <= prev_length) {

            if (!compressed_output.emit(prev_match, pos-1, prev_length))
                return 0;

            /* Insert in hash table all strings up to the end of the match.
             * pos-1 and pos are already inserted. If there is not
             * enough lookahead, the last two strings are not inserted in
             * the hash table.
             */
            remaining -= prev_length-1;
            prev_length -= 2;
            do {
                ++pos;
                if (src+pos <= srcend-MIN_MATCH) {
                    hash.update(src[pos + MIN_MATCH-1]);
                    hash.insert(pos);
                }
            } while (--prev_length != 0);
            match_available = false;
            match_length = MIN_MATCH-1;
            ++pos;

        } else  {
            match_available = true;
            ++pos;
            --remaining;
        }
    }
    assert(pos == srcend - src);
    if (!compressed_output.emit(pos, pos, 0))
        return 0;

    byte* dstsize = compressed_output.get_end();
    if (pad && dstsize < dstend) {
        memset(dstsize, 0xFC, dstend-dstsize);
        dstsize = dstend;
    }

    dbpf_compressed_file_header* hdr = (dbpf_compressed_file_header*)dst;
    put(hdr->compressed_size, dstsize - dst);
    put(hdr->compression_id, DBPF_COMPRESSION_QFS);
    put(hdr->uncompressed_size, srcend-src);

    return dstsize;
}


/*

// This is SimPE's database of type ID information, derived from tgi.xml
// (Revision 312 in the SimPE repository, dated 2007 Mar 14). At some point
// I might provide a library interface to access it, or you can just copy
// and paste it into your program.

struct type_id_info
{
    unsigned type_id;  // table index
    bool embedded_filename;
    const char* ext;
    const char* short_name;
    const char* long_name;
};

type_id_info tgi[] = {
    { 0x00000000, 0,"uiScript","UI","UI Data" },
    { 0x0A284D0B, 0, 0,     "WGRA", "Wall Graph" },
    { 0x0B9EB87E, 0, 0,     "TRKS", "Track Settings" },
    { 0x0BF999E7, 0, 0,     "LTXT", "Lot Description" },
    { 0x0C1FE246, 0, "xml", "XMOL", "Mesh Overlay XML" },
    { 0x0C560F39, 0, 0,     "BINX", "Binary Index" },
    { 0x0C7E9A76, 0, "jpg", "JPG",  "JPEG Image" },
    { 0x0C900FDB, 0, 0,     "UNK",  "UNK: 0x0C900FDB" },
    { 0x0C93E3DE, 0, "xml", "XFMD", "Face Modifier XML" },
    { 0x104F6A6E, 0, 0,     "BNFO", "Business Info" },
    { 0x1C4A276C, 0, "6tx", "TXTR", "Texture Image" },
    { 0x2026960B, 0, "mp3", "MP3",  "mp3 or xa Sound File" },
    { 0x25232B11, 0, "5sc", "SCEN", "Scene Node" },
    { 0x2A51171B, 0, 0,     "3ARY", "3D Array" },
    { 0x2C1FD8A1, 0, "xml", "XTOL", "Texture Overlay XML" },
    { 0x2C30E040, 0, "jpg", "THUB", "Fence Arch Thumbnail" },
    { 0x2C310F46, 0, 0,     "POPS", "Popups" },
    { 0x2C43CBD4, 0, "jpg", "THUB", "Foundation or Pool Thumbnail" },
    { 0x2C488BCA, 0, "jpg", "THUB", "Dormer Thmbnail" },
    { 0x2CB230B8, 0, "xml", "XFNC", "Fence XML" },
    { 0x3053CF74, 0, 0,     "SCOR", "Sim: Scores" },
    { 0x42434F4E, 1, 0,     "BCON", "Behaviour Constant" },
    { 0x42484156, 1, 0,     "BHAV", "Behaviour Function" },
    { 0x424D505F, 1, "bmp", "BMP",  "Bitmap Image" },
    { 0x43415453, 0, 0,     "CATS", "Catalog String" },
    { 0x43545353, 0, 0,     "CTSS", "Catalog Description" },
    { 0x44475250, 0, 0,     "DGRP", "Layered Image" },
    { 0x46414345, 0, 0,     "FACE", "Face Properties" },
    { 0x46414D49, 0, 0,     "FAMI", "Family Information" },
    { 0x46414D68, 0, 0,     "FAMH", "Family Unknown" },
    { 0x46434E53, 0, 0,     "FCNS", "Function" },
    { 0x46574156, 1, 0,     "FWAV", "Audio Reference" },
    { 0x474C4F42, 1, 0,     "GLOB", "Global Data" },
    { 0x484F5553, 0, 0,     "HOUS", "House Descriptor" },
    { 0x49596978, 0, "5tm", "TXMT", "Material Definition" },
    { 0x49FF7D76, 0, 0,     "WRLD", "World Database" },
    { 0x4B58975B, 0, 0,     "LTTX", "Lot Texture" },
    { 0x4C158081, 0, "xml", "XSTN", "Skin Tone XML" },
    { 0x4C697E5A, 0, 0,     "MMAT", "Material Override" },
    { 0x4D51F042, 0, "5cs", "CINE", "Cinematic Scene" },
    { 0x4D533EDD, 0, "jpg", "JPG",  "JPEG Image" },
    { 0x4DCADB7E, 0, "xml", "XFLR", "Floor XML" },
    { 0x4E474248, 0, 0,     "NGBH", "Neighborhood/Memory" },
    { 0x4E524546, 1, 0,     "NREF", "Name Reference" },
    { 0x4E6D6150, 0, 0,     "NMAP", "Name Map" },
    { 0x4F424A44, 1, 0,     "OBJD", "Object Data" },
    { 0x4F424A66, 1, 0,     "OBJf", "Object Functions" },
    { 0x4F626A4D, 0, 0,     "OBJM", "Object Material?" },
    { 0x4F6FD33D, 0, 0,     "INIT", "Inventory Item" },
    { 0x50414C54, 0, 0,     "PALT", "Image Color Palette (Version 1)" },
    { 0x50455253, 0, 0,     "UNK",  "UNK: 0x50455253" },
    { 0x504F5349, 0, 0,     "POSI", "Stack Script" },
    { 0x50544250, 0, 0,     "PTBP", "Package Text" },
    { 0x53494D49, 0, 0,     "SIMI", "Sim Information" },
    { 0x534C4F54, 1, 0,     "SLOT", "Slot File" },
    { 0x53505232, 1, 0,     "SPR2", "Sprites" },
    { 0x53545223, 1, 0,     "STR#", "Text Lists" },
    { 0x54415454, 1, 0,     "TATT", "TATT" },
    { 0x54505250, 1, 0,     "TPRP", "Edith Simantics Behaviour Labels" },
    { 0x5452434E, 1, 0,     "TRCN", "Behaviour Constant Labels" },
    { 0x54524545, 1, 0,     "TREE", "Edith Flowchart Trees" },
    { 0x54535053, 0, 0,     "GROP", "Groups Cache" },
    { 0x54544142, 1, 0,     "TTAB", "Pie Menu Functions" },
    { 0x54544173, 1, 0,     "TTAs", "Pie Menu Strings" },
    { 0x584D544F, 0, 0,     "XMTO", "Material Object?" },
    { 0x584F424A, 0, "xml", "XOBJ", "Object XML" },
    { 0x61754C1B, 1, 0,     "SLUA", "SimPE Object Lua" },
    { 0x6A97042F, 0, "5el", "LGHT", "Lighting (Environment Cube Light)" },
    { 0x6B943B43, 0, 0,     "LOTG", "Lot Terrain Geometry" },
    { 0x6C4F359D, 0, 0,     "COLL", "Collection" },
    { 0x6C589723, 0, 0,     "UNK",  "UNK: 0x6C589723" },
    { 0x6C93B566, 0, "xml", "XFNU", "Face Neural XML" },
    { 0x6D619378, 0, "xml", "XNGB", "Neighborhood Object XML" },
    { 0x6D814AFE, 0, "xml", "WNTT", "Wants Tree Item" },
    { 0x6F626A74, 0, 0,     "OBJT", "Object" },
    { 0x7181C501, 0, 0,     "PUNK", "Pet Unknown" },
    { 0x7B1ACFCD, 0, 0,     "UNK",  "UNK: 0x7B1ACFCD" },
    { 0x7BA3838C, 0, "5gn", "GMND", "Geometric Node" },
    { 0x856DDBAC, 0, "jpg", "IMG",  "jpg/tga/png Image" },
    { 0x8A84D7B0, 0, 0,     "WLAY", "Wall Layer" },
    { 0x8B0C79D6, 0, 0,     "UNK",  "UNK: 0x8B0C79D6" },
    { 0x8C1580B5, 0, "xml", "XHTN", "Hair Tone XML" },
    { 0x8C31125E, 0, "jpg", "THUB", "Wall Thumbnail" },
    { 0x8C311262, 0, "jpg", "THUB", "Floor Thumbnail" },
    { 0x8C3CE95A, 0, "jpg", "JPG",  "JPEG Image" },
    { 0x8C870743, 0, 0,     "FAMT", "Family Ties" },
    { 0x8C93BF6C, 0, "xml", "XFRG", "Face Region XML" },
    { 0x8C93E35C, 0, "xml", "XFCH", "Face Arch XML" },
    { 0x8CC0A14B, 0, 0,     "SDBA", "UNK: 0x8CC0A14B" },
    { 0x8DB5E4C2, 0, 0,     "FXSD", "FX Sound" },
    { 0x9012468A, 0, 0,     "GLUA", "Global Object Lua" },
    { 0x9012468B, 0, 0,     "OLUA", "Object Lua" },
    { 0xA2E3D533, 0, 0,     "KEYD", "Accelerator Key Definitions" },
    { 0xAACE2EFB, 0, 0,     "SDSC", "Sim Description" },
    { 0xAB4BA572, 0, 0,     "FPST", "Fence Post Layer" },
    { 0xAB9406AA, 0, 0,     "UNK",  "UNK: 0xAB9406AA" },
    { 0xABCB5DA4, 0, 0,     "NHTG", "Neighborhood Terrain Geometry" },
    { 0xABD0DC63, 0, 0,     "NHTR", "Neighborhood Terrain" },
    { 0xAC06A66F, 0, "5lf", "LGHT", "Lighting (Linear Fog Light)" },
    { 0xAC06A676, 0, "5ds", "LGHT", "Lighting (Draw State Light)" },
    { 0xAC2950C1, 0, "jpg", "THUB", "Thumbnail" },
    { 0xAC4F8687, 0, "5gd", "GMDC", "Geometric Data Container" },
    { 0xAC506764, 0, 0,     "3IDR", "3D ID Referencing File" },
    { 0xAC598EAC, 0, 0,     "AGED", "Age Data" },
    { 0xAC8A7A2E, 0, 0,     "IDNO", "ID Number" },
    { 0xACA8EA06, 0, "xml", "XROF", "Roof XML" },
    { 0xACE46235, 0, 0,     "RTEX", "Road Texture" },
    { 0xADEE8D84, 0, "nlo", "NLO",  "Light Override" },
    { 0xBA353CE1, 0, 0,     "TSSG", "TSSG System" },
    { 0xBC66BAEC, 0, 0,     "UNK",  "UNK: 0xBC66BAEC" },
    { 0xC9C81B9B, 0, "5dl", "LGHT", "Lighting (Directional Light)" },
    { 0xC9C81BA3, 0, "5al", "LGHT", "Lighting (Ambient Light)" },
    { 0xC9C81BA9, 0, "5pl", "LGHT", "Lighting (Point Light)" },
    { 0xC9C81BAD, 0, "5sl", "LGHT", "Lighting (Spot Light)" },
    { 0xCAC4FC40, 0, 0,     "SMAP", "String Map" },
    { 0xCB4387A1, 0, 0,     "VERT", "Vertex" },
    { 0xCC2A6A34, 0, 0,     "SCID", "Sim Creation Index" },
    { 0xCC30CDF8, 0, "jpg", "THUB", "Fence Thumbnail" },
    { 0xCC364C2A, 0, 0,     "SREL", "Sim Relations" },
    { 0xCC44B5EC, 0, "jpg", "THUB", "Modular Stair Thumbnail" },
    { 0xCC489E46, 0, "jpg", "THUB", "Roof Thumbnail" },
    { 0xCC48C51F, 0, "jpg", "THUB", "Chimney Thumbnail" },
    { 0xCCA8E925, 0, "xml", "XOBJ", "Object XML" },
    { 0xCCCEF852, 0, 0,     "LxNR", "Facial Structure" },
    { 0xCD7FE87A, 0,"matshad","MATSHAD","Maxis Material Shader" },
    { 0xCD8B6498, 0, 0,     "UNK",  "UNK: 0xCD8B6498" },
    { 0xCD95548E, 0, 0,     "SWAF", "Sim Wants and Fears" },
    { 0xCDB467B8, 0, 0,     "CREG", "Content Registry" },
    { 0xD1954460, 0, 0,     "PBOP", "Pet Body Options" },
    { 0xE519C933, 0, "5cr", "CRES", "Resource Node" },
    { 0xE86B1EEF, 0, "dir", "CLST", "Directory of Compressed Files" },
    { 0xEA5118B0, 0, "fx",  "FX",   "Effects List" },
    { 0xEBCF3E27, 0, 0,     "GZPS", "Property Set" },
    { 0xEBFEE33F, 0, 0,     "SDNA", "Sim DNA" },
    { 0xEBFEE342, 0, 0,     "VERS", "Version Information" },
    { 0xEBFEE345, 0, 0,     "AUDT", "Audio Test" },
    { 0xEC3126C4, 0, "jpg", "THUB", "Terrain Thumbnail" },
    { 0xEC44BDDC, 0, 0,     "UNK",  "UNK: 0xEC44BDDC" },
    { 0xED534136, 0, "6li", "LIFO", "Large Image File" },
    { 0xED7D7B4D, 0, "xml", "XWNT", "Wants XML" },
    { 0xFA1C39F7, 0, 0,     "OBJT", "Object" },
    { 0xFB00791E, 0, "5an", "ANIM", "Animation Resource" },
    { 0xFC6EB1F7, 0, "5sh", "SHPE", "Shape" },
    { 0xFFFFFFFF, 0, 0,     "----", "--- User Defined ---" }
};

*/
