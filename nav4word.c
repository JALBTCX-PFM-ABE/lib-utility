
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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "nav4word.h"
#include "pfm_nvtypes.h"

/****************************************
// --------------------------------- 
//  uint8_t init_NAVREC(navNAVREC *)
// --------------------------------- 
//                                                    
//  Function initializes a navNAVREC struct        
//                                              
//  Returns FALSE if unable to allocate memory  
//  Returns TRUE if no issues                   
//                                                    
****************************************/

uint8_t init_NAVREC (navNAVREC *record)
{
  init_HEADREC(&record->headrec);

  record->numOfPingsAllocated = 10000;
  record->numOfSegsAllocated = 100;

  if((record->dataPings = (navDATAREC *) calloc(record->numOfPingsAllocated, sizeof(navDATAREC))) == NULL) return(NVFalse);

  int32_t x;

  for(x = 0; x < record->numOfPingsAllocated; x++)
    {
      record->dataPings[x].lat = 9999 ;
      record->dataPings[x].lon = 9999;
      record->dataPings[x].navMode = 9999;
    }
        
  if ((record->gapIndexList = (int32_t *) calloc(record->numOfSegsAllocated, sizeof(int32_t))) == NULL) return(NVFalse);

  record->numOfSegs = 0;
  record->numOfPings = 0;

  return (NVTrue);
}



/********************************************
// ---------------------------------------
//  uint8_t init_HEADREC(navHEADREC *)
// ---------------------------------------
//
//  Function initializes a navHEADREC struct
//
//  Returns TRUE
//
*********************************************/
uint8_t init_HEADREC(navHEADREC *headrec)
{
  headrec->endian = 0x00010203;
  headrec->process = 0;
  headrec->access = 0;
  headrec->datum = 0;
  strcpy(headrec->survop, "helloworld");

  return(NVTrue);
}



/********************************************
//----------------------------------------
//  uint8_t oRead_HEADREC(navHEADREC *, char *)
//----------------------------------------
//
//  Function opens a .nav file and reads ONLY the header
//
//  Returns FALSE if file errors
//  Returns TRUE if no issues
//
********************************************/

uint8_t oRead_HEADREC(navHEADREC *headrec, char *filename)
{
  FILE *inputFile;

  if((inputFile = fopen(filename, "rb")) == NULL)
    {
      printf("Unable to open %s for binary read.\n", filename);
      printf("Program will now terminate...\n\n");

      return(NVFalse);
    }
  
  if((fread(headrec, sizeof(navHEADREC), 1, inputFile)) != 0)
    {
      printf("Unable to read header from file.\n");
      printf("Program will now terminate...\n\n");

      fclose (inputFile);

      return(NVFalse);
    }


  fclose (inputFile);

  return(NVTrue);
}



/***************************************************
//-----------------------------------------------
//  uint8_t oRead_NAVREC(navNAVREC *, FILE *)
//-----------------------------------------------
//
//  Function opens and reads a .nav file
//
//        Exits program if cannot open file for read
//  Exits program if memory allocation errors
//  Returns FALSE if file errors
//  Returns TRUE if no issues
//
****************************************************/

