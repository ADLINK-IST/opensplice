static const char version[] = "$Id$";

/* 
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifdef HAVE_CONFIG_H
#       include "config.h"
#endif
#include <sd/test.h>
#include <sd/malloc.h>
#include <sd/sprintf.h>
#include <sd/sd_xplatform.h>
 
#include <stdlib.h>
#include <string.h>

typedef XP_UINT64 usec_t;

#define MAX_NFUNC 100

struct __sd_test
{
    const char*         name;
    char                in_filename[128];
    char                ref_filename[128];
    char                out_filename[128];
    FILE*               in;
    FILE*               out;
    FILE*               err;
    int                 verbose;
    int                 timed;
    sd_test_func_t**    funcs;
    int                 size;
    int			argc;
    char**		argv;
};


/******************************************************************************/
static usec_t now(void)
{
 
#ifdef _WIN32
 FILETIME tv;
 ULARGE_INTEGER   li;
#else
    struct timeval tv;
#endif

    SD_GETTIMEOFDAY(&tv, NULL);

#ifdef _WIN32
	memcpy(&li, &tv, sizeof(FILETIME));
	li.QuadPart /= 10;                /* In microseconds */
	/* printf("timestampstamp usec %I64u\n", li.QuadPart);*/
	return li.QuadPart;
#else
    return (usec_t) (tv.tv_sec * 1000000 + tv.tv_usec);
#endif
}

/******************************************************************************/
static int test_compare(sd_test_t* this, int a_argc, char* a_argv[])
{
    char cmd[1024];

    if (SD_ACCESS_READ(this->ref_filename) ||
	SD_ACCESS_READ(this->out_filename))
	return 1;
    
    snprintf(cmd, sizeof(cmd), "%s %s %s 1>/dev/null 2>&1", DIFF_CMD,
             this->ref_filename, this->out_filename);
   
    return ! system(cmd);
}

/******************************************************************************/
extern sd_test_t* sd_test_new(int a_argc, char* a_argv[])
{
    sd_test_t* this;
    int c;
    char* ptr;

    this        = sd_calloc(1, sizeof(sd_test_t));
    this->funcs = sd_calloc(MAX_NFUNC, sizeof(sd_test_func_t *));

    /*
     * get rid of libtool frontend script
     */
    ptr = strstr(a_argv[0], "lt-");
    if (ptr)
	this->name = ptr + 3;
    else
      this->name = a_argv[0];    

    snprintf(this->ref_filename, sizeof(this->ref_filename), "%s.ref",
	     this->name);
    snprintf(this->in_filename,  sizeof(this->in_filename),  "%s.in",
	     this->name);
    snprintf(this->out_filename, sizeof(this->out_filename), "%s.out",
	     this->name);
    
    this->in    = fopen(this->in_filename,  "r");
    this->out   = fopen(this->out_filename, "w");
    this->err   = 0;
    this->verbose= 0;
    this->size  = 0;

    while ((c = SD_GETOPT(a_argc, a_argv, "vt")) != EOF) {
        switch(c) {
        case 'v': this->verbose = 1; break;
        case 't': this->timed   = 1; break;
        default:                     break; 
        }
    }

    this->argc = a_argc - (SD_OPTIND - 1);
    this->argv = a_argv + (SD_OPTIND - 1);
    return this;
}

/******************************************************************************/
extern void sd_test_delete(sd_test_t* this)
{
    if (!this)
        return;

    if (this->in) fclose(this->in);
    if (this->out) fclose(this->out);
    free(this->funcs);
    free(this);
}
        
/******************************************************************************/
extern const char* sd_test_get_name(const sd_test_t* this)
{
    return this ? this->name : NULL;
}

/******************************************************************************/
extern int sd_test_get_verbose(const sd_test_t* this)
{
    return this ? this->verbose: 0;
}

/******************************************************************************/
extern int sd_test_set_verbose(sd_test_t* this, int a_verbose)
{
    if (!this)
        return 0;

    return this->verbose = a_verbose;
}

/******************************************************************************/
extern FILE* sd_test_in(sd_test_t* this)
{
    if (!this)
        return NULL;

    return this->in ? this->in : stdin;
}

/******************************************************************************/
extern FILE* sd_test_out(sd_test_t* this)
{
    if (!this)
        return NULL;

    if (this->verbose)
        return stdout;

    return this->out ? this->out : stdout;
}

/******************************************************************************/
extern FILE* sd_test_err(sd_test_t* this)
{
    if (!this)
        return NULL;

    return this->err ? this->err : stderr;
}

/******************************************************************************/
extern int sd_test_run(sd_test_t* this, int argc, char* argv[])
{
    int i, passed = 0;

    if (!this)
        return -1;

    sd_test_add(this, test_compare);
    
    fprintf(sd_test_err(this), "%s: ", this->name);
    
    for (i = 0; i < this->size; i++) {
        int t;
        usec_t elapsed;

        fprintf(sd_test_out(this), "=> test #%d :\n", i);

        elapsed = now();
        t = (*this->funcs[i])(this, this->argc, this->argv);
        elapsed = now() - elapsed;

        fprintf(sd_test_out(this), "=> test #%d : %s\n", i, t ? " passed" : " failed");
        fflush(sd_test_out(this));

        passed += (t != 0);
        
        fprintf(sd_test_err(this), "%c", t ? '+' : '-');
        if (this->timed)
            fprintf(sd_test_err(this), "%lld ", elapsed);
    }
    
    fprintf(sd_test_err(this), " %d/%d %s.\n", passed, this->size,
            passed == this->size ? " passed" : " failed");
    
    return passed == this->size;
}

/******************************************************************************/
extern int sd_test_add(sd_test_t* this, sd_test_func_t a_func)
{
    if (!this)
        return -1;

    if (this->size == MAX_NFUNC)
        return this->size;

    this->funcs[this->size] = a_func;

    return this->size++;
}
