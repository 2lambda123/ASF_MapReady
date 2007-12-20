#ifndef PLAN_INT_H
#define PLAN_INT_H

#include "plan.h"
#include "date_util.h"

/* kml.c */
void kml_aoi(FILE *kml_file, double clat, double clon, Polygon *aoi);
void write_pass_to_kml(FILE *kml_file, double lat, double lon, PassInfo *pi);

/* tle.c */
void read_tle(const char *tle_filename, const char *satellite,
              stateVector *st, double *st_time);

/* overlap.c */
OverlapInfo *overlap_new(int pct, int n, Polygon *viewable_region,
                         double clat, double clon, stateVector *st,
                         double t);
void overlap_free(OverlapInfo *oi);

/* pass.c */
PassInfo *pass_info_new(void);
void pass_info_add(PassInfo *pi, double t, OverlapInfo *oi);
void pass_info_free(PassInfo *pi);
PassCollection *pass_collection_new();
void pass_collection_add(PassCollection *pc, PassInfo *pi);

#endif