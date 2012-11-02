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
#ifndef COLL_DEFS_H
#define COLL_DEFS_H

#define COLL_ERROR_START                (0x400u)
#define COLL_OK                         (COLL_ERROR_START)
#define COLL_ERROR_ALLOC                (COLL_ERROR_START + 1)
#define COLL_ERROR_NOT_EMPTY            (COLL_ERROR_START + 2)
#define COLL_ERROR_ALREADY_EXISTS       (COLL_ERROR_START + 3)
#define COLL_ERROR_PRECONDITION_NOT_MET (COLL_ERROR_START + 4)

#ifndef NULL
#define NULL ((void *) 0)
#endif

#if defined (__cplusplus)
}
#endif

#endif /* COLL_DEFS_H */
