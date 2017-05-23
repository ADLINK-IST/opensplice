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
#ifndef NN_LAT_ESTIM_H
#define NN_LAT_ESTIM_H

#include "os_defs.h"

#include "q_log.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define NN_LAT_ESTIM_MEDIAN_WINSZ 7

struct nn_lat_estim {
  /* median filtering with a small window in an attempt to remove the
     worst outliers */
  int index;
  float window[NN_LAT_ESTIM_MEDIAN_WINSZ];
  /* simple alpha filtering for smoothing */
  float smoothed;
};

void nn_lat_estim_init (struct nn_lat_estim *le);
void nn_lat_estim_fini (struct nn_lat_estim *le);
void nn_lat_estim_update (struct nn_lat_estim *le, os_int64 est);
double nn_lat_estim_current (const struct nn_lat_estim *le);
int nn_lat_estim_log (logcat_t logcat, const char *tag, const struct nn_lat_estim *le);

#if defined (__cplusplus)
}
#endif

#endif /* NN_LAT_ESTIM_H */

/* SHA1 not available (unoffical build.) */