uint8_t oRead_NAVREC(navNAVREC *record, char *filename)
{
  FILE    *inputFile;
  uint8_t isNewFormat = NVFalse;

  if((inputFile = fopen(filename, "rb")) == NULL)
    {
      printf("Unable to open %s for binary read.\n", filename);
      printf("Program will now terminate...\n\n");   

      exit(-2);
    }
  else
    {
      if((fread(&record->headrec, sizeof(navHEADREC), 1, inputFile)) == 0)
        {
          printf("Error reading nav header from file.\n");

          fclose(inputFile);

          return(NVFalse);
        }
      else if((fread(&record->dataPings[record->numOfPings++], sizeof(navDATAREC), 1, inputFile)) == 0)
        {
          printf("Error reading nav data from file.\n");

          fclose(inputFile);

          return(NVFalse);
        }
      else
        {
          if(record->dataPings[record->numOfPings-1].navMode & 0x80000000)
            {
              isNewFormat = NVTrue;
              record->gapIndexList[record->numOfSegs++] = record->numOfPings - 1;
            }
        }

      if(isNewFormat)
        {
          while((fread(&record->dataPings[record->numOfPings++], sizeof(navDATAREC), 1, inputFile)) != 0)
            {
              if(record->dataPings[record->numOfPings-1].navMode & 0x80000000)
                {
                  if(record->numOfSegs >= record->numOfSegsAllocated)
                    {
                      if(!check_mem_gapIndexList(record, 100))
                        {
                          exit(-2);
                        }
                    }

                  record->gapIndexList[record->numOfSegs++] = record->numOfPings - 1;
                }

              if(!check_mem_dataPings(record, 10000))
                {
                  exit(-2);
                } 
            }
        }
      else
        {
          while((fread(&record->dataPings[record->numOfPings++], sizeof(navDATAREC), 1, inputFile)) != 0)
            {
              if(record->numOfPings >= record->numOfPingsAllocated)
                {
                  if(!check_mem_dataPings(record, 10000))
                    {
                      exit(-2);
                    }
                }
            }
        }
    }

  record->numOfPings--;

  fclose(inputFile);

  return(NVTrue);
}



/***************************************************
//-----------------------------------------------
//  uint8_t write_NAVREC(navNAVREC *, FILE *)
//-----------------------------------------------
//
//  Function writes a .nav file
//
//  Returns FALSE if file errors or allocate errors
//  Returns TRUE if no issues
//
***************************************************/

uint8_t write_NAVREC(navNAVREC *record, char *filename)
{
  FILE      *outputFile;
  int32_t   count;

  if((outputFile = fopen(filename, "wb")) == NULL)
    {
      printf("Unable to open %s for binary write.\n", filename);
      printf("Program will now terminate...\n\n");

      return(NVFalse);
    }
  else
    {
      if((fwrite(&record->headrec, sizeof(navHEADREC), 1, outputFile)) != 1)
        {
          printf("Error writing header to binary file.\n");
          printf("Program will now terminate...\n\n");

          return(NVFalse);
        }
    
      for(count = 0; count < record->numOfPings; count++)
        {
          if(fwrite(&record->dataPings[count], sizeof(navDATAREC), 1, outputFile) != 1)
            {
              printf("Unable to write data record to nav file!  Skipping record.\n");
            }
        }
    }
  
  fclose(outputFile);
  
  return(NVTrue);
}



/***********************************************************************
// -------------------------------------------------------------------
//        uint8_t make_NAV_Header(navNAVREC *, int32_t, int32_t, int32_t
//                                int16_t, int16_t, char *)
// -------------------------------------------------------------------
//        
//        Function creates a navHEADREC struct which is the header of a .nav file
//
//        Returns FALSE if navNAVREC struct was not initialized
//        Returns TRUE if no issues
//
***********************************************************************/

uint8_t make_NAV_Header(navNAVREC *record, int32_t endian, int32_t ship, int32_t process, int16_t access, int16_t datum, char *survop)
{
  if(record == NULL)
    {
      printf("Your NAVREC struct was not initialized.\n");
      printf("This will cause null references downstream.\n");

      return(NVFalse);
    }

  record->headrec.endian = endian;
  record->headrec.ship = ship;        
  record->headrec.process = process;
  record->headrec.access = access;
  record->headrec.datum = datum;
  strcpy(record->headrec.survop, survop);

  return(NVTrue);
}



/********************************************************************************
// ----------------------------------------------------------------------------
//        int32_t make_NAV_POS_DataRec(navNAVREC *, int32_t, double, double,
//                                      int32_t, uint8_t)
// ----------------------------------------------------------------------------
//
//  Function creates a navDATAREC struct and stores it in the navNAVREC dataPings array
//  Function requires POS/SBET time as input
//
//  NOTE:  For all other file types/time, use make_NAV_NRML_DataRec()
//
//  Returns  1 if no issues and flag for new line segment is FALSE
//  Returns  2 if no issues and flag for new line segment is TRUE
//
*********************************************************************************/

