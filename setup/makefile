ifeq ($(INCLUDE_FILTERS),yes)
link:
	(cd wrappers; make compile)
	(cd filters; make compile)

compile:
	(cd wrappers; make compile)
	(cd filters; make compile)

clean:
	(cd wrappers; make clean)
	(cd filters; make clean)

qac:

gcov:

analyse:

else # INCLUDE_FILTERS=no

link:
	(cd wrappers; make compile)
compile:
	(cd wrappers; make compile)
clean:
	(cd wrappers; make clean)
qac:
gcov:
analyse:

endif
