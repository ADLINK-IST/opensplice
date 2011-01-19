$(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcc: filter.c
	make filter CC=gcc
	rm -f $(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcc
	mkdir -p $(OSPL_HOME)/exec/$(SPLICE_HOST)
	cp `pwd`/filter $(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcc
	rm -f filter

clean:
	rm -f $(OSPL_HOME)/exec/$(SPLICE_HOST)/filter_gcc
	rm -f filter
