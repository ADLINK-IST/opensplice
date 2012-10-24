/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef OS_SIGNATURE_H
#define OS_SIGNATURE_H

#define OS_MUTEX_MAGIC_SIG  0xF0A7B4C9E6A98510ULL
#define OS_COND_MAGIC_SIG   0xF0A7B4C9E6A98511ULL
#ifdef OSPL_STRICT_MEM
#define OS_MALLOC_MAGIC_SIG 0xF0A7B4C9E6A98512ULL
#define OS_FREE_MAGIC_SIG   0xC1BF0D82E5B89623ULL

/* Check for either of the above values */
#define OS_MAGIC_SIG_CHECK(ptr) *(ptr) != 0xF0 \
		  || *((ptr)+1) != 0xA7 \
		  || *((ptr)+2) != 0xB4 \
		  || *((ptr)+3) != 0xC9 \
		  || *((ptr)+4) != 0xE6 \
		  || *((ptr)+5) != 0xA9 \
		  || *((ptr)+6) != 0x85 \
		  || (*((ptr)+7) & 0xFCU) != 0x10 
#endif

#endif
