HOW TO REGENERATE FROM THE idl.yy

use yacc on ultra5, remove tabs and rename the generated files, then patch
the files.

/usr/ccs/bin/yacc -d idl.yy
expand y.tab.h > y_tab.hh
expand y.tab.c > y_tab.cpp
patch < y_tab.hh.patch
patch < y_tab.cpp.patch

NOTE : if there is no other way and you must manually alter the cpp or hh files KEEP THE PATCH FILES UP TO DATE.
