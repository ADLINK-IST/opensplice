# Configure paths for LOG4C
# Owen Taylor     97-11-3

dnl AM_PATH_LOG4C([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for LOG4C, and define LOG4C_CFLAGS and LOG4C_LIBS
dnl
AC_DEFUN(AM_PATH_LOG4C,
[dnl 
dnl Get the cflags and libraries from the log4c-config script
dnl
AC_ARG_WITH(log4c-prefix,[  --with-log4c-prefix=PFX   Prefix where LOG4C is installed (optional)],
            log4c_config_prefix="$withval", log4c_config_prefix="")
AC_ARG_WITH(log4c-exec-prefix,[  --with-log4c-exec-prefix=PFX Exec prefix where LOG4C is installed (optional)],
            log4c_config_exec_prefix="$withval", log4c_config_exec_prefix="")
AC_ARG_ENABLE(log4ctest, [  --disable-log4ctest       Do not try to compile and run a test LOG4C program],
		    , enable_log4ctest=yes)

  if test x$log4c_config_exec_prefix != x ; then
     log4c_config_args="$log4c_config_args --exec-prefix=$log4c_config_exec_prefix"
     if test x${LOG4C_CONFIG+set} != xset ; then
        LOG4C_CONFIG=$log4c_config_exec_prefix/bin/log4c-config
     fi
  fi
  if test x$log4c_config_prefix != x ; then
     log4c_config_args="$log4c_config_args --prefix=$log4c_config_prefix"
     if test x${LOG4C_CONFIG+set} != xset ; then
        LOG4C_CONFIG=$log4c_config_prefix/bin/log4c-config
     fi
  fi

  AC_PATH_PROG(LOG4C_CONFIG, log4c-config, no)
  min_log4c_version=ifelse([$1], ,0.99.7,$1)
  AC_MSG_CHECKING(for LOG4C - version >= $min_log4c_version)
  no_log4c=""
  if test "$LOG4C_CONFIG" = "no" ; then
    no_log4c=yes
  else
    LOG4C_CFLAGS=`$LOG4C_CONFIG $log4c_config_args --cflags`
    LOG4C_LIBS=`$LOG4C_CONFIG $log4c_config_args --libs`
    log4c_config_major_version=`$LOG4C_CONFIG $log4c_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    log4c_config_minor_version=`$LOG4C_CONFIG $log4c_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    log4c_config_micro_version=`$LOG4C_CONFIG $log4c_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
    if test "x$enable_log4ctest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $LOG4C_CFLAGS"
      LIBS="$LOG4C_LIBS $LIBS"
dnl
dnl Now check if the installed LOG4C is sufficiently new. (Also sanity
dnl checks the results of log4c-config to some extent
dnl
      rm -f conf.log4ctest
      AC_TRY_RUN([
#include <log4c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.log4ctest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = strdup("$min_log4c_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_log4c_version");
     exit(1);
   }

  if ((log4c_major_version != $log4c_config_major_version) ||
      (log4c_minor_version != $log4c_config_minor_version) ||
      (log4c_micro_version != $log4c_config_micro_version))
    {
      printf("\n*** 'log4c-config --version' returned %d.%d.%d, but LOG4C (%d.%d.%d)\n", 
             $log4c_config_major_version, $log4c_config_minor_version, $log4c_config_micro_version,
             log4c_major_version, log4c_minor_version, log4c_micro_version);
      printf ("*** was found! If log4c-config was correct, then it is best\n");
      printf ("*** to remove the old version of LOG4C. You may also be able to fix the error\n");
      printf("*** by modifying your LD_LIBRARY_PATH enviroment variable, or by editing\n");
      printf("*** /etc/ld.so.conf. Make sure you have run ldconfig if that is\n");
      printf("*** required on your system.\n");
      printf("*** If log4c-config was wrong, set the environment variable LOG4C_CONFIG\n");
      printf("*** to point to the correct copy of log4c-config, and remove the file config.cache\n");
      printf("*** before re-running configure\n");
    } 
  else if ((log4c_major_version != LOG4C_MAJOR_VERSION) ||
	   (log4c_minor_version != LOG4C_MINOR_VERSION) ||
           (log4c_micro_version != LOG4C_MICRO_VERSION))
    {
      printf("*** LOG4C header files (version %d.%d.%d) do not match\n",
	     LOG4C_MAJOR_VERSION, LOG4C_MINOR_VERSION, LOG4C_MICRO_VERSION);
      printf("*** library (version %d.%d.%d)\n",
	     log4c_major_version, log4c_minor_version, log4c_micro_version);
    }
  else
    {
      if ((log4c_major_version > major) ||
        ((log4c_major_version == major) && (log4c_minor_version > minor)) ||
        ((log4c_major_version == major) && (log4c_minor_version == minor) && (log4c_micro_version >= micro)))
      {
        return 0;
       }
     else
      {
        printf("\n*** An old version of LOG4C (%d.%d.%d) was found.\n",
               log4c_major_version, log4c_minor_version, log4c_micro_version);
        printf("*** You need a version of LOG4C newer than %d.%d.%d. The latest version of\n",
	       major, minor, micro);
        printf("***\n");
        printf("*** If you have already installed a sufficiently new version, this error\n");
        printf("*** probably means that the wrong copy of the log4c-config shell script is\n");
        printf("*** being found. The easiest way to fix this is to remove the old version\n");
        printf("*** of LOG4C, but you can also set the LOG4C_CONFIG environment to point to the\n");
        printf("*** correct copy of log4c-config. (In this case, you will have to\n");
        printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
        printf("*** so that the correct libraries are found at run-time))\n");
      }
    }
  return 1;
}
],, no_log4c=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_log4c" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$LOG4C_CONFIG" = "no" ; then
       echo "*** The log4c-config script installed by LOG4C could not be found"
       echo "*** If LOG4C was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LOG4C_CONFIG environment variable to the"
       echo "*** full path to log4c-config."
     else
       if test -f conf.log4ctest ; then
        :
       else
          echo "*** Could not run LOG4C test program, checking why..."
          CFLAGS="$CFLAGS $LOG4C_CFLAGS"
          LIBS="$LIBS $LOG4C_LIBS"
          AC_TRY_LINK([
#include <log4c.h>
#include <stdio.h>
],      [ return ((log4c_major_version) || (log4c_minor_version) || (log4c_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding LOG4C or finding the wrong"
          echo "*** version of LOG4C. If it is not finding LOG4C, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"
          echo "***" ],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means LOG4C was incorrectly installed"
          echo "*** or that you have moved LOG4C since it was installed. In the latter case, you"
          echo "*** may want to edit the log4c-config script: $LOG4C_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     LOG4C_CFLAGS=""
     LOG4C_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LOG4C_CFLAGS)
  AC_SUBST(LOG4C_LIBS)
  rm -f conf.log4ctest
])
