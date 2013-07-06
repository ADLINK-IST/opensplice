ifeq ($(INCLUDE_FILTERS),yes)
link:
	(cd wrappers; make compile)
	(cd filters; make compile)
	(cd utilities; make link)

compile:
	(cd wrappers; make compile)
	(cd filters; make compile)
	(cd utilities; make compile)

clean:
	(cd wrappers; make clean)
	(cd filters; make clean)
	(cd utilities; make clean)

qac:

gcov:

analyse:

else # INCLUDE_FILTERS=no

link:
	(cd wrappers; make compile)
	(cd utilities; make link)
compile:
	(cd wrappers; make compile)
	(cd utilities; make compile)
clean:
	(cd wrappers; make clean)
	(cd utilities; make clean)
qac:
gcov:
analyse:

endif
