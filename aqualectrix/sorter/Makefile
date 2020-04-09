objects = sorterMain.o sortProcess.o \
					benrq_dbpf.o

sorter : $(objects)
	$(MAKE) -C ../../CatOfEvilGenius/library libCatOfEvilGenius_dbpf.a
	g++ -o sorter $(objects) -L ../../CatOfEvilGenius/library -l CatOfEvilGenius_dbpf

sortProcess.o : ../../CatOfEvilGenius/library/DBPF.h \
                ../../CatOfEvilGenius/library/DBPF_types.h

benrq_dbpf.o :  ../../benrq/dbpf.cpp ../../benrq/dbpf.h
	g++	-c -o benrq_dbpf.o ../../benrq/dbpf.cpp

.PHONY: clean
clean :
	rm sorter sorter.exe $(objects)
