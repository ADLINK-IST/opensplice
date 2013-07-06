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
