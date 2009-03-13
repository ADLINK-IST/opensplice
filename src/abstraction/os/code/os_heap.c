
/****************************************************************
 * Implementation for heap memory management                    *
 ****************************************************************/

/** \file os/code/os_heap.c
 *  \brief Heap memory management - allocate and free memory from heap
 *
 * Heap memory management provides an abstraction which enables the
 * possibility to set alternative heap memory management services.
 * This may be required for debugging purposes or improve the memory
 * management if the standard is not sufficient.
 *
 * Heap memory management provides services for allocating memory
 * from heap and releasing allocated memory to heap.
 */

#include <os_heap.h>

/* include OS specific heap memory management implementation 	*/
#include <code/os_heap.c>
