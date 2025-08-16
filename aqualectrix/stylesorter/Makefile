LDFLAGS = -L ../../CatOfEvilGenius/library -L ../../benrq/
LDLIBS = -l CatOfEvilGenius_dbpf -l benrq_dbpf

stylesorter : stylesorter.py bodyShopProcessWrapper.py libBodyShopProcess.so
  # Windows-specific
	pyinstaller --noconfirm stylesorter.py --add-binary="libBodyShopProcess.so;."

libBodyShopProcess.so: shared_bodyShopProcess.o
	g++ -shared -static-libgcc -static-libstdc++ -o libBodyShopProcess.so \
		shared_bodyShopProcess.o \
	    $(LDFLAGS) $(LDLIBS)

shared_bodyShopProcess.o: bodyShopProcess.cpp \
	../../CatOfEvilGenius/library/DBPF.h \
	../../CatOfEvilGenius/library/DBPF_types.h \
	../../CatOfEvilGenius/library/DBPF_BINX.h \
	../../CatOfEvilGenius/library/DBPF_CPF.h \
	../../CatOfEvilGenius/library/DBPF_GZPS.h
	g++ -c -fPIC -o shared_bodyShopProcess.o bodyShopProcess.cpp

.PHONY: clean
clean :
	rm -rf *.o *.so *.spec dist build __pycache__
