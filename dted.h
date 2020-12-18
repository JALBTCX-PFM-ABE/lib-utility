
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

#ifndef __DTED_H__
#define __DTED_H__


#ifdef  __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "pfm_nvtypes.h"
#include "nvdef.h"


typedef struct
{
  char               sentinel[4];
  double             ll_lon;
  double             ll_lat;
  double             lon_int;
  double             lat_int;
  char               vertical_accuracy[5];
  char               security_code[3];
  char               unique_reference[13];
  uint16_t           num_lon_lines;
  uint16_t           num_lat_points;
  char               multiple_accuracy;
} UHL;


typedef struct
{
  char               sentinel[3];
  char               security_code;
  char               security_control[2];
  char               security_handling[27];
  char               reserved1[26];
  char               NIMA_designator[5];
  char               unique_reference[15];
  char               reserved2[8];
  char               edition_number[2];
  char               match_merge_version;
  char               maintenance_date[4];
  char               match_merge_date[4];
  char               maintenance_description[4];
  char               producer_code[8];
  char               reserved3[16];
  char               product_specification[9];
  char               product_spec_numbers[2];
  char               product_spec_date[4];
  char               vertical_datum[3];
  char               horizontal_datum[5];
  char               collection_system[10];
  char               latitude_origin[9];
  char               longitude_origin[10];
  char               latitude_sw[7];
  char               longitude_sw[8];
  char               latitude_nw[7];
  char               longitude_nw[8];
  char               latitude_ne[7];
  char               longitude_ne[8];
  char               latitude_se[7];
  char               longitude_se[8];
  char               orientation[9];
  char               lat_int[4];
  char               lon_int[4];
  char               num_lat_lines[4];
  char               num_lon_lines[4];
  char               partial_cell_indicator[2];
  char               reserved_NIMA[101];
  char               reserved_producer[100];
  char               reserved_free_text[156];
} DSI;


typedef struct
{
  char               sentinel[3];
  char               absolute_horizontal_accuracy[4];
  char               absolute_vertical_accuracy[4];
  char               relative_horizontal_accuracy[4];
  char               relative_vertical_accuracy[4];
  char               not_really_pad[2681];
} ACC;


typedef struct
{
  uint8_t            sentinel;
  int32_t            block_count;
  int16_t            longitude_count;
  int16_t            latitude_count;
  int16_t            elev[3601];
  uint32_t           checksum;
} DTED_DATA;



int32_t read_uhl (FILE *fp, UHL *uhl);
int32_t read_dsi (FILE *fp, DSI *dsi);
int32_t read_acc (FILE *fp, ACC *acc);
int32_t read_dted_data (FILE *fp, int32_t count, int32_t block, DTED_DATA *dted_data);
int32_t write_dted_data (FILE *fp, int32_t count, int32_t block, DTED_DATA dted_data);
void dump_uhl (UHL uhl);
void dump_dsi (DSI dsi);
void dump_acc (ACC acc);
void dump_dted_data (DTED_DATA dted_data);


#ifdef  __cplusplus
}
#endif

#endif
