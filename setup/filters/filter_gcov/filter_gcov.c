#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef char *line_segments[20];

typedef struct file_stat {
    char *name;
    float sle;
    int sl;
    float be;
    float bt;
    int b;
    float ce;
    int c;
    int mf;
    struct file_stat *next;
} file_stat;

typedef struct component_s {
    char *name;
    int files_fail_on_limit;
    int files_fail_untouched_functions;
    int files_pass_on_limit;
    file_stat *stat;
    struct component_s *next;
} component_stat;

static char *component = NULL;
static component_stat *comp_stat = NULL;
static int tot_filecount = 0;
static float tot_sle = 0.0;
static int tot_sl = 0;
static float tot_be = 0.0;
static int tot_b = 0;
static float tot_ce = 0.0;
static int tot_c = 0;
static int tot_files_fail_on_limit = 0;
static int tot_files_pass_on_limit = 0;
static int tot_files_fail_untouched_functions = 0;
static int missed_functions = 0;
static int distribution_count[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static char *
f_strdup(
    const char *src)
{
	char *dest = NULL;
	
	if (src) {
		dest = malloc(strlen(src) + 1);
		if (dest) {
			strcpy(dest, src);
		}
	}
	return dest;
}

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

void split_line (
    char *line,
    line_segments *segm
    )
{
    int i = 0;
    int j = 0;

    while (line[i] != '\0') {
	while (line[i] == ' ') {
	    i++;
	}
	(*segm)[j] = &line[i];
	j++;
        while ((line[i] != '\0') && (line[i] != ' ') && (line[i] != '\n')) {
	    i++;
	}
	if ((line[i] == ' ') || (line[i] == '\n')) {
	    line[i] = '\0';
	    i++;
	}
    }
}

void create_statistics (
    char *component
    )
{
    component_stat *cs = malloc (sizeof (component_stat));

    memset (cs, 0, sizeof (component_stat));
    cs->name = component;
    if (comp_stat == NULL) {
	comp_stat = cs;
    } else {
	cs->next = comp_stat;
	comp_stat = cs;
    }
}

char *clean_filename (
    char *fn
    )
{
    char *end = fn + strlen(fn);

    if (r_index (fn, '/')) {
	fn = r_index (fn, '/') + 1;
    }
    return fn;
}

component_stat *
find_component (
    char *name
    )
{
    component_stat *cs;

    cs = comp_stat;
    while (cs && (strcmp(name, cs->name) != 0)) {
	cs = cs->next;
    }
    return cs;
}

file_stat *
find_filestat (
    component_stat *cs,
    char *name
    )
{
    file_stat *fs;

    fs = cs->stat;
    while (fs && (strcmp(name, fs->name) != 0)) {
	fs = fs->next;
    }
    return fs;
}

void register_missed_functions (
    char *file,
    int missed_functions
    )
{
    component_stat *cs;
    file_stat *fs;
    char *dot;

    dot = r_index (file, '.');
    if (dot) {
	*dot = '\0';
        dot = r_index (file, '.');
        if (dot) {
	    *dot = '\0';
	}
    }
    cs = find_component (component);
    if (cs) {
	fs = find_filestat (cs, file);
	if (fs == NULL) {
	    fs = malloc (sizeof (file_stat));
	    memset (fs, 0, sizeof (file_stat));
	    fs->name = f_strdup(file);
	    if (cs->stat == NULL) {
		cs->stat = fs;
	    } else {
		fs->next = cs->stat;
		cs->stat = fs;
	    }
	}
    }
    fs->mf = missed_functions;
}

void create_le_statistics (
    char *le,
    char *l,
    char *file
    )
{
    file_stat *fs;
    char *fn;
    component_stat *cs;

    fn = clean_filename (file);
    cs = find_component (component);
    if (cs) {
	fs = find_filestat (cs, fn);
	if (fs == NULL) {
	    fs = malloc (sizeof (file_stat));
	    memset (fs, 0, sizeof (file_stat));
	    fs->name = f_strdup(fn);
	    if (cs->stat == NULL) {
		cs->stat = fs;
	    } else {
		fs->next = cs->stat;
		cs->stat = fs;
	    }
	}
    }
    sscanf (le, "%f", &fs->sle);
    sscanf (l, "%d", &fs->sl);
}

void create_be_statistics (
    char *be,
    char *b,
    char *file
    )
{
    file_stat *fs;
    char *fn;
    component_stat *cs;

    fn = clean_filename (file);
    cs = find_component (component);
    if (cs) {
	fs = find_filestat (cs, fn);
	if (fs == NULL) {
	    fs = malloc (sizeof (file_stat));
	    memset (fs, 0, sizeof (file_stat));
	    fs->name = f_strdup(fn);
	    if (cs->stat == NULL) {
		cs->stat = fs;
	    } else {
		fs->next = cs->stat;
		cs->stat = fs;
	    }
	}
    }
    sscanf (be, "%f", &fs->be);
    sscanf (b, "%d", &fs->b);
#if 0
    if ((fs->be < 80.0) && !(fs->be == 0.0 && fs->sle == 100.0)) {
	cs->files_fail_on_limit++;
    } else {
	cs->files_pass_on_limit++;
	if (missed_functions) {
	    cs->files_fail_untouched_functions++;
	}
    }
    fs->mf = missed_functions;
#endif
}

void create_ce_statistics (
    char *ce,
    char *c,
    char *file
    )
{
    file_stat *fs;
    char *fn;
    component_stat *cs;

    fn = clean_filename (file);
    cs = find_component (component);
    if (cs) {
	fs = find_filestat (cs, fn);
	if (fs == NULL) {
	    fs = malloc (sizeof (file_stat));
	    memset (fs, 0, sizeof (file_stat));
	    fs->name = f_strdup(fn);
	    if (cs->stat == NULL) {
		cs->stat = fs;
	    } else {
		fs->next = cs->stat;
		cs->stat = fs;
	    }
	}
    }
    sscanf (ce, "%f", &fs->ce);
    sscanf (c, "%d", &fs->c);
}

void extract_set_component (
    char *line
    )
{
    char *ce;
    char *cs = &line[4];

    ce = strstr (line, "/bld/");
    if (ce) {
	*ce = '\0';
    }
    if ((component == NULL) || (strcmp (component, cs) != 0)) {
	component = f_strdup (cs);
	create_statistics (component);
    }
}

void filter_line (
    char *line
    )
{
    static int ignore_file = 0;

    if (strncmp (line, "==> ", 4) == 0) {
	extract_set_component (line);
	// missed_functions = 0;
    } else if (strncmp (line, "Creating ", 9) == 0) {
        line_segments segm;
	if (ignore_file == 0) {
	    split_line (line, &segm);
	    register_missed_functions (segm[1], missed_functions);
	} else {
	    ignore_file = 0;
	}
	missed_functions = 0;
    } else if (strstr (line, "/opt/nds/ACE_wrappers")) {
	/* skip this file */
	ignore_file = 1;
    } else if (strstr (line, "/usr/include/c++")) {
	/* skip this file */
	ignore_file = 1;
    } else if (strstr (line, "/opt/s2dev")) {
	/* skip this file */
	ignore_file = 1;
    } else if (strstr (line, "testsuite")) {
	/* skip this file */
	ignore_file = 1;
    } else if (strstr (line, "source lines executed in file")) {
        line_segments segm;
	split_line (line, &segm);
	create_le_statistics (segm[0], segm[2], segm[8]);
    } else if (strstr (line, "lines executed in file")) {
        line_segments segm;
	split_line (line, &segm);
	create_le_statistics (segm[0], segm[2], segm[7]);
    } else if (strstr (line, "branches executed in file")) {
        line_segments segm;
	split_line (line, &segm);
	create_be_statistics (segm[0], segm[2], segm[7]);
    } else if (strstr (line, "branches taken at least once in file")) {
    } else if (strstr (line, "calls executed in file")) {
        line_segments segm;
	split_line (line, &segm);
	create_ce_statistics (segm[0], segm[2], segm[7]);
    } else if (strstr (line, "lines executed in function")) {
        line_segments segm;
	split_line (line, &segm);
	if (strcmp (segm[0], "0.00%") == 0) {
	    missed_functions++;
	}
    }
}

void filter (
    char *inf
    )
{
    FILE *ifile;
    char line [1024];
    char *lp;

    ifile = fopen (inf, "r");
    if (ifile != NULL) {
	while ((lp = fgets (line, sizeof (line), ifile)) != NULL) {
	    filter_line (lp);
	}
    } else {
	printf ("Cannot open input-file %s\n", inf);
    }
}

void show_filestat (
    component_stat *cs,
    double *bc,
    double *lc,
    double *cc,
    int *fc
    )
{
    int filecount = 0;
    float totsle = 0.0;
    int totsl = 0;
    float totbe = 0.0;
    int totb = 0;
    float totce = 0.0;
    int totc = 0;
    int pass;
    file_stat *fs;

    fs = cs->stat;
    while (fs) {

        if ((fs->be < 80.0) && !(fs->be == 0.0 && fs->sle == 100.0)) {
	    pass = 0;
	    cs->files_fail_on_limit++;
        } else {
	    pass = 1;
	    cs->files_pass_on_limit++;
	    if (fs->mf) {
	        cs->files_fail_untouched_functions++;
	    }
        }

	filecount++;
	tot_filecount++;
	(*fc)++;
	totsle += fs->sle;
	tot_sle += fs->sle;
	*lc += (double)fs->sle;
	totsl += fs->sl;
	tot_sl += fs->sl;
	totbe += fs->be;
	tot_be += fs->be;
	*bc += (double)fs->be;
	totb += fs->b;
	tot_b += fs->b;
	totce += fs->ce;
	tot_ce += fs->ce;
	*cc += (double)fs->ce;
	tot_c += fs->c;
	totc += fs->c;
	printf ("%40s   %6.2f/%5d  %6.2f/%5d  %6.2f/%5d  %3d %c\n",
	    fs->name,
	    fs->be,
	    fs->b,
	    fs->sle,
	    fs->sl,
	    fs->ce,
	    fs->c,
	    fs->mf,
	    pass ? fs->mf ? '-' : ' ' : '*');
        if (fs->be == 0.0 && fs->sle == 100.0) {
	    distribution_count[10]++;
        } else {
	    distribution_count[((int)(fs->be))/10]++;
        }
	fs = fs->next;
    }
    printf ("Totals: %d files, %.2f%c branch coverage on %d branches\n",
	filecount,
	totbe/(float)filecount,
	'%',
	totb);
    printf ("        %d files, %.2f%c line coverage on %d lines\n",
	filecount,
	totsle/(float)filecount,
	'%',
	totsl);
    printf ("        %d files, %.2f%c call coverage on %d calls\n",
	filecount,
	totce/(float)filecount,
	'%',
	totc);
}

void show_statistics (
    void
    )
{
    component_stat *cs;
    double bc = 0.0;
    double lc = 0.0;
    double cc = 0.0;
    int fc = 0;

    cs = comp_stat;
    while (cs) {
	printf ("%-40s    branch exec    lines exec    calls exec   mf\n", cs->name);
	printf ("========================================================================================\n");
	show_filestat (cs, &bc, &lc, &cc, &fc);
        printf ("        %d files fail on limit of 80%% branch coverage\n", cs->files_fail_on_limit);
        printf ("        %d files pass on limit of 80%% branch coverage\n", cs->files_pass_on_limit);
        printf ("        %d files fail extra if untouched functions are regarded\n", cs->files_fail_untouched_functions);
        tot_files_fail_on_limit += cs->files_fail_on_limit;
        tot_files_pass_on_limit += cs->files_pass_on_limit;
        tot_files_fail_untouched_functions += cs->files_fail_untouched_functions;
	printf ("\n");
        cs = cs->next;
    }
    printf ("Final totals: Brach coverage %.2f%c, Line coverage %.2f%c, Call coverage %.2f%c\n",
	bc/(double)fc, '%', lc/(double)fc, '%', cc/(double)fc, '%');
    printf ("              %d files, %.2f%c branch coverage on %d branches\n",
	tot_filecount,
	tot_be/(float)tot_filecount,
	'%',
	tot_b);
    printf ("              %d files, %.2f%c line coverage on %d lines\n",
	tot_filecount,
	tot_sle/(float)tot_filecount,
	'%',
	tot_sl);
    printf ("              %d files, %.2f%c call coverage on %d calls\n",
	tot_filecount,
	tot_ce/(float)tot_filecount,
	'%',
	tot_c);
    printf ("              %d files fail on limit of 80%c branch coverage\n", tot_files_fail_on_limit, '%');
    printf ("              %d files pass on limit of 80%c branch coverage\n", tot_files_pass_on_limit, '%');
    printf ("              %d files fail extra if untouched functions are regarded\n", tot_files_fail_untouched_functions);

    printf ("\nDistribution of branch coverage\n");
    printf ("\tPercentage cov  #files  %%files\n");
    printf ("\t 0.0 -   9.9   %4d     (%5.2f%%)\n", distribution_count[0], (float)distribution_count[0]*100.0/(float)tot_filecount);
    printf ("\t10.0 -  19.9   %4d     (%5.2f%%)\n", distribution_count[1], (float)distribution_count[1]*100.0/(float)tot_filecount);
    printf ("\t20.0 -  29.9   %4d     (%5.2f%%)\n", distribution_count[2], (float)distribution_count[2]*100.0/(float)tot_filecount);
    printf ("\t30.0 -  39.9   %4d     (%5.2f%%)\n", distribution_count[3], (float)distribution_count[3]*100.0/(float)tot_filecount);
    printf ("\t40.0 -  49.9   %4d     (%5.2f%%)\n", distribution_count[4], (float)distribution_count[4]*100.0/(float)tot_filecount);
    printf ("\t50.0 -  59.9   %4d     (%5.2f%%)\n", distribution_count[5], (float)distribution_count[5]*100.0/(float)tot_filecount);
    printf ("\t60.0 -  69.9   %4d     (%5.2f%%)\n", distribution_count[6], (float)distribution_count[6]*100.0/(float)tot_filecount);
    printf ("\t70.0 -  79.9   %4d     (%5.2f%%)\n", distribution_count[7], (float)distribution_count[7]*100.0/(float)tot_filecount);
    printf ("\t80.0 -  89.9   %4d     (%5.2f%%)\n", distribution_count[8], (float)distribution_count[8]*100.0/(float)tot_filecount);
    printf ("\t90.0 - 100.0   %4d     (%5.2f%%)\n", distribution_count[9] + distribution_count[10],
	((float)distribution_count[9] + (float)distribution_count[10])*100.0/(float)tot_filecount);
}

int main (
    int argc,
    char *argv[]
    )
{
    char *inf;

    if (argc == 1) {
	inf = "gcov.log";
    } else if (argc == 2) {
	inf = argv[1];
    } else {
	printf ("Usage: %s [<input-file>]\n", argv[0]);
	exit (1);
    }
    filter (inf);
    show_statistics ();
}
