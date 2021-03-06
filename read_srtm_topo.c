
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


#include "read_srtm_topo.h"


static uint8_t one_open = NVFalse;
static uint8_t two_open = NVFalse;
static uint8_t three_open = NVFalse;
static uint8_t thirty_open = NVFalse;
static uint8_t no_file = NVFalse;
static uint8_t exclude_srtm2 = NVFalse;


/***************************************************************************\
*                                                                           *
*   Module Name:        set_exclude_srtm2_data                              *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2007                                       *
*                                                                           *
*   Purpose:            Set flag to force no read of srtm2 limdis data.     *
*                                                                           *
*   Arguments:          flag = NVTrue for exclude, NVFalse for include      *
*                                                                           *
*   Returns:            Nada                                                *
*                                                                           *
\***************************************************************************/


void set_exclude_srtm2_data (uint8_t flag)
{
  exclude_srtm2 = flag;
}



/***************************************************************************\
*                                                                           *
*   Module Name:        read_srtm_topo_one_degree                           *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Reads the 1, 3, and/or 30 second SRTM compressed    *
*                       topographic elevation files (*.cte) and returns a   *
*                       one-degree single dimensioned array containing the  *
*                       elevations in the same format as the srtm3 data     *
*                       files (see Caveats below).  The width/height of the *
*                       array is returned, that is, 3600 for 1 second data, *
*                       1200 for 3 second data, or 120 for 30 second data.  *
*                                                                           *
*   Arguments:          lat             -   degree of latitude, S negative  *
*                       lon             -   degree of longitude, W negative *
*                       array           -   topo array                      *
*                                                                           *
*   Returns:            0 for all water cell, 2 for undefined cell, or      *
*                       120/1200/3600 (width/height of array, it's usually  *
*                       square).  If 1800 is returned then the cell is 1800 *
*                       by 3600 (wide by high).                             *
*                                                                           *
*   Caveats:            The array is one dimensional so the user/caller     *
*                       must index into the array accordingly.  The data is *
*                       stored in order from the northwest corner of the    *
*                       cell, west to east, then north to south so the last *
*                       point in the returned array is the southeast        *
*                       corner.  See pointlat and pointlon in the following *
*                       example code:                                       *
*                                                                           *
*                                                                           *
*                       #include "read_srtm_topo.h"                         *
*                                                                           *
*                       int16_t            *array;                          *
*                       int32_t            wsize, hsize;                    *
*                       double             winc, hinc, pointlat, pointlon;  *
*                       int32_t            lat, lon;                        *
*                                                                           *
*                       if (lat < 0) lat--;                                 *
*                       if (lon < 0) lon--;                                 *
*                                                                           *
*                       wsize = read_srtm_topo_one_degree (lat, lon,        *
*                           &array);                                        *
*                                                                           *
*                       if (wsize > 2)                                      *
*                         {                                                 *
*                           hsize = wsize;                                  *
*                           if (wsize == 1800) hsize = 3600;                *
*                                                                           *
*                           winc = 1.0L / wsize;                            *
*                           hinc = 1.0L / hsize;                            *
*                           for (i = 0 ; i < hsize ; i++)                   *
*                             {                                             *
*                               pointlat = (lat + 1.0L) - (i + 1) * hinc;   *
*                               for (j = 0 ; j < wsize ; j++)               *
*                                 {                                         *
*                                   pointlon = lon + j * winc;              *
*                                   //DO SOMETHING WITH array[i * size + j] *
*                                 }                                         *
*                             }                                             *
*                         }                                                 *
*                                                                           *
*                       IMPORTANT NOTE: The latitude and longitude used to  *
*                       retrieve a one-degree cell are the lat and lon of   *
*                       the southwest corner of the cell.  Since the data   *
*                       was originally stored north to south you need to    *
*                       add one to the latitude to get the proper position  *
*                       (note the pointlat computation above).  This also   *
*                       means that you should be very careful about         *
*                       hemisphere when retrieving.  If you want the cell   *
*                       containing 19.88564N,156.3265W you need to request  *
*                       the cell 19,-157 not 19,-156.  The same holds true  *
*                       in the southern hemisphere.  If you need the cell   *
*                       holding 19.88564S,156.3265W you need to request     *
*                       -20,-157 not -19,157.                               *
*                                                                           *
*                                                                           *
*                       You should also call cleanup_srtm_topo after you    *
*                       are finished using the database in order to close   *
*                       the open files and free the associated memory.      *
*                                                                           *
*                                                                           *
\***************************************************************************/