int32_t make_NAV_POS_DataRec(navNAVREC *record, int32_t time, double lat, double lon, int32_t mode, uint8_t flag)
{
  record->dataPings[record->numOfPings].time = time;
  record->dataPings[record->numOfPings].lat = (int32_t) (lat * 10000000);
  record->dataPings[record->numOfPings].lon = (int32_t) (lon * 10000000);
  record->dataPings[record->numOfPings].navMode = mode;

  if(flag)
    {
      flag_New_LineSeg(record, record->numOfPings);
    }
  else
    {
      record->numOfPings++;
      return(1);
    }

  record->numOfPings++;

  return(2);
}



/********************************************************************************
// ----------------------------------------------------------------------------
//  int32_t make_NAV_NRML_DataRec(navNAVREC *, int32_t, double, double,
//                                 int32_t, uint8_t)
// ----------------------------------------------------------------------------
//
//  Function creates a navDATAREC struct and stores it in the NAVREC dataPings array
//  Function is not for POS/SBET time
//  Function is for all other times - {GSF, HAWKEYE, WLF, ...}
//
//  NOTE:  For POS/SBET time, use make_NAV_POS_DataRec()
//
//  Returns  1 if no issues and flag for new line segment is FALSE
//  Returns  2 if no issues and flag for new line segment is TRUE
//
********************************************************************************/

int32_t make_NAV_NRML_DataRec(navNAVREC *record, int32_t time, double lat, double lon, int32_t mode, uint8_t flag)
{
  record->dataPings[record->numOfPings].time = time;
  record->dataPings[record->numOfPings].lat = (int32_t) (lat * 10000000);
  record->dataPings[record->numOfPings].lon = (int32_t) (lon * 10000000);
  record->dataPings[record->numOfPings].navMode = mode;

  if(flag)
    {
      flag_New_LineSeg(record, record->numOfPings);
    }
  else
    {
      record->numOfPings++;
      return(1);
    }

  record->numOfPings++;

  return(2);
}



/*******************************************************************************
//---------------------------------------------------------------------------
//  uint8_t make_NAVREC_from_old(navNAVREC *, navHEADREC *, navDATAREC *)
//---------------------------------------------------------------------------
//
//  Function takes old header and old nav data array and makes a new navrec
//
//  NOTE:  Does not CONVERT to new format, it merely loads it into the new struct
//         IE:  It saves into the navNAVREC struct but DOES NOT have negative navModes
//
//  Returns TRUE
//
*******************************************************************************/

uint8_t make_NAVREC_from_old(navNAVREC *record, navHEADREC *header, navDATAREC *dataPings)
{
  make_NAV_Header(record, header->endian, header->ship, header->process, header->access, header->datum, header->survop);

  record->dataPings = dataPings;

  return(NVTrue);
}



/**********************************************************
//------------------------------------------------------
//  uint8_t check_mem_gapIndexList(navNAVREC *, int32_t)
//------------------------------------------------------
//
//  Function reallocates numOfSegs by a user defined amount
//
//  Returns FALSE if memory errors
//  Returns TRUE if no issues
//
**********************************************************/

uint8_t check_mem_gapIndexList(navNAVREC *record, int32_t number)
{
  if(record->numOfSegs >= record->numOfSegsAllocated)
    {
      record->numOfSegsAllocated += number;

      if((record->gapIndexList = (int32_t *) (realloc(record->gapIndexList, record->numOfSegsAllocated * sizeof(int32_t)))) == NULL)
        {
          printf("Unable to reallocate memory for gapIndexList.\n");
          printf("Program will now terminate...\n\n");

          return(NVFalse);
        }
    }

  return(NVTrue);
}



/*************************************************************
//---------------------------------------------------------
//  uint8_t check_mem_dataPings(navNAVREC *, int32_t)
//---------------------------------------------------------
//
//  Function reallocates dataPings by a user defined amount
//
//  Returns FALSE if memory errors
//  Returns TRUE if no issues
//
*************************************************************/

uint8_t check_mem_dataPings(navNAVREC *record, int32_t number)
{
  if(record->numOfPings >= record->numOfPingsAllocated)
    {
      record->numOfPingsAllocated += number;

      if((record->dataPings = (navDATAREC *) (realloc(record->dataPings, record->numOfPingsAllocated * sizeof(navDATAREC)))) == NULL)
        { 
          printf("Unable to reallocate memory for dataPings.\n");
          printf("Program will now terminate...\n\n");

          return(NVFalse);
        }
    }

  return(NVTrue);
}



