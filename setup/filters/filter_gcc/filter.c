
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <limits.h>

char line_base [1024][512];
FILE *gcc_log;
FILE *filter_log;

/* r_index implements a portable rindex function */
/* dependency with abstraction layer is not acceptable */
char *
r_index (
    const char *s,
    int c
    )
{
    const char *last = NULL;

    if (s == NULL) {
        return NULL;
    }
    while ((int)(*s) != 0) {
        if ((int)(*s) == c) {
            last = s;
        }
        s++;
    }
    return (char *)last;
}

int
skip_line (
    char *line
    )
{
    if (strstr (line, "does not support `long long\'")) {
	return 1;
    } else if (strstr (line, "does not support the `ll\' printf length modifier")) {
	return 1;
    } else if (strstr (line, "does not support the `ll\' scanf length modifier")) {
	return 1;
    } else if (strstr (line, "forbids long long integer constants")) {
	return 1;
    } else if (strstr (line, "long long integer constant")) {
	return 1;
    } else if (strstr (line, "unused parameter")) {
	return 1;
    } else if (strstr (line, "defined but not used")) {
	return 1;
    } else if (strstr (line, "anonymous variadic macros were introduced in C99")) {
    return 1;
    }
    return 0;
}

int
error_line (
    char *line
    )
{
    if (strstr (line, "control reaches end of non-void function")) {
	return 1;
    } else if (strstr (line, "suggest parentheses around assignment used as truth value")) {
	return 1;
    }
    return 0;
}

void
filter (
    int li
    )
{
    static int origin_line = -1;
    static int end_origin = -1;
    int i;

    if (strstr (line_base[li], "In file included from")) {
	origin_line = li;
	end_origin = li;
    } else if (strstr (line_base[li], "                 from")) {
	if (origin_line >= 0) {
	    end_origin = li;
	}
    } else if (strstr (line_base[li], "In function")) {
	origin_line = li;
	end_origin = li;
    } else if (strstr (line_base[li], "At top level")) {
	origin_line = li;
	end_origin = li;
    } else {
	if (origin_line >= 0) {
	    if (!skip_line (line_base[li])) {
		for (i = origin_line; i < (end_origin+1); i++) {
		    write (fileno(stdout), line_base[i], strlen(line_base[i]));
		    write (fileno(gcc_log), line_base[i], strlen(line_base[i]));
		}
		origin_line = -1;
		end_origin = -1;
		write (fileno(stdout), line_base[li], strlen(line_base[li]));
		write (fileno(gcc_log), line_base[li], strlen(line_base[li]));
	    } else {
		write (fileno(filter_log), line_base[li], strlen(line_base[li]));
	    }
	} else {
	    if (!skip_line (line_base[li])) {
		write (fileno(stdout), line_base[li], strlen(line_base[li]));
		write (fileno(gcc_log), line_base[li], strlen(line_base[li]));
	    } else {
		write (fileno(filter_log), line_base[li], strlen(line_base[li]));
	    }
	}
    }
}

int main (
    int argc,
    char *argv[]
    )
{
    int line = 0;
    int i;
    char curpath[PATH_MAX+1];
    char *splice_home;
    char *splice_target;
    char logfile[1024];
    char filterfile[1024];
    char gcc_command[8192];
    FILE *infile;
    char target[512];
    int filter_error = 0;
    int next_is_target = 0;
    char info[1024];

    getcwd (curpath, sizeof(curpath));
    splice_home = getenv ("OSPL_HOME");
    splice_target = getenv ("SPLICE_TARGET");
    snprintf (logfile, 1024, "%s/%s_gcc.log", splice_home, splice_target);
    snprintf (filterfile, 1024, "%s/%s_filter.log", splice_home, splice_target);

    if (argc < 2) {
	printf ("Usage: %s gcc gcc-options\n", argv[0]);
        return 1;
    }
    gcc_command[0] = '\0';
    for (i = 1; i < argc; i++) {
        if ( r_index( argv[i], ' ') != NULL) {
	  /* Option contains one or more spaces, so quote it */
	  strcat (gcc_command, "\"");
	  strcat (gcc_command, argv[i]);
	  strcat (gcc_command, "\"");
	} else {
	  strcat (gcc_command, argv[i]);
	}
	strcat (gcc_command, " ");
	if (next_is_target) {
	    strcpy (target, argv[i]);
	    next_is_target = 0;
	} else {
	    if (strcmp (argv[i], "-o") == 0) {
	        next_is_target = 1;
	    } else {
		int len = strlen (argv[i]);
		if ((r_index (argv[i], '.') == &argv[i][len-2]) &&
		    (argv[i][len-1] == 'c')) {
		    if (r_index (argv[i], '/')) {
			strcpy (target, r_index (argv[i], '/') + 1);
			target [strlen(target) - 1] = 'o';
		    } else {
			strcpy (target, argv[i]);
		    }
	        }
	    }
	}
    }
    strcat (gcc_command, "2>&1");
    infile = popen (gcc_command, "r");
    if (infile) {
        while (fgets (line_base[line], 512, infile)) {
	    line++;
        }
        gcc_log = fopen (logfile, "a+");
        filter_log = fopen (filterfile, "a+");
        snprintf (info, sizeof(info), "############ %s ############\n", curpath);
	write (fileno(gcc_log), info, strlen(info));
	write (fileno(filter_log), info, strlen(info));
        for (i = 0; i < line; i++) {
	    filter (i);
	    if (error_line (line_base[i])) {
	        snprintf (info, sizeof(info),
		    "\n*** SPLICE-DDS fatal error (correct error first) :\n\t%s\n", line_base[i]);
		write (fileno(gcc_log), info, strlen(info));
	        filter_error = 1;
		unlink (target);
	    }
        }
        if (pclose (infile)) {
	    filter_error = 1;
	}
    }
    fclose (gcc_log);
    fclose (filter_log);

    if (filter_error) {
        return 127;
    }
    return 0;
}
