# Configure paths for EXPAT
# Owen Taylor     97-11-3

dnl AM_PATH_EXPAT([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for EXPAT, and define EXPAT_CFLAGS and EXPAT_LIBS
dnl
AC_DEFUN([AM_PATH_EXPAT],
[dnl 
dnl Get the cflags and libraries from the expat-config script
dnl

AC_ARG_WITH(expat-prefix,
    AC_HELP_STRING([--with-expat-prefix=PFX],
              	    [LOG4C: Prefix where EXPAT is installed (defaults to just
                    looking in the standard library locations).  If
                    --without-expat is set to yes then this option has
                    no effect)]))
                    
AC_ARG_ENABLE(expattest,
    AC_HELP_STRING([--disable-expattest],
      	      	    [LOG4C: Do not try to compile and run a test EXPAT program.
                    (default is no).  If without-expat is set to yes this
                    option has no effect.]),
		    , enable_expattest=yes)

  if test x$with_expat_prefix != x ; then
    EXPAT_CFLAGS="-I$with_expat_prefix/include"
    EXPAT_LIBS="-L$with_expat_prefix/lib"
  fi

  min_expat_version=ifelse([$1], ,1.95.1, $1)
dnl  AC_MSG_NOTICE([value of with_expat_prefix=$with_expat_prefix \
dnl    value of with_expat=$with_expat])
  AC_MSG_CHECKING(for EXPAT - version >= $min_expat_version)
 

  EXPAT_CFLAGS="$EXPAT_CFLAGS"
  EXPAT_LIBS="$EXPAT_LIBS -lexpat"

  if test "x$enable_expattest" = "xyes" ; then
    ac_save_CFLAGS="$CFLAGS"
    ac_save_LIBS="$LIBS"
    CFLAGS="$CFLAGS $EXPAT_CFLAGS"
    LIBS="$EXPAT_LIBS $LIBS"
dnl
dnl Now check if the installed EXPAT is sufficiently new. (Also sanity
dnl checks the results of expat-config to some extent
dnl
    rm -f conf.expattest
    AC_TRY_RUN([
#include <expat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int 
main ()
{
   int expat_major, expat_minor, expat_micro;
   int major, minor, micro;
   char *tmp_expat_version;
   char *tmp_version;

   system ("touch conf.expattest");

   /* HP/UX 9 (%@#!) writes to sscanf strings */
   tmp_expat_version = strdup(XML_ExpatVersion());
   if (sscanf(tmp_expat_version, "expat_%d.%d.%d", &expat_major, &expat_minor, &expat_micro) != 3) {
     printf("%s, bad expat version string\n", XML_ExpatVersion());
     exit(1);
   }

   /* HP/UX 9 (%@#!) writes to sscanf strings */
   tmp_version = strdup("$min_expat_version");
   if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_expat_version");
     exit(1);
   }

   if ((expat_major > major) ||
      ((expat_major == major) && (expat_minor > minor)) ||
      ((expat_major == major) && (expat_minor == minor) && (expat_micro >= micro)))
   {
        return 0;
   }
   else
   {
        printf("\n*** An old version of EXPAT (%d.%d.%d) was found.\n",
               expat_major, expat_minor, expat_micro);
        printf("*** You need a version of EXPAT newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the expat-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of EXPAT, but you can also set the EXPAT_CONFIG environment to point to the\n");
        printf("*** correct copy of expat-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
   }
   return 1;
}
],, no_expat=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
  fi

  if test "x$no_expat" = x ; then
     AC_MSG_RESULT(yes)
     use_expat=yes
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     use_expat=no
     if test -f conf.expattest ; then
        :
     else
          echo "*** Could not run EXPAT test program, checking why..."
          CFLAGS="$CFLAGS $EXPAT_CFLAGS"
          LIBS="$LIBS $EXPAT_LIBS"
          AC_TRY_LINK([
#include <expat.h>
#include <stdio.h>
],      [ return (XML_ExpatVersion()); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding EXPAT or finding the wrong"
          echo "*** version of EXPAT. If it is not finding EXPAT you can specify"
          echo "*** an install location using --with-expat-prefix option to"
          echo "*** configure. You can also set the "
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***" 
          echo "*** Log4C will still run without EXPAT--it uses some bundled"
          echo "*** lex/yacc code to parse the configuration file"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means EXPAT was incorrectly installed"
          echo "*** or that you have moved EXPAT since it was installed. In the latter case, you"
          echo "*** may want to edit the expat-config script: '$EXPAT_CONFIG'"
          echo "*** Log4C will still run without EXPAT--it uses some bundled"
          echo "*** lex/yacc code to parse the configuration file"])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
     fi
     EXPAT_CFLAGS=""
     EXPAT_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(EXPAT_CFLAGS)
  AC_SUBST(EXPAT_LIBS)
  rm -f conf.expattest
])
