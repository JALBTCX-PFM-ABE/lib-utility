
#ifndef _INSIDE_POLYGON_H_
#define _INSIDE_POLYGON_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include "pfm_nvtypes.h"


  int32_t inside_polygon (NV_F64_COORD2 *poly, int32_t npol, double x, double y);
  int32_t inside_polygon2 (double *poly_x, double *poly_y, int32_t npol, double x, double y);
  int32_t inside_polygon3 (int32_t *xs, int32_t *ys, int32_t npol, int32_t x, int32_t y);


#ifdef  __cplusplus
}
#endif

#endif
