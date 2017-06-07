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
#ifndef OS_SIGNATURE_H
#define OS_SIGNATURE_H

#define OS_MUTEX_MAGIC_SIG  0xF0A7B4C9E6A98510ULL
#define OS_COND_MAGIC_SIG   0xF0A7B4C9E6A98511ULL
#ifdef OSPL_STRICT_MEM
#define OS_MALLOC_MAGIC_SIG 0xF0A7B4C9E6A98512ULL
#define OS_FREE_MAGIC_SIG   0xC1BF0D82E5B89623ULL

/* Check for either of the above values */
#ifdef PA_BIG_ENDIAN
#define OS_MAGIC_SIG_CHECK(ptr) *(ptr) != 0xF0 \
		  || *((ptr)+1) != 0xA7 \
		  || *((ptr)+2) != 0xB4 \
		  || *((ptr)+3) != 0xC9 \
		  || *((ptr)+4) != 0xE6 \
		  || *((ptr)+5) != 0xA9 \
		  || *((ptr)+6) != 0x85 \
		  || (*((ptr)+7) & 0xFCU) != 0x10 
#else
#define OS_MAGIC_SIG_CHECK(ptr) *((ptr)+7) != 0xF0  \
		  || *((ptr)+6) != 0xA7 \
		  || *((ptr)+5) != 0xB4 \
		  || *((ptr)+4) != 0xC9 \
		  || *((ptr)+3) != 0xE6 \
		  || *((ptr)+2) != 0xA9 \
		  || *((ptr)+1) != 0x85 \
          || (*(ptr) & 0xFCU) != 0x10 
#endif
#endif

#endif
