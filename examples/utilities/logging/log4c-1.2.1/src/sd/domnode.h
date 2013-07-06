/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __sd_domnode_h
#define __sd_domnode_h

/**
 * @file domnode.h @ingroup sd
 *
 * @brief Generic DOM object.
 */

#include <stdio.h>
#include <sd/list.h>

__SD_BEGIN_DECLS

typedef struct {
    const char*	name;
    const char*	value;
    sd_list_t*	children;
    sd_list_t*	attrs;
} sd_domnode_t;

extern sd_domnode_t*	sd_domnode_new(const char* a_name,
				       const char* a_value);

extern void		sd_domnode_delete(sd_domnode_t* this);

extern int		sd_domnode_read(sd_domnode_t* this,
					const char* a_buffer, size_t asize);
extern int		sd_domnode_write(sd_domnode_t* this, char** a_buffer,
					 size_t* asize);

extern int		sd_domnode_fread(sd_domnode_t* this, FILE* a_stream);
extern int		sd_domnode_fwrite(const sd_domnode_t* this,
					  FILE* a_stream);

extern int		sd_domnode_load(sd_domnode_t* this,
					const char* a_filename);

extern int		sd_domnode_store(const sd_domnode_t* this, 
					 const char* a_filename);

extern sd_domnode_t*	sd_domnode_search(const sd_domnode_t* this,
					  const char* a_name);

extern sd_domnode_t* 	sd_domnode_attrs_put(sd_domnode_t* this,
					     sd_domnode_t* a_attr);
extern sd_domnode_t*	sd_domnode_attrs_get(const sd_domnode_t* this,
					     const char* a_name);
extern sd_domnode_t*	sd_domnode_attrs_remove(sd_domnode_t* this,
						const char* a_name);

/** Creates a new node. */
extern sd_domnode_t* __sd_domnode_new(const char* name, const char* a_value,
				      int is_elem);

__SD_END_DECLS

#endif

