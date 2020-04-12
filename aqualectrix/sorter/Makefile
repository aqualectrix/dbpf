LDFLAGS = -L ../../CatOfEvilGenius/library -L ../../benrq/
LDLIBS = -l CatOfEvilGenius_dbpf -l benrq_dbpf

objects = sorterMain.o sortProcess.o \
					benrq_dbpf.o

sorter : $(objects) ../../CatOfEvilGenius/library/libCatOfEvilGenius_dbpf.a
	$(MAKE) -C ../../CatOfEvilGenius/library libCatOfEvilGenius_dbpf.a
	g++ -o sortindex $(objects) -L ../../CatOfEvilGenius/library -l CatOfEvilGenius_dbpf

sortProcess.o : ../../CatOfEvilGenius/library/DBPF.h \
                ../../CatOfEvilGenius/library/DBPF_types.h \
								../../CatOfEvilGenius/library/DBPF_BINX.h

benrq_dbpf.o :  ../../benrq/dbpf.cpp ../../benrq/dbpf.h
	g++	-c -o benrq_dbpf.o ../../benrq/dbpf.cpp

libSortProcess.so: shared_sortProcess.o
	g++ -shared -static-libgcc -static-libstdc++ -o libSortProcess.so shared_sortProcess.o \
	    $(LDFLAGS) $(LDLIBS)

shared_sortProcess.o: sortProcess.cpp \
	../../CatOfEvilGenius/library/DBPF.h \
	../../CatOfEvilGenius/library/DBPF_types.h \
	../../CatOfEvilGenius/library/DBPF_BINX.h
	g++ -c -fPIC -o shared_sortProcess.o sortProcess.cpp

.PHONY: clean
clean :
	rm sortindex sortindex.exe $(objects) *.o *.so
