/* $Id$
 *
 * Copyright 2001-2003, Meiosys (www.meiosys.com). All rights reserved.
 * See the COPYING file for the terms of usage and distribution.
 */

#ifndef __sd_domnode_xml_h
#define __sd_domnode_xml_h

/**
 * @file domnode-xml.h @ingroup sd
 *
 * @brief Private API for XML parsing.
 */

#include "domnode.h"
#include "stack.h"

struct __sd_domnode_xml_maker {
    void*		scanner;
    sd_stack_t*		elements;
    sd_domnode_t*	root;
};

extern int __sd_domnode_xml_fread(sd_domnode_t** a_node, FILE* a_stream);
extern int __sd_domnode_xml_fwrite(const sd_domnode_t* a_node, FILE* a_stream);

extern int __sd_domnode_xml_read(sd_domnode_t** a_node, const char* a_buffer,
				 size_t a_size);
extern int __sd_domnode_xml_write(const sd_domnode_t* a_node, char** a_buffer,
				  size_t* a_size);

#endif
