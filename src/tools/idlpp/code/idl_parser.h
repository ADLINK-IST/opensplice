/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef IDL_PARSER_H
#define IDL_PARSER_H

#include "c_typebase.h"
#include "c_base.h"
#include "c_module.h"

/* The yacc/bison parser does not export interfaces yet, therefor define here */
extern void idl_idlinit(c_module schema);
extern int idl_idlparse(const char *fname);

c_base 
idl_parseFile (
    const char *filename,
    c_bool traceInput);

#endif /* IDL_PARSER_H */