int32_t read_srtm_topo_one_degree (int32_t lat, int32_t lon, int16_t **array)
{
  int32_t size;


  if (no_file) return (-1);


  size = read_srtm1_topo_one_degree (lat, lon, array);
  if (size != -1) one_open = NVTrue;

  if (size == -1 || size == 2)
    {
      if (!exclude_srtm2)
        {
          size = read_srtm2_topo_one_degree (lat, lon, array);
          if (size != -1) two_open = NVTrue;
        }

      if (size == -1 || size == 2)
        {
          size = read_srtm3_topo_one_degree (lat, lon, array);
          if (size != -1) three_open = NVTrue;

          if (size == -1 || size == 2) 
            {
              size = read_srtm30_topo_one_degree (lat, lon, array);
              if (size != -1) thirty_open = NVTrue;
            }
        }
    }


  if (size == -1) no_file = NVTrue;


  return (size);
}




/***************************************************************************\
*                                                                           *
*   Module Name:        read_srtm_topo                                      *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Reads the 1, 3, and/or 30 second SRTM compressed    *
*                       topographic elevation files (*.cte) and returns the *
*                       elevation value.  If the value is undefined at that *
*                       point it will return -32768.  For water it will     *
*                       return 0.                                           *
*                                                                           *
*   Arguments:          lat             -   latitude degrees, S negative    *
*                       lon             -   longitude degrees, W negative   *
*                                                                           *
*   Returns:            0 = water, -32768 undefined, elevation, or 32767 on *
*                       error                                               *
*                                                                           *
\***************************************************************************/


int16_t read_srtm_topo (double lat, double lon)
{
  static int16_t     *array;
  static int32_t     prev_ilat = -999, prev_ilon = -999, wsize = 0, hsize = 0;
  static double      winc = 0.0, hinc = 0.0;
  int32_t            ilat, ilon, lat_index, lon_index;


  if (no_file) return (32767);


  /*  If we're working in the real 0-360 world (where 0 to 0 is 0 to 360) we want to turn longitudes greater
      than 180 into negatives before we switch to the bogus 0-360 world (where -180 to 180 is 0 to 180).  */

  if (lon >= 180.0) lon -= 360.0;
  if (lon < 0.0) lon -= 1.0;  
  if (lat < 0.0) lat -= 1.0;

  ilat = (int32_t) lat;
  ilon = (int32_t) lon;


  /*  No point in calling the function if we didn't change cells.  */

  if (ilat != prev_ilat || ilon != prev_ilon) 
    {
      prev_ilat = ilat;
      prev_ilon = ilon;


      wsize = read_srtm1_topo_one_degree (ilat, ilon, &array);

      if (wsize != -1) one_open = NVTrue;

      if (wsize == 0) return (0);

      if (wsize == -1 || wsize == 2)
        {
          wsize = read_srtm2_topo_one_degree (ilat, ilon, &array);
          if (wsize != -1) two_open = NVTrue;

          if (wsize == 0) return (0);

          if (wsize == -1 || wsize == 2)
            {
              wsize = read_srtm3_topo_one_degree (ilat, ilon, &array);
              if (wsize != -1) three_open = NVTrue;

              if (wsize == 0) return (0);

              if (wsize == -1 || wsize == 2)
                {
                  wsize = read_srtm30_topo_one_degree (ilat, ilon, &array);
                  if (wsize != -1) thirty_open = NVTrue;
                }
            }
        }


      if (wsize == -1) return (32767);


      hsize = wsize;
      if (wsize == 1800) hsize = 3600;

      winc = 1.0L / (double) wsize;
      hinc = 1.0L / (double) hsize;
    }

  if (wsize == 0) return (0);
  if (wsize == 2) return (-32768);


  /*  Get the cell index.  */

  lon += 180.0;
  ilon = (int32_t) lon;

  lat += 90.0;
  ilat = (int32_t) lat;

  lat_index = (int32_t) ((((double) ilat + 1.0L) - lat) / hinc) + 1;
  lon_index = (int32_t) ((lon - (double) ilon) / winc);

  return (array[lat_index * wsize + lon_index]);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        cleanup_srtm_topo                                   *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Closes the open srtm1/2/3/30 topo files and frees   *
*                       memory.                                             *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Returns:            Nada                                                *
*                                                                           *
\***************************************************************************/


void cleanup_srtm_topo ()
{
  if (one_open) cleanup_srtm1_topo ();
  if (two_open) cleanup_srtm2_topo ();
  if (three_open) cleanup_srtm3_topo ();
  if (thirty_open) cleanup_srtm30_topo ();
  one_open = NVFalse;
  two_open = NVFalse;
  three_open = NVFalse;
  thirty_open = NVFalse;
  no_file = NVFalse;
}
