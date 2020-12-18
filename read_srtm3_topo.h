
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.

*********************************************************************************************/

#ifndef _READ_SRTM3_TOPO_H_
#define _READ_SRTM3_TOPO_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include "pfm_nvtypes.h"
#include "nvdef.h"


  uint8_t check_srtm3_topo ();
  int32_t read_srtm3_topo_one_degree (int32_t lat, int32_t lon, int16_t **array);
  int16_t read_srtm3_topo (double lat, double lon);
  void cleanup_srtm3_topo ();


#ifdef  __cplusplus
}
#endif

#endif
