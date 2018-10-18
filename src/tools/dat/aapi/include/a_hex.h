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
 * This file contains the operation(s) for printing a hex dump
 * of a specific part of memory, for Splice purposes typically
 * as of 0xA0000000, but any accessible memory address may be
 * specified.
 */


#include <stdio.h>

#ifndef A_HEX_H
#define A_HEX_H



/**
 * \brief
 * Prints a hex dump
 *
 * This operation will print a memory dump in hexdecimal format,
 * specified by memory start address and size.
 *
 * \param output
 * File pointer to print to, typically stdout
 *
 * \param addrPtr
 * Pointer to the memory address to start the dump from
 *
 * \param length
 * Length of the memory segment to dump
 *
 * \param lineWidth
 * Number of bytes to print per line
 *
 * \param groupWidth
 * Number of bytes to group, after which an extra space space is
 * printed, for visual purposes.
 */
void a_hexDump(
	FILE *output, void *addrPtr, int length, int lineWidth, int groupWidth);


#endif

//END a_hex.h
