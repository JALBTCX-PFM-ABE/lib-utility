
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

#ifndef NAV4WORD_LIB_H_
#define NAV4WORD_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pfm_nvtypes.h"


  /*NAV format for header*/

  typedef struct
  {
    int32_t         endian;
    int32_t         ship;
    int32_t         process;
    int16_t         access;
    int16_t         datum;
    char            survop[16];
  } navHEADREC;


  /*NAV format for data*/

  typedef struct
  {
    int32_t         time;
    int32_t         lat;
    int32_t         lon;
    int32_t         navMode;
  } navDATAREC;


  /*NAV container for simplicity and for awesome*/

  typedef struct
  {
    navHEADREC      headrec;
    navDATAREC      *dataPings;
    int32_t         numOfPings;
    int32_t         *gapIndexList;
    int32_t         numOfSegs;
    int32_t         numOfSegsAllocated;
    int32_t         numOfPingsAllocated;
  } navNAVREC;


  /*Library function declarations*/

  uint8_t init_NAVREC(navNAVREC *);
  uint8_t init_HEADREC(navHEADREC *);
  uint8_t oRead_HEADREC(navHEADREC *, char *);
  uint8_t oRead_NAVREC(navNAVREC *, char *);
  uint8_t write_NAVREC(navNAVREC *, char *);
  uint8_t make_NAV_Header(navNAVREC *, int32_t, int32_t, int32_t, int16_t, int16_t, char *);
  int32_t make_NAV_POS_DataRec(navNAVREC *, int32_t, double, double, int32_t, uint8_t);
  int32_t make_NAV_NRML_DataRec(navNAVREC *, int32_t, double, double, int32_t, uint8_t);
  uint8_t make_NAVREC_from_old(navNAVREC *, navHEADREC *, navDATAREC *);
  uint8_t check_mem_gapIndexList(navNAVREC *, int32_t);
  uint8_t check_mem_dataPings(navNAVREC *, int32_t);
  uint8_t flag_New_LineSeg(navNAVREC *, int32_t);
  uint8_t check_For_LineSeg(navNAVREC *, int32_t);
  int32_t convert_1960_time(int32_t *, int32_t *, int32_t *, int32_t *, float *);
  int32_t nav_compare(const void *, const void *);
  uint8_t NavRecSplitter(navNAVREC **, int32_t, int32_t);
  uint8_t sortDataPings(navNAVREC *);
  uint8_t sortNavRecs(navNAVREC **, int32_t);
  uint8_t mergeNavRecs(navNAVREC **, int32_t);
  uint8_t createMasterRec(navNAVREC **, int32_t);
  uint8_t completeNavRecSort(navNAVREC **, int32_t);
  void free_NAVREC(navNAVREC *); 

#ifdef __cplusplus
}
#endif

#endif
