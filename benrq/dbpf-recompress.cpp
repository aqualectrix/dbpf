/*
 * Demonstration program for the DBPF library: a command-line utility
 * to recompress package files. Not very well written or commented
 * at the moment. Version 20070601.
 *
 * This file is Copyright 2007 Ben Rudiak-Gould. Anyone may use it
 * under the terms of the GNU General Public License, version 2 or
 * (at your option) any later version. This code comes with
 * NO WARRANTY. Make backups!
 */

#include "dbpf.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>

//#include <assert.h>
#define assert(expr) do{}while(0)


typedef unsigned char byte;


static int stdio_read(void* ctx, int start, int length, void* buf, const char** error)
{
    FILE* f = (FILE*)ctx;
    fseek(f, start, SEEK_SET);
    if (fread(buf, 1, length, f) == (size_t)length) {
        return 0;
    } else {
        *error = ferror(f) ? "read error" : "unexpected end of file while reading";
        return -1;
    }
}

static int stdio_write(void* ctx, int start, int length, const void* buf, const char** error)
{
    FILE* f = (FILE*)ctx;
    fseek(f, start, SEEK_SET);
    if (fwrite(buf, 1, length, f) == (size_t)length) {
        return 0;
    } else {
        *error = "write error";
        return -1;
    }
}

static int stdio_close(void* ctx)
{
    return fclose((FILE*)ctx);
}

DBPF* dbpf_open_stdio(FILE* f, const char** error)
{
    return dbpf_open(f, stdio_read, stdio_write, stdio_close, error);
}


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

template<class T>
class auto_mydelete {
public:
    T* val;
    auto_mydelete(T* newval) { val = newval; }
    ~auto_mydelete() { mydelete(val); }
    T* operator=(T* newval) { return val = newval; }
    bool operator!() { return !val; }
    T& operator[](int i) { return val[i]; }
    operator T*() { return val; }
    T* keep() { T* temp = val; val = 0; return temp; }
};

class auto_close_dbpf
{
public:
    DBPF* val;
    auto_close_dbpf(DBPF* newval) { val = newval; }
    ~auto_close_dbpf() { dbpf_close(val); }
    bool operator!() { return !val; }
    operator DBPF*() { return val; }
};


bool verify(FILE* f, FILE* g)
{
    const char* error = "??? unknown error (bug)";

    auto_close_dbpf dbpf1 = dbpf_open_stdio(f, &error);
    if (!dbpf1) {
        printf("  *** verify failed: reopen of old file failed: %s\n", error);
        return false;
    }

    auto_close_dbpf dbpf2 = dbpf_open_stdio(g, &error);
    if (!dbpf2) {
        printf("  *** verify failed: reopen of new file failed: %s\n", error);
        return false;
    }

    int entry_count = dbpf_get_entry_count(dbpf1);
    if (entry_count != dbpf_get_entry_count(dbpf2)) {
        printf("  *** verify failed: entry count mismatch\n");
        return false;
    }

    const dbpf_entry* entries1 = dbpf_get_entries(dbpf1);
    const dbpf_entry* entries2 = dbpf_get_entries(dbpf2);

    int max_size = 0;

    int i;
    for (i = 0; i < entry_count; ++i) {
        const dbpf_entry *a = &entries1[i], *b = &entries2[i];
        if (a->type_id != b->type_id || a->group_id != b->group_id
            || a->instance_id != b->instance_id
            || a->instance_id_2 != b->instance_id_2
            || a->size != b->size)
        {
            printf("  *** verify failed: file metadata mismatch\n");
            return false;
        }
        if (max_size < a->size) max_size = a->size;
    }

    auto_mydelete<byte> buf = mynew<byte>(max_size * 2);

    for (i = 0; i < entry_count; ++i) {
        const dbpf_entry *a = &entries1[i];
        byte* buf2 = buf + a->size;

        if (dbpf_read(dbpf1, i, buf, &error) < 0) {
            printf("  *** verify failed: read of old file failed: %s\n", error);
            return false;
        }
        if (dbpf_read(dbpf2, i, buf2, &error) < 0) {
            printf("  *** verify failed: read of new file failed: %s\n", error);
            return false;
        }
        if (memcmp(buf, buf2, a->size) != 0) {
            printf("  *** verify failed: old and new files are different\n");
            return false;
        }
    }

    return true;
}


struct write_info {
    DBPF* dbpf;
    const dbpf_entry* entries;
    byte* p;
};

