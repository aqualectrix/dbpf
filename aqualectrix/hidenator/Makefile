LDFLAGS = -L ../../CatOfEvilGenius/library -L ../../benrq/
LDLIBS = -l CatOfEvilGenius_dbpf -l benrq_dbpf

objects = hideMain.o hideProcess.o

hide : $(objects)
	g++		-o hide $(objects) $(LDFLAGS) $(LDLIBS)

hideMain.o : hideMain.cpp

hideProcess.o : ../../CatOfEvilGenius/library/DBPF.h \
						    ../../CatOfEvilGenius/library/DBPF_types.h \
						    ../../CatOfEvilGenius/library/DBPF_GZPS.h
