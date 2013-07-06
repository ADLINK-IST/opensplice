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
#ifndef unctrl
#define unctrl(c) _unctrl[0xff&(int)(c)]
#define Unctrl(c) _Unctrl[0xff&(int)(c)]
extern char *_unctrl[];
extern char *_Unctrl[];
#endif
