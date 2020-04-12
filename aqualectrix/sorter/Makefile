LDFLAGS = -L ../../CatOfEvilGenius/library -L ../../benrq/
LDLIBS = -l CatOfEvilGenius_dbpf -l benrq_dbpf

sortindex : sortindex.py sortProcessWrapper.py libSortProcess.so
	pyinstaller sortindex.py

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
	rm -rf sortindex sortindex.exe *.o *.so dist build __pycache__
