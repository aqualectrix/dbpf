objects = dbpf.o

libbenrq_dbpf.a : $(objects)
	ar rcs libbenrq_dbpf.a $(objects)

dbpf.o : dbpf.h

.PHONY : clean
clean :
	rm libbenrq_dbpf.* $(objects)
