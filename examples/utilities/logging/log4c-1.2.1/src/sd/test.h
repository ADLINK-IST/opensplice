/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 *
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __sd_test_h
#define __sd_test_h

/**
 * @file test.h
 */

#include <stdio.h>
#include <sd/defs.h>

__SD_BEGIN_DECLS

#define SD_TEST_MAX_NFUNC  100

struct __sd_test;

typedef struct __sd_test sd_test_t;

typedef int (sd_test_func_t)(sd_test_t* a_test, int argc, char* argv[]);
	
extern sd_test_t*	sd_test_new(int a_argc, char* a_argv[]);
extern void sd_test_delete(sd_test_t* a_this);
	
extern const char* sd_test_get_name(const sd_test_t* a_this);
extern int sd_test_get_verbose(const sd_test_t* a_test);
extern int sd_test_set_verbose(sd_test_t* a_this, int a_verbose);

extern FILE* sd_test_in(sd_test_t* a_this);
extern FILE* sd_test_out(sd_test_t* a_this);
extern FILE* sd_test_err(sd_test_t* a_this);

extern int sd_test_run(sd_test_t* a_this, int argc, char* argv[]);
extern int sd_test_add(sd_test_t* a_this, sd_test_func_t a_func);

__SD_END_DECLS

#endif
