objects = hairMain.o hairProcess.o \
					../library/DBPF.o ../library/DBPF_2.o \
					../library/DBPFcompress.o ../library/DBPF_types.o \
					../library/DBPF_3IDR.o ../library/DBPF_GZPS.o \
					../library/DBPF_XHTN.o ../library/DBPF_TXMT.o \
					../library/DBPF_TXTR.o ../library/DBPF_STR.o \
					../library/DBPF_resource.o ../library/DBPF_CPF.o \
					../library/DBPF_RCOL.o ../library/DBPF_CPFresource.o \
					../library/DBPF_byteStreamFunctions.o benrq_dbpf.o

hair_debloater : $(objects)
	g++ -o hair_debloater $(objects)

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
	g++	-c -o benrq_dbpf.o ../../benrq/dbpf.cpp

.PHONY : clean
clean :
	rm hair_debloater $(objects)
