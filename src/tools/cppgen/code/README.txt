HOW TO REGENERATE FROM THE idl.yy

use yacc on ultra5, remove tabs and rename the generated files, then patch
the files.

/usr/ccs/bin/yacc -d idl.yy
expand y.tab.h > y_tab.hh
expand y.tab.c > y_tab.cpp
patch < y_tab.hh.patch
patch < y_tab.cpp.patch

NOTE : if there is no other way and you must manually alter the cpp or hh files KEEP THE PATCH FILES UP TO DATE.


How to generate lex_yy.c from idl.ll

NOTE: The version of flex we use is outdated. Because of that we need the
      lex_yy.cpp.patch patch file. Should we ever upgrade to a newer version.
      Please use different means of excluding unistd.h. e.g.
      Use the command line parameter: --nounistd
      Use the the option: %option nounistd

flex -olex_yy.cpp idl.ll
patch < lex_yy.cpp.patch
