#include "asf.h"
#include "asf_meta.h"


meta_state_vectors *meta_state_vectors_init(int vector_count);



meta_parameters *meta_copy(meta_parameters *src)
{
  meta_parameters *ret = raw_init();

  ret->meta_version = src->meta_version;
  memcpy(ret->general, src->general, sizeof(meta_general));
  if (src->sar) {
    ret->sar = meta_sar_init();
    memcpy(ret->sar, src->sar, sizeof(meta_sar));
  }
  if (src->projection) {
    ret->projection = (meta_projection*)MALLOC(sizeof(meta_projection));
    memcpy(ret->projection, src->projection, sizeof(meta_projection));
  }
  if (src->state_vectors) {
    int vector_count = src->state_vectors->vector_count;
    ret->state_vectors = meta_state_vectors_init(vector_count);
    memcpy(ret->state_vectors, src->state_vectors, sizeof(meta_state_vectors)+vector_count*sizeof(state_loc));
  }
  if (src->stats) {
    ret->stats = meta_statistics_init(src->stats->band_count);
    memcpy(ret->stats, src->stats, sizeof(meta_stats));
  }

/* Copy Depricated structures
  memcpy(ret->geo, src->geo, sizeof(geo_parameters));
  memcpy(ret->ifm, src->ifm, sizeof(ifm_parameters));
  if (src->info) {
    ret->info = (extra_info*)MALLOC(sizeof(extra_info));
    memcpy(ret->info, src->info, sizeof(extra_info));
  }

* Set depricated redundant stVec to point at new state vectors structure *
ret->stVec = ret->state_vectors;*/

  return ret;
}
