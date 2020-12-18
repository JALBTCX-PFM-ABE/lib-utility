
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


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! are being used by Doxygen to document the
    software.  Dashes in these comment blocks are used to create bullet lists.  The lack of
    blank lines after a block of dash preceeded comments means that the next block of dash
    preceeded comments is a new, indented bullet list.  I've tried to keep the Doxygen
    formatting to a minimum but there are some other items (like <br> and <pre>) that need
    to be left alone.  If you see a comment that starts with / * ! and there is something
    that looks a bit weird it is probably due to some arcane Doxygen syntax.  Be very
    careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#ifndef _CHRTR_H_
#define _CHRTR_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pfm_nvtypes.h"
#include "nvdef.h"
#include "swap_bytes.h"


#define CHRTRNULL               10000000000000000.0  /*!<  CHRTR null value  */


#define MAX_CHRTR_FILES         32                   /*!<  MAximum number of CHRTR files that may be opened at once  */


  typedef struct 
  {
    float               wlon;                        /*!<  Western longitude (west negative)  */
    float               elon;                        /*!<  Eastern longitude (west negative)  */
    float               slat;                        /*!<  Southern latitude (south negative)  */
    float               nlat;                        /*!<  Northern latitude (south negative)  */
    float               grid_minutes;                /*!<  Grid size in minutes  */
    int32_t             width;                       /*!<  Width in grid cells  */
    int32_t             height;                      /*!<  Height in grid cells  */
    int32_t             endian;                      /*!<  Endian indicator (0x00010203)  */
    float               min_z;                       /*!<  Minimum observed Z value in file  */
    float               max_z;                       /*!<  Maximum observed Z value in file  */
  } CHRTR_HEADER;


  void bit_set (float *value, int32_t bit, int32_t binary);
  int32_t bit_test (float value, int32_t bit);
  int32_t open_chrtr (const char *path, CHRTR_HEADER *header);
  int32_t create_chrtr (const char *path, CHRTR_HEADER *header);
  void close_chrtr (int32_t hnd);
  uint8_t read_chrtr (int32_t hnd, int32_t row, int32_t start_col, int32_t num_cols, float *data);
  uint8_t write_chrtr (int32_t hnd, int32_t row, int32_t start_col, int32_t num_cols, float *data);
  uint8_t get_chrtr_value (int32_t hnd, double lat, double lon, float *value);
  void dump_chrtr_header (int32_t hnd);



#ifdef  __cplusplus
}
#endif

#endif