/*********************************************
// -----------------------------------------
//  uint8_t flag_New_LineSeg(navNAVREC *, int32_t)
// -----------------------------------------
//
//  Function updates navNAVREC with a new line segment
//  This is done by flagging the 32b of the 32b signed int
//
//  Returns FALSE if memory issues Returns TRUE
//
*********************************************/

uint8_t flag_New_LineSeg(navNAVREC *record, int32_t index)
{
  if(record->numOfSegs >= record->numOfSegsAllocated)
    {
      record->numOfSegsAllocated += 100;

      if((record->gapIndexList = (int32_t *) (realloc(record->gapIndexList, record->numOfSegsAllocated * sizeof(int32_t)))) == NULL)
        {
          printf("Unable to reallocate memory for gapIndexList.\n");
          printf("Program will now terminate...\n\n");

          return(NVFalse);
        }
    }

  /*        record->dataPings[index].navMode = record->dataPings[index].navMode ^ 0x80000000;*/
  record->dataPings[index].navMode *= -1;
  record->gapIndexList[record->numOfSegs++] = index;

  return(NVTrue);
}



/*****************************************************************
// -------------------------------------------------------------
//  uint8_t check_For_LineSeg(navNAVREC *record, int32_t check)
// -------------------------------------------------------------
//
//  Function checks for new line segment at index of check
//
//  Returns TRUE if there is a line split at index check
//  Returns FALSE if there is not a line split at index check
//
*****************************************************************/

uint8_t check_For_LineSeg(navNAVREC *record, int32_t check)
{
  if(record->dataPings[check].navMode & 0x80000000) return(NVTrue);

  return(NVFalse);
}



/******************************************************************************
// --------------------------------------------------------------------------
//  int32_t convert_1960_time(int32_t *, int32_t *, int32_t *, int32_t *,
//                             float *)
// --------------------------------------------------------------------------
//
//  Function takes a time from the standard 1970 epoch and formats the time to 
//  the official NAV format of 01/01/1960 epoch with years as 366 days
//
//  Returns the converted time as an int32_t
//
******************************************************************************/

int32_t convert_1960_time (int32_t *conv_year, int32_t *conv_jday, int32_t *conv_hour, int32_t *conv_min, float *conv_sec)
{
  int32_t   convertedTime;

  convertedTime = ((*conv_year - 60) * 31622400) + ((*conv_jday - 1) * 86400) + (*conv_hour * 3600) + (*conv_min * 60) + (int32_t) (*conv_sec);

  return(convertedTime);
}



/******************************************************
// --------------------------------------------------
//  int32_t nav_compare(const void *, const void *)
// --------------------------------------------------
//
//  Function is specifically used for the qsort() call in the
//  sortDataPings() function
//  Function compares two times in two different DATAREC structs
//  and returns back to the qsort() call a value
//
//  NOTE:  This function is SPECIFICALLY for the qsort() call
//
//  Returns a -1 if *arg1 time is less than *arg2 time
//  Returns a  1 if *arg1 time is greater than *arg2 time
//
******************************************************/

int32_t nav_compare(const void *arg1, const void *arg2)
{
  navDATAREC *record1;
  navDATAREC *record2;

  record1 = (navDATAREC *) arg1;
  record2 = (navDATAREC *) arg2;

  if (record1->time < record2->time) return(-1);

  if (record1->time > record2->time) return(1);

  return(1);
}



/****************************************************************
// -----------------------------------------------------
//  uint8_t navRecSplitter(navNAVREC **, int32_t *, int32_t)
// -----------------------------------------------------
//
//  Function takes the current navNAVREC data structure and adds
//  another record dynamically during runtime
//  It then updates the total numOfRecs and the navNAVREC index
//
//  Exits with code (-2) if it has memory allocation issues
//  Returns TRUE if the record was split
//
****************************************************************/

