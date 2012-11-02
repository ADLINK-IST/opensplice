#include <ctype.h>
#include <stddef.h>

#include "nn_log.h"
#include "nn_unused.h"
#include "nn_lat_estim.h"

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
  fest = est / 1e3; /* we do latencies microseconds */
  le->window[le->index] = fest;
  if (++le->index == NN_LAT_ESTIM_MEDIAN_WINSZ)
    le->index = 0;
  memcpy (tmp, le->window, sizeof (tmp));
  qsort (tmp, NN_LAT_ESTIM_MEDIAN_WINSZ, sizeof (tmp[0]), (int (*) (const void *, const void *)) cmpfloat);
  med = tmp[NN_LAT_ESTIM_MEDIAN_WINSZ / 2];
  if (le->smoothed == 0 && le->index == 0)
    le->smoothed = med;
  else if (le->smoothed)
    le->smoothed = (1.0f - alpha) * le->smoothed + alpha * med;
}

int nn_lat_estim_log (logbuf_t lb, logcat_t logcat, const char *tag, const struct nn_lat_estim *le)
{
  if (le->smoothed == 0.0f)
    return 0;
  else
  {
    float tmp[NN_LAT_ESTIM_MEDIAN_WINSZ];
    int i;
    memcpy (tmp, le->window, sizeof (tmp));
    qsort (tmp, NN_LAT_ESTIM_MEDIAN_WINSZ, sizeof (tmp[0]), (int (*) (const void *, const void *)) cmpfloat);
    nn_logb (lb, logcat, "LAT(%s) %e [", tag, le->smoothed);
    for (i = 0; i < NN_LAT_ESTIM_MEDIAN_WINSZ; i++)
      nn_logb (lb, logcat, "%s%e", (i > 0) ? ", " : "", tmp[i]);
    nn_logb (lb, logcat, "]");
    return 1;
  }
}

#if 0 /* not implemented yet */
double nn_lat_estim_current (const struct nn_lat_estim *le)
{
}
#endif
