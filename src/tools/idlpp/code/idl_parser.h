/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
