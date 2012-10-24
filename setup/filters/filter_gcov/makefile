$(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcov: filter_gcov.c
	make filter_gcov CC=gcc CFLAGS=-g
	rm -f $(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcov
	mkdir -p $(OSPL_HOME)/exec/$(SPLICE_HOST)
	cp `pwd`/filter_gcov $(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcov
	rm -f filter_gcov

clean:
	rm -f $(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcov
	rm -f filter_gcov