const byte* get_data(void* ctx, int entry_index, const char** error)
{
    write_info* wi = (write_info*)ctx;
    const dbpf_entry* e = &wi->entries[entry_index];
    if (e->size == 0) return (const byte*)"";
    wi->p = (byte*)realloc(wi->p, e->size);
    if (dbpf_read(wi->dbpf, entry_index, wi->p, error) >= 0) {
/*
        byte buf[65];
        memset(buf, 'z', 65);
        if (dbpf_read_64bytes(wi->dbpf, entry_index, buf, error) >= 0) {
            if (buf[64] != 'z') abort();
            if (memcmp(buf, wi->p, 64)) abort();
//            for (int j=0; j<64; ++j)
//                putchar(buf[j] < 32 || buf[j] >= 127 ? '.' : buf[j]);
//            putchar('\n');
        } else {
            if (e->size >= 64) abort();
        }
*/
        return wi->p;
    } else {
        return 0;
    }
}


bool recompress(const char* srcname, const char* dstname, bool decompress)
{
    FILE* f = fopen(srcname, "rb");
    if (!f) {
        puts("  *** open failed\n");
        return false;
    }

    const char* error = "??? unknown error (bug)";
    auto_close_dbpf dbpf_in = dbpf_open_stdio(f, &error);
    if (!dbpf_in) {
        printf("  *** open failed: %s\n", error);
        return false;
    }

    FILE* g = fopen(dstname, "w+b");
    if (!g) {
        puts("  *** output open failed\n");
        return false;
    }

    unsigned char hdr[DBPF_EMPTY_ARCHIVE_LENGTH];
    dbpf_create(hdr);
    fwrite(hdr, 1, DBPF_EMPTY_ARCHIVE_LENGTH, g);

    auto_close_dbpf dbpf_out = dbpf_open_stdio(g, &error);
    if (!dbpf_out) {
        printf("  *** output open failed: %s\n", error);
        return false;
    }

    int entry_count = dbpf_get_entry_count(dbpf_in);
    const dbpf_entry* entries = dbpf_get_entries(dbpf_in);

    auto_mydelete<dbpf_entry> new_entries = mynew<dbpf_entry>(entry_count);
    memcpy(new_entries, entries, entry_count * sizeof(dbpf_entry));
    for (int i = 0; i < entry_count; ++i) {
        new_entries[i] = entries[i];
        new_entries[i].write_disposition = decompress ? dbpf_write_uncompressed : dbpf_write_compressed;
    }

    write_info wi = { dbpf_in, entries, 0 };
    bool success = (dbpf_write(dbpf_out, new_entries, entry_count, &wi, get_data, &error) >= 0);
    if (wi.p) free(wi.p);
    if (!success) {
        printf("  *** rewrite failed: %s\n", error);
        return false;
    }

    return verify(f,g);
}


bool go(const char* name, bool decompress)
{
    printf("%s\n", name);

    auto_mydelete<char> name_new = mynew<char>(strlen(name) + 6);
    auto_mydelete<char> name_old = mynew<char>(strlen(name) + 6);
    sprintf(name_new, "%s.$new", name);
    sprintf(name_old, "%s.$old", name);

    if (recompress(name, name_new, decompress)) {
        if (rename(name, name_old) < 0) {
            printf("  *** renaming \"%s\" to \"%s\" failed; cleaning up\n", name, (char*)name_old);
            remove(name_new);
            return false;
        }
        if (rename(name_new, name) < 0) {
            printf("  *** renaming \"%s\" to \"%s\" failed; please check \"%s\" and \"%s\" and rename as appropriate\n", (char*)name_new, name, (char*)name_new, (char*)name_old);
            return false;
        }
        if (remove(name_old) < 0) {
            printf("  *** removing \"%s\" failed; please delete manually\n", (char*)name_old);
        }
    } else {
        remove(name_new);
        return false;
    }

    return true;
}


int main(int argc, char** argv)
{
    bool decompress = false;
    if (argc >= 2 && strcmp(argv[1], "-d") == 0) {
        decompress = true;
        --argc;
        ++argv;
    }
    if (argc < 2) {
        printf("usage: dbpf-recompress [-d] a.package b.package ...\n"
               "  -d  decompress all files instead of recompressing\n");
        return 0;
    }
    for (int i=1; i<argc; ++i)
        if (!go(argv[i], decompress))
            return 1;
    return 0;
}
