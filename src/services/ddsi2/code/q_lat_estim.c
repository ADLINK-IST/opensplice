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
#include <ctype.h>
#include <stddef.h>

#include "q_log.h"
#include "q_unused.h"
#include "q_lat_estim.h"
#include <stdlib.h>
#include <string.h>

void nn_lat_estim_init (struct nn_lat_estim *le)
{
  int i;
  le->index = 0;
  for (i = 0; i < NN_LAT_ESTIM_MEDIAN_WINSZ; i++)
    le->window[i] = 0;
  le->smoothed = 0;
}

void nn_lat_estim_fini (UNUSED_ARG (struct nn_lat_estim *le))
{
}

static int cmpfloat (const float *a, const float *b)
{
  return (*a < *b) ? -1 : (*a > *b) ? 1 : 0;
}

void nn_lat_estim_update (struct nn_lat_estim *le, os_int64 est)
{
  const float alpha = 0.01f;
  float fest, med;
  float tmp[NN_LAT_ESTIM_MEDIAN_WINSZ];
  if (est <= 0)
    return;
  fest = (float) est / 1e3f; /* we do latencies in microseconds */
  le->window[le->index] = fest;
  if (++le->index == NN_LAT_ESTIM_MEDIAN_WINSZ)
    le->index = 0;
  memcpy (tmp, le->window, sizeof (tmp));
  qsort (tmp, NN_LAT_ESTIM_MEDIAN_WINSZ, sizeof (tmp[0]), (int (*) (const void *, const void *)) cmpfloat);
  med = tmp[NN_LAT_ESTIM_MEDIAN_WINSZ / 2];
  if (le->smoothed == 0 && le->index == 0)
    le->smoothed = med;
  else if (le->smoothed != 0)
    le->smoothed = (1.0f - alpha) * le->smoothed + alpha * med;
}

int nn_lat_estim_log (logcat_t logcat, const char *tag, const struct nn_lat_estim *le)
{
  if (le->smoothed == 0.0f)
    return 0;
  else
  {
    float tmp[NN_LAT_ESTIM_MEDIAN_WINSZ];
    int i;
    memcpy (tmp, le->window, sizeof (tmp));
    qsort (tmp, NN_LAT_ESTIM_MEDIAN_WINSZ, sizeof (tmp[0]), (int (*) (const void *, const void *)) cmpfloat);
    if (tag)
      nn_log (logcat, " LAT(%s: %e {", tag, le->smoothed);
    else
      nn_log (logcat, " LAT(%e {", le->smoothed);
    for (i = 0; i < NN_LAT_ESTIM_MEDIAN_WINSZ; i++)
      nn_log (logcat, "%s%e", (i > 0) ? "," : "", tmp[i]);
    nn_log (logcat, "})");
    return 1;
  }
}

#if 0 /* not implemented yet */
double nn_lat_estim_current (const struct nn_lat_estim *le)
{
}
#endif

/* SHA1 not available (unoffical build.) */
