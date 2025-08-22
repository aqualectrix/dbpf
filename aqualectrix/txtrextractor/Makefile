LDFLAGS = -L ../../CatOfEvilGenius/library -L ../../benrq/
LDLIBS = -l CatOfEvilGenius_dbpf -l benrq_dbpf

txtrextractor : txtrextractor.py txtrExtractProcessWrapper.py libTxtrExtractProcess.so
  # Windows-specific
	pyinstaller --noconfirm txtrextractor.py --add-binary="libTxtrExtractProcess.so;."

libTxtrExtractProcess.so: shared_txtrExtractProcess.o
	g++ -shared -static-libgcc -static-libstdc++ -o libTxtrExtractProcess.so \
		shared_txtrExtractProcess.o \
	    $(LDFLAGS) $(LDLIBS)

shared_txtrExtractProcess.o: txtrExtractProcess.cpp \
	../../CatOfEvilGenius/library/DBPF.h \
	../../CatOfEvilGenius/library/DBPF_types.h \
	../../CatOfEvilGenius/library/DBPF_TXTR.h
	g++ -c -fPIC -o shared_txtrExtractProcess.o txtrExtractProcess.cpp

.PHONY: clean
clean :
	rm -rf *.o *.so *.spec dist build __pycache__