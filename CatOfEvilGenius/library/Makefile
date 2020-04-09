objects = DBPF.o DBPF_types.o DBPF_resource.o \
					DBPF_2.o DBPFcompress.o DBPF_byteStreamFunctions.o \
					DBPF_CPF.o DBPF_CPFresource.o \
					DBPF_3IDR.o DBPF_GZPS.o DBPF_RCOL.o DBPF_STR.o \
					DBPF_TXMT.o DBPF_TXTR.o DBPF_XHTN.o

libCatOfEvilGenius_dbpf.a : $(objects)
	ar rcs libCatOfEvilGenius_dbpf.a $(objects)

# top-level definitions and utilities
DBPF.o : DBPF.h DBPF_types.h DBPF_resource.h
DBPF_types.o : DBPF_types.h
DBPF_resource.o : DBPF_resource.h DBPF_types.h DBPFcompress.h

# i/o and byte manipulation
DBPF_2.o : DBPF.h DBPFcompress.h DBPF_types.h DBPF_3IDR.h \
           DBPF_GZPS.h DBPF_XHTN.h DBPF_TXMT.h DBPF_TXTR.h \
					 DBPF_STR.h
DBPFcompress.o : DBPF_byteStreamFunctions.h
DBPF_byteStreamFunctions.o : DBPF_byteStreamFunctions.h

# CPF - base for key/value store resources
DBPF_CPF.o : DBPF_CPF.h DBPF_byteStreamFunctions.h
DBPF_CPFresource.o : DBPF_CPFresource.h DBPF_types.h \
                     DBPF_byteStreamFunctions.h

# Resource type implementations
DBPF_3IDR.o : DBPF_3IDR.h DBPF_byteStreamFunctions.h DBPF_types.h
DBPF_GZPS.o : DBPF_GZPS.h
DBPF_RCOL.o : DBPF_RCOL.h DBPF_byteStreamFunctions.h \
              DBPF_types.h
DBPF_STR.o : DBPF_STR.h DBPF_byteStreamFunctions.h
DBPF_TXMT.o : DBPF_TXMT.h DBPF_types.h DBPF_byteStreamFunctions.h
DBPF_TXTR.o : DBPF_TXTR.h DBPF_types.h DBPF_byteStreamFunctions.h
DBPF_XHTN.o : DBPF_XHTN.h

.PHONY : clean
clean :
	rm libCatOfEvilGenius_dbpf.a $(objects)