uint8_t NavRecSplitter(navNAVREC **records, int32_t numOfRecs, __attribute__ ((unused)) int32_t count)
{
  if((records = (navNAVREC **) realloc(records, (numOfRecs + 1) * sizeof(navNAVREC))) == NULL)
    {
      printf("\nALLOCATION ERROR:  Unable to allocate for navNAVREC **records\n");
      printf("\nProgram will now terminate...\n\n");

      exit(-2);
    }
  else if((records[numOfRecs + 1] = (navNAVREC *) realloc(records[numOfRecs + 1], 1 * sizeof(navNAVREC))) == NULL)
    {
      printf("\nALLOCATION ERROR:  Unable to allocate for navNAVREC *records\n");
      printf("\nProgram will now terminate...\n\n");

      exit(-2);
    }
  else
    {
      init_NAVREC(records[numOfRecs + 1]);
    }

  return(NVTrue);
}



/****************************************
// ------------------------------------
//  uint8_t sortDataPings(navNAVREC *)
// ------------------------------------
//
//  Function uses built in qsort() call to sort
//  the navDATAREC structs by time
//
//  Returns FALSE if passed record is NULL
//  Returns TRUE if no issues
//
****************************************/

uint8_t sortDataPings(navNAVREC *record)
{
  if(record != NULL)
    {
      qsort(record->dataPings, record->numOfPings, sizeof(navDATAREC), nav_compare);
    }
  else
    {
      return(NVFalse);
    }

  return(NVTrue);
}



/********************************************
//----------------------------------------
//  uint8_t sortNavRecs(navNAVREC **, int32_t)
//----------------------------------------
//
//  Function sorts navNAVREC** at dataPings[0]
//
//  NOTE:  Function requres SPECIFIC data structure
//
//  Returns TRUE
//
********************************************/

uint8_t sortNavRecs(navNAVREC **records, int32_t numOfRecs)
{
  int32_t i;
  int32_t j;
  int32_t j2;

  navNAVREC *temp;

  for(i = 0; i < numOfRecs; i++)
    {
      if(records[i] != NULL)
        {
          for(j = numOfRecs - 1; j > i; j--)
            {
              if(records[j] != NULL)
                {
                  j2 = j - 1;

                  while(records[j2] == NULL)
                    {
                      j2--;

                      if(j2 == i) break;
                    }

                  if(j2 > i)
                    {
                      if(records[j]->dataPings[0].time < records[j2]->dataPings[0].time)
                        {
                          temp = records[j];
                          records[j] = records[j2];
                          records[j2] = temp;
                        }
                    }
                }
            }
        }
    }

  return(NVTrue);
}



/**********************************************************
//------------------------------------------------------
//  uint8_t mergeNavRecs(navNAVREC **, int32_t)
//------------------------------------------------------
//
//  Function mergers all navRecords into a single nav records
//
//  NOTE: Function requires SPECIFIC data structure
//
//  Exits program if memory allocation fails
//  Returns TRUE if no issues
//
**********************************************************/

uint8_t mergeNavRecs(navNAVREC **records, int32_t numOfRecs)
{
  int32_t x = 0;
  int32_t y;
  int32_t maxTime;
  int32_t currentNum = 0;

  while(records[x] == NULL)
    {
      x++;
  
      if(x == numOfRecs) break;
    }

  if(x == numOfRecs) return(NVFalse);

  currentNum = x;

  maxTime = records[currentNum]->dataPings[records[currentNum]->numOfPings-1].time;

  for(x = currentNum + 1; x < numOfRecs; x++)
    {
      if(records[x] != NULL)
        {
          if(maxTime >= records[x]->dataPings[0].time)
            {
              for(y = 0; y < records[x]->numOfPings; y++)
                {
                  if(records[currentNum]->numOfPings >= records[currentNum]->numOfPingsAllocated)
                    {
                      if(!check_mem_dataPings(records[currentNum], 10000)) exit(-2);

                      printf("\n# of Pings Allocated is %d.  # of Pings is %d.\n\n", records[currentNum]->numOfPingsAllocated, records[currentNum]->numOfPings);
                    }

                  records[currentNum]->dataPings[records[currentNum]->numOfPings].time = records[x]->dataPings[y].time;
                  records[currentNum]->dataPings[records[currentNum]->numOfPings].lat = records[x]->dataPings[y].lat;
                  records[currentNum]->dataPings[records[currentNum]->numOfPings].lon = records[x]->dataPings[y].lon;
                  records[currentNum]->dataPings[records[currentNum]->numOfPings++].navMode = records[x]->dataPings[y].navMode;
                }

              if(maxTime < records[x]->dataPings[records[x]->numOfPings - 1].time) maxTime = records[x]->dataPings[records[x]->numOfPings - 1].time;

              free_NAVREC(records[x]);
              free(records[x]);
              records[x] = NULL;
            }
          else
            {
              currentNum = x;
            }
        }
    }

  return(NVTrue);
}



