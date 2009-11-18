$(OSPL_HOME)/bin/ospl_wincmd: ospl_wincmd
	cp `pwd`/ospl_wincmd $(OSPL_HOME)/bin/ospl_wincmd

ospl_wincmd: ospl_wincmd.c
	gcc ospl_wincmd.c -o ospl_wincmd

clean:
	rm -f $(OSPL_HOME)/bin/ospl_wincmd
	rm -f ospl_wincmd
