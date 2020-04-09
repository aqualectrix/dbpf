objects = hairMain.o hairProcess.o \
					benrq_dbpf.o

hair_debloater : $(objects)
	$(MAKE) -C ../library libCatOfEvilGenius_dbpf.a
	g++    -o hair_debloater $(objects) -L ../library -l CatofEvilGenius_dbpf

hairMain.o : ../library/DBPF.h \
             ../library/DBPFcompress.h \
             ../library/DBPF_types.h \
             ../library/DBPF_3IDR.h \
						 ../library/DBPF_GZPS.h \
						 ../library/DBPF_XHTN.h \
						 ../library/DBPF_TXMT.h \
						 ../library/DBPF_TXTR.h \
						 ../library/DBPF_STR.h

hairProcess.o : ../library/DBPF.h \
                ../library/DBPFcompress.h \
                ../library/DBPF_types.h \
             		../library/DBPF_3IDR.h \
						 		../library/DBPF_GZPS.h \
						 		../library/DBPF_XHTN.h \
						 		../library/DBPF_TXMT.h \
						 		../library/DBPF_TXTR.h \
						 		../library/DBPF_STR.h

benrq_dbpf.o :  ../../benrq/dbpf.cpp ../../benrq/dbpf.h
	g++    -c -o benrq_dbpf.o ../../benrq/dbpf.cpp

.PHONY : clean
clean :
	rm hair_debloater* $(objects)
