LDFLAGS = -L ../../CatOfEvilGenius/library -L ../../benrq/
LDLIBS = -l CatOfEvilGenius_dbpf -l benrq_dbpf

texref : texref.py texRefProcessWrapper.py libTexRefProcess.so libGetTexIdProcess.so
  # Windows-specific
	pyinstaller texref.py --add-binary="libTexRefProcess.so;." --add-binary="libGetTexIdProcess.so;."
	# *nix-y systems
	# pyinstaller texref.py --add-binary="libTexRefProcess.so:." --add-binary="libGetTexIdProcess.so:."

libTexRefProcess.so: shared_texRefProcess.o
	g++ -shared -static-libgcc -static-libstdc++ -o libTexRefProcess.so shared_texRefProcess.o \
	    $(LDFLAGS) $(LDLIBS)

shared_texRefProcess.o: texRefProcess.cpp \
	../../CatOfEvilGenius/library/DBPF.h \
	../../CatOfEvilGenius/library/DBPF_types.h \
	../../CatOfEvilGenius/library/DBPF_TXMT.h \
	../../CatOfEvilGenius/library/DBPF_TXTR.h
	g++ -c -fPIC -o shared_texRefProcess.o texRefProcess.cpp

libGetTexIdProcess.so: shared_getTexIdProcess.o
	g++ -shared -static-libgcc -static-libstdc++ -o libGetTexIdProcess.so shared_getTexIdProcess.o \
	$(LDFLAGS) $(LDLIBS)

shared_getTexIdProcess.o: getTexIdProcess.cpp \
	../../CatOfEvilGenius/library/DBPF.h \
	../../CatOfEvilGenius/library/DBPF_types.h \
	../../CatOfEvilGenius/library/DBPF_TXMT.h
	g++ -c -fPIC -o shared_getTexIdProcess.o getTexIdProcess.cpp

.PHONY: clean
clean :
	rm -rf *.o *.so *.spec dist build __pycache__
