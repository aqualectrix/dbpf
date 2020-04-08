objects = sorterMain.o sortProcess.o \
					../../CatOfEvilGenius/library/DBPF.o \
					../../CatOfEvilGenius/library/DBPF_resource.o \
					../../CatOfEvilGenius/library/DBPF_CPF.o \
					../../CatOfEvilGenius/library/DBPF_CPFresource.o \
					../../CatOfEvilGenius/library/DBPF_byteStreamFunctions.o \
					../../CatOfEvilGenius/library/DBPFcompress.o \
					../../CatOfEvilGenius/library/DBPF_types.o \
					benrq_dbpf.o \
					../../CatOfEvilGenius/library/DBPF_2.o \
					../../CatOfEvilGenius/library/DBPF_TXMT.o \
					../../CatOfEvilGenius/library/DBPF_3IDR.o \
					../../CatOfEvilGenius/library/DBPF_TXTR.o \
					../../CatOfEvilGenius/library/DBPF_STR.o \
					../../CatOfEvilGenius/library/DBPF_RCOL.o

sorter : $(objects)
	g++ -o sorter $(objects)

sortProcess.o : ../../CatOfEvilGenius/library/DBPF.h \
                ../../CatOfEvilGenius/library/DBPF_types.h

benrq_dbpf.o :  ../../benrq/dbpf.cpp ../../benrq/dbpf.h
	g++	-c -o benrq_dbpf.o ../../benrq/dbpf.cpp

.PHONY: clean
clean :
	rm sorter sorter.exe $(objects)
