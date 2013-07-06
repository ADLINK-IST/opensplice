ifneq (,$(or $(findstring win32,$(SPLICE_HOST)),$(findstring win64,$(SPLICE_HOST))))
compile:
	(cd wincmd; make)
	(cd winln; make)

clean:
	(cd wincmd; make clean)
	(cd winln; make clean)
else
compile:
clean:
endif
