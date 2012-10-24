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
#ifndef NN_MLV_H
#define NN_MLV_H

#include "os_defs.h"

struct mlv_stats {
  int current;
  int nblocks;
  int nzeroblocks;
  int nwhiledisabled;
  os_uint64 seq;
};

void mlv_init (void);
void mlv_setforreal (int enable);
void mlv_fini (void);
void mlv_stats (struct mlv_stats *st);
void mlv_printlive (os_uint64 seq0, os_uint64 seq1, int (*print) (const char *fmt, ...));

#endif /* NN_MLV_H */
