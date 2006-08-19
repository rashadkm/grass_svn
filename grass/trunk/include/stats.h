
#ifndef GRASS_STATS_H
#define GRASS_STATS_H

#include <grass/gis.h>

typedef void stat_func(DCELL *, DCELL *, int);

extern stat_func c_ave;
extern stat_func c_count;
extern stat_func c_divr;
extern stat_func c_intr;
extern stat_func c_max;
extern stat_func c_maxx;
extern stat_func c_median;
extern stat_func c_min;
extern stat_func c_minx;
extern stat_func c_mode;
extern stat_func c_stddev;
extern stat_func c_sum;
extern stat_func c_var;
extern stat_func c_reg_m;
extern stat_func c_reg_c;
extern stat_func c_quart1;
extern stat_func c_quart3;
extern stat_func c_perc90;

extern int sort_cell(DCELL *, int);

#endif

