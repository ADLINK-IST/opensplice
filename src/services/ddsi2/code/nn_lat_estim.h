#ifndef NN_LAT_ESTIM_H
#define NN_LAT_ESTIM_H

#include "os_defs.h"

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
int nn_lat_estim_log (logbuf_t lb, logcat_t logcat, const char *tag, const struct nn_lat_estim *le);

#endif /* NN_LAT_ESTIM_H */