/*********************************************************
//-----------------------------------------------------
//  uint8_t createMasterRec(navNAVREC **, int32_t
//-----------------------------------------------------
//
//  Function takes the merged nav records and makes a master rec
//
//  NOTE: Function requires SPECIFIC data structure
//
//  Exits program if memory allocation fails
//  Returns TRUE if no issues
//
*********************************************************/

uint8_t createMasterRec(navNAVREC **records, int32_t numOfRecs)
{
  int32_t x;
  int32_t y;

  for(x = 0; x < numOfRecs; x++)
    {
      if(records[x] != NULL)
        {
          for(y = 0; y < records[x]->numOfPings; y++)
            {
              if(records[numOfRecs]->numOfPings >= records[numOfRecs]->numOfPingsAllocated)
                {
                  if(!check_mem_dataPings(records[numOfRecs], 10000)) exit(-2);

                  printf("\n# of Pings Allocated is %d.  # of Pings is %d.\n\n", records[numOfRecs]->numOfPingsAllocated, records[numOfRecs]->numOfPings);
                }

              if(records[x]->dataPings[y].navMode == 0) records[x]->dataPings[y].navMode = 9999;
    
              if(y == 0) records[x]->dataPings[y].navMode = records[x]->dataPings[y].navMode * -1;

              records[numOfRecs]->dataPings[records[numOfRecs]->numOfPings].time = records[x]->dataPings[y].time;
              records[numOfRecs]->dataPings[records[numOfRecs]->numOfPings].lat = records[x]->dataPings[y].lat;
              records[numOfRecs]->dataPings[records[numOfRecs]->numOfPings].lon = records[x]->dataPings[y].lon;
              records[numOfRecs]->dataPings[records[numOfRecs]->numOfPings++].navMode = records[x]->dataPings[y].navMode;

              if(records[x]->dataPings[y].navMode & 0x80000000)
                {
                  if(records[numOfRecs]->numOfSegs >= records[numOfRecs]->numOfSegsAllocated)
                    {
                      if(!check_mem_gapIndexList(records[numOfRecs], 100)) exit(-2);
                    }
    
                  records[numOfRecs]->gapIndexList[records[numOfRecs]->numOfSegs++] = records[numOfRecs]->numOfPings - 1;
                }
            }

          free_NAVREC(records[x]);
          free(records[x]);
          records[x] = NULL;
        }
    }

  return(NVTrue);
}



/************************************************************
//--------------------------------------------------------
//  uint8_t completeNavRecSort(navNAVREC **, int32_t
//--------------------------------------------------------
//
//  Function does the ENTIRE suite of making a complete nav record
//
//  NOTE:Function requires SPECIFIC data structure
//
//  Exits program if memory allocation errors
//  Returns TRUE if no issues
//
************************************************************/

uint8_t completeNavRecSort(navNAVREC **records, int32_t numOfRecs)
{
  int32_t x;

  for(x = 0; x < numOfRecs; x++)
    {
      if(records[x] != NULL) sortDataPings(records[x]);
    }

  sortNavRecs(records, numOfRecs);

  mergeNavRecs(records, numOfRecs);

  for(x = 0; x < numOfRecs; x++)
    {
      if(records[x] != NULL) sortDataPings(records[x]);
    }

  createMasterRec(records, numOfRecs);  

  return(NVTrue);
}



/*******************************************
// ------------------------------
//  void        free_NAVREC(navNAVREC *)
// ------------------------------
//
//  Function frees any memory allocated by a 
//  navNAVREC struct
//
//  Returns nothing - VOID
//
*******************************************/

void free_NAVREC(navNAVREC *record)
{
  if(record->dataPings != NULL) free(record->dataPings);
  if(record->gapIndexList != NULL) free(record->gapIndexList);
}
