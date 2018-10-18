/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#ifndef CODE_IDL_GENMATLABHELPER_H_
#define CODE_IDL_GENMATLABHELPER_H_

#include "idl_scope.h"
#include "idl_tmplExp.h"
#include "idl_program.h"

c_char * idl_matlabId(const char *identifier);
c_char *idl_scopeStackMatlab (idl_scope scope, const char *scopeSepp, const char *name);
void idl_openMatlabPackage (idl_scope scope, const char *name);
void idl_closeMatlabPackage (void);
c_char *idl_matlabTypeName(idl_typeSpec typeSpec);
void idl_streamOutTypeInfoEntries(idl_streamOut so, const char *name, idl_typeSpec typeSpec);
int idl_matlabIsSequence(idl_typeSpec typeSpec);
int idl_matlabIsArray(idl_typeSpec typeSpec);
int idl_matlabIsArrayOfSequences(idl_typeSpec typeSpec);
int idl_matlabIsString(idl_typeSpec typeSpec);
int idl_matlabIsStruct(idl_typeSpec typeSpec);
int idl_matlabIsEnum(idl_typeSpec typeSpec);
int idl_matlabIsNotMultiValued(idl_typeSpec typeSpec);
idl_typeSpec idl_matlabFindArrayWrappedType(idl_typeSpec typeSpec);
idl_typeSpec idl_matlabFindSequenceWrappedType(idl_typeSpec typeSpec);
c_ulong idl_matlabStringBound(idl_typeSpec typeSpec);
c_ulong idl_matlabSequenceBound(idl_typeSpec typeSpec);
int idl_scopeIsInStruct(idl_scope scope);
int idl_matlabShouldBeCellArray(idl_typeSpec typeSpec);
c_char *idl_matlabFormattedDimensions(idl_typeSpec typeSpec);
c_char *idl_matlabFieldInitialization(idl_typeSpec typeSpec);
#endif /* CODE_IDL_GENMATLABHELPER_H_ */
