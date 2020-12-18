
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


/*

    Description of the compressed topographic elevation (.cte) file format (look Ma, no endians!)



    Header - 16384 bytes, ASCII

    [HEADER SIZE] = 16384
    [CREATION DATE] = 
    [VERSION] = 
    [ZLIB VERSION] =
    [END OF HEADER]


    One-degree map - 64800 * 36 bits, double precision integer, stored as characters.
    
        Records start at 90S,180W and proceed west to east then south to north (that is, the second record
        is for 90S,179W and the 361st record is for 89S,180W).
        Record contains 0 if all water, 2 if undefined, or address.


    Data -

        3 bits  - Not used (originally planned for resolution in mixed files)
        30 bits - size of the zlib level 9 compressed block (SB)
        31 bits - size of the uncompressed block
        SB bytes - data

        Inside the compressed block the data has already been delta coded and bit packed.  The format of the
        data stored in the compressed block is as follows:

            16 bits - signed short, starting value, stored as characters
            16 bits - signed short, bias value, stored as characters
            4 bits  - number of bits used to store delta coding offsets (NB)
            NB bits - first offset
            NB bits - offset from first
            NB bits - offset from second
            .
            .
            .
            NB bits - last offset (may be the 12,960,000th, 1,440,000th, or 14,400th offset depending on 
            resolution)

            Undefined values (-32768) will be stored as (int) pow (2.0L, NB) - 1 and will not be used in the
            delta chain.

            The deltas are computed by subtracting the previous valid value from the current value.  The data
            is traversed west to east for one row, then east to west for the next row, then west to east, etc.
            I call this the snake dance (named after the lines at Disney World ;-)  After the delta is computed
            the bias is added to it.
            
            
        The compression is compliments of the ZLIB compression library which can be found at 
        http://www.zlib.net/.  Many thanks to Jean-loup Gailly, Mark Adler, and all others associated with
        that effort.


*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <zlib.h>


#include "pfm_nvtypes.h"
#include "nvdef.h"
#include "bit_pack.h"


#define HEADER_SIZE      16384
#define MAP_BYTES        (64800 * 36) / 8


static uint8_t           no_file = NVFalse, first = NVTrue;
static int32_t           prev_size = -1;
static FILE              *fp = NULL;
static int16_t           *box = NULL;
static uint8_t           *map, block_map[64800];;



/***************************************************************************\
*                                                                           *
*   Module Name:        check_srtm1_topo                                    *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       November 2007                                       *
*                                                                           *
*   Purpose:            Checks for availability of the                      *
*                       SRTM1 topo data file.                               *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Returns:            NVTrue or NVFalse                                   *
*                                                                           *
\***************************************************************************/


uint8_t check_srtm1_topo ()
{
  char       dir[512], file[512];
  FILE       *block_fp;

  if (getenv ("ABE_DATA") == NULL)
    {
      no_file = NVTrue;
      return (NVFalse);
    }

  strcpy (dir, getenv ("ABE_DATA"));


  /*  Check the block map file.  */

  sprintf (file, "%s%1csrtm_data%1csrtm1%1csrtm1_block_map.dat", dir, SEPARATOR, SEPARATOR, SEPARATOR);

  if ((block_fp = fopen (file, "rb")) == NULL)
    {
      no_file = NVTrue;
      return (NVFalse);
    }
  fclose (block_fp);


  return (NVTrue);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        read_srtm1_topo_one_degree                          *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Reads the SRTM compressed topographic elevation     *
*                       file (*.cte) and returns a one-degree single        *
*                       dimensioned array containing the elevations in the  *
*                       same format as the srtm1 data files.  The           *
*                       width/height of the array is returned.              *
*                                                                           *
*   Arguments:          lat             -   degree of latitude, S negative  *
*                       lon             -   degree of longitude, W negative *
*                       array           -   topo array                      *
*                                                                           *
*   Returns:            0 for all water cell, 2 for undefined cell, 3600    *
*                       (width/height of array, it's square), or -1 on error*
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
*                       #include "read_srtm1_topo.h"                        *
*                                                                           *
*                       int16_t            *array;                          *
*                       int32_t            size;                            *
*                       double             inc, pointlat, pointlon;         *
*                                                                           *
*                       size = read_srtm1_topo_one_degree (lat, lon,        *
*                           &array);                                        *
*                                                                           *
*                       if (size > 2)                                       *
*                         {                                                 *
*                           inc = 1.0L / size;                              *
*                           for (i = 0 ; i < size ; i++)                    *
*                             {                                             *
*                               pointlat = (lat + 1.0L) - (i + 1) * inc;    *
*                               for (j = 0 ; j < size ; j++)                *
*                                 {                                         *
*                                   pointlon = lon + j * inc;               *
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
*                       -20,-157 not -19,-156.                              *
*                                                                           *
*                                                                           *
*                       You should also call cleanup_srtm1_topo after you   *
*                       are finished using the database in order to close   *
*                       the open file and free the associated memory.       *
*                                                                           *
\***************************************************************************/


int32_t read_srtm1_topo_one_degree (int32_t lat, int32_t lon, int16_t **array)
{
  static char            dir[512], file[512], version[128], zversion[128];
  static int32_t         header_size, prev_lat = -999, prev_lon = -999, prev_block = -1;
  FILE                   *block_fp;
  char                   varin[1024], info[1024], header_block[HEADER_SIZE];
  uint8_t                *buf, *bit_box = NULL, head[8];
  int32_t                i, j, shift_lat, shift_lon, pos, size = 0, status, ndx, block;
  uLong                  csize;
  uLongf                 bsize;
  int64_t                address;
  int16_t                start_val, bias, null_val, num_bits, temp, last_val;

  if (no_file) return (-1);


  /*  First time through, open the file and read the header.    */

  if (first)
    {
      if (getenv ("ABE_DATA") == NULL)
        {
          fprintf (stderr, "Unable to find ABE_DATA environment variable\n");
          fflush (stderr);
          no_file = NVTrue;
          return (-1);
        }

      strcpy (dir, getenv ("ABE_DATA"));

      sprintf (file, "%s%1csrtm_data%1csrtm1%1csrtm1_block_map.dat", dir, SEPARATOR, SEPARATOR, SEPARATOR);


      if ((block_fp = fopen (file, "rb")) == NULL)
        {
          perror (file);
          no_file = NVTrue;
          return (-1);
        }


      fseek (block_fp, 0, SEEK_SET);
      if (!fread (block_map, sizeof (block_map), 1, block_fp))
	{
	  fprintf (stderr, "Bad return in file %s, function %s at line %d.  This should never happen!", __FILE__, __FUNCTION__, __LINE__ - 2);
	  fflush (stderr);
          fclose (block_fp);
	  return (-1);
	}

      fclose (block_fp);

      first = NVFalse;
    }


  /*  If we're working in the real 0-360 world (where 0 to 0 is 0 to 360) we want to turn longitudes greater
      than 180 into negatives before we switch to the bogus 0-360 world (where -180 to 180 is 0 to 180).  */

  if (lon >= 180) lon -= 360;


  /*  Shift into a 0 to 180 by 0 to 360 world.  */

  shift_lat = lat + 90;
  shift_lon = lon + 180;


  block = shift_lat * 360 + shift_lon;


  if (block_map[block] != prev_block)
    {
      /*  If the block_map value is 0 then no data was loaded for this cell.  */

      if (!block_map[block]) return (2);

      prev_block = block;
      sprintf (file, "%s%1csrtm1%1cRegion_0%1d.cte", dir, SEPARATOR, SEPARATOR, block_map[block]);


      /*  Note that we are returning undefined if we can't find the file.  This is to allow the use of 
          a subset of the area block files without requiring all of the files to be present.  */

      if (fp) fclose (fp);
      if ((fp = fopen (file, "rb")) == NULL) return (2);


      /*  Allocate the map memory.  */

      if (map) free (map);
      map = (uint8_t *) calloc (MAP_BYTES, sizeof (uint8_t));
      if (map == NULL)
        {
          perror ("Allocating map memory");
          exit (-1);
        }


      /*  Read the header block.  */

      if (!fread (header_block, HEADER_SIZE, 1, fp))
	{
	  fprintf (stderr, "Bad return in file %s, function %s at line %d.  This should never happen!", __FILE__, __FUNCTION__, __LINE__ - 2);
	  fflush (stderr);
	  return (-1);
	}


      ndx = 0;
      while (header_block[ndx] != 0)
        {
          for (i = 0 ; i < 1024 ; i++)
            {
              if (header_block[ndx] == '\n') break;
              varin[i] = header_block[ndx];
              ndx++;
            }

          varin[i] = 0;

          if (strstr (varin, "[END OF HEADER]")) break;


          /*  Put everything to the right of the equals sign in 'info'.   */

          if (strchr (varin, '=') != NULL) strcpy (info, (strchr (varin, '=') + 1));

          if (strstr (varin, "[VERSION]")) strcpy (version, info);

          if (strstr (varin, "[ZLIB VERSION]"))
            {
              strcpy (zversion, info);

              sscanf (zversion, "%d.", &i);
              sscanf (zlibVersion (), "%d.", &j);

              if (i != j)
                {
                  fprintf (stderr, "\n\nZlib library version (%s) is not compatible with version used to build SRTM file (%s)\n\n",
                           zlibVersion (), zversion);
                  exit (-1);
                }
            }

          if (strstr (varin, "[HEADER SIZE]")) sscanf (info, "%d", &header_size);

          ndx++;
        }


      if (header_size != HEADER_SIZE)
        {
          fprintf (stderr, "Header sizes do not match, WTF, over!\n");
          exit (-1);
        }


      /*  Move past the end of the header and read the map.  */

      fseek (fp, (int64_t) header_size, SEEK_SET);
      if (!fread (map, MAP_BYTES, 1, fp))
	{
	  fprintf (stderr, "Bad return in file %s, function %s at line %d.  This should never happen!", __FILE__, __FUNCTION__, __LINE__ - 2);
	  fflush (stderr);
	  return (-1);
	}

      first = NVFalse;
    }


  /*  Only read the data if we have changed one-degree cells.  */

  if (prev_lat != shift_lat || prev_lon != shift_lon)
    {
      /*  Unpack the address from the map.  */

      address = double_bit_unpack (map, (shift_lat * 360 + shift_lon) * 36, 36);

      prev_lat = shift_lat;
      prev_lon = shift_lon;


      /*  If the address is 0 (water) or 2 (undefined), return the address.  */

      if (address < header_size) return ((int32_t) address);


      /*  Move to the address and read/unpack the header.  */

      fseek (fp, address, SEEK_SET);
      if (!fread (head, 8, 1, fp))
	{
	  fprintf (stderr, "Bad return in file %s, function %s at line %d.  This should never happen!", __FILE__, __FUNCTION__, __LINE__ - 2);
	  fflush (stderr);
	  return (-1);
	}

      pos = 0;
      csize = (uLong) bit_unpack (head, pos, 30); pos += 30;
      bsize = (uLongf) bit_unpack (head, pos, 31);


      size = 3600;


      /*  We have to set an approximate size for unpacking (see the ZLIB documentation).  */

      bsize += NINT ((float) bsize * 0.10) + 12;


      /*  Allocate the uncompressed memory.  */

      bit_box = (uint8_t *) calloc (bsize, sizeof (uint8_t));
      if (bit_box == NULL)
        {
          perror ("Allocating bit_box memory in read_srtm1_topo");
          exit (-1);
        }


      /*  Allocate the compressed memory.  */

      buf = (uint8_t *) calloc (csize, sizeof (uint8_t));
      if (buf == NULL)
        {
          perror ("Allocating buf memory");
          exit (-1);
        }


      /*  Read the compressed data.  */

      if (!fread (buf, csize, 1, fp))
	{
	  fprintf (stderr, "Bad return in file %s, function %s at line %d.  This should never happen!", __FILE__, __FUNCTION__, __LINE__ - 2);
	  fflush (stderr);
          free (bit_box);
          free (buf);
	  return (-1);
	}


      /*  Uncompress the data.  */

      status = uncompress (bit_box, &bsize, buf, csize);
      if (status)
        {
          fprintf (stderr, "Error %d uncompressing record\n", status);
          exit (-1);
        }

      free (buf);


      /*  Unpack the internal header.  */

      pos = 0;
      start_val = bit_unpack (bit_box, pos, 16); pos += 16;
      bias = bit_unpack (bit_box, pos, 16); pos += 16;
      num_bits = bit_unpack (bit_box, pos, 4); pos += 4;
      null_val = NINT (pow (2.0L, (double) num_bits)) - 1;


      /*  Allocate the cell memory.  */

      if (box != NULL) free (box);

      box = (int16_t *) calloc (size * size, sizeof (int16_t));
      if (box == NULL)
        {
          perror ("Allocating box memory in read_srtm1_topo");
          exit (-1);
        }


      /*  Uncompress the data (delta coded snake dance).  */

      last_val = start_val;
      for (i = 0 ; i < size ; i++)
        {
          if (!(i % 2))
            {
              for (j = 0 ; j < size ; j++)
                {
                  temp = bit_unpack (bit_box, pos, num_bits); pos += num_bits;

                  if (temp < null_val)
                    {
                      box[i * size + j] = last_val + temp - bias;
                      last_val = box[i * size + j];
                    }
                  else
                    {
                      box[i * size + j] = -32768;
                    }
                }
            }
          else
            {
              for (j = size - 1 ; j >= 0 ; j--)
                {
                  temp = bit_unpack (bit_box, pos, num_bits); pos += num_bits;

                  if (temp < null_val)
                    {
                      box[i * size + j] = last_val + temp - bias;
                      last_val = box[i * size + j];
                    }
                  else
                    {
                      box[i * size + j] = -32768;
                    }
                }
            }
        }


      free (bit_box);

      prev_size = size;
    }


  /*  We didn't change cells so set the same size as last time.  */

  else
    {
      size = prev_size;
    }


  *array = box;


  return (size);
}




/***************************************************************************\
*                                                                           *
*   Module Name:        read_srtm1_topo                                     *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       September 2006                                      *
*                                                                           *
*   Purpose:            Reads the SRTM compressed topographic elevation     *
*                       file (*.cte) and returns the elevation value.  If   *
*                       the value is undefined at that point it will return *
*                       -32768.  For water it will return 0.                *
*                                                                           *
*   Arguments:          lat             -   latitude degrees, S negative    *
*                       lon             -   longitude degrees, W negative   *
*                                                                           *
*   Returns:            0 = water, -32768 undefined, elevation, 32767 on    *
*                       error                                               *
*                                                                           *
\***************************************************************************/


int16_t read_srtm1_topo (double lat, double lon)
{
  static int16_t     *array;
  static int32_t     prev_ilat = -999, prev_ilon = -999, size = 0;
  static double      inc = 0.0;
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


      size = read_srtm1_topo_one_degree (ilat, ilon, &array);


      if (size < 0) return (32767);


      if (size == 0) return (0);


      inc = 1.0L / (double) size;
    }


  if (size == 0) return (0);
  if (size == 2) return (-32768);


  /*  Get the cell index.  */

  lon += 180.0;
  ilon = (int32_t) lon;

  lat += 90.0;
  ilat = (int32_t) lat;

  lat_index = (int32_t) ((((double) ilat + 1.0L) - lat) / inc) + 1;
  lon_index = (int32_t) ((lon - (double) ilon) / inc);

  return (array[lat_index * size + lon_index]);
}



/***************************************************************************\
*                                                                           *
*   Module Name:        cleanup_srtm1_topo                                  *
*                                                                           *
*   Programmer(s):      Jan C. Depner                                       *
*                                                                           *
*   Date Written:       October 2006                                        *
*                                                                           *
*   Purpose:            Closes the open srtm1 topo file, frees memory, and  *
*                       sets the first flag back to NVTrue.                 *
*                                                                           *
*   Arguments:          None                                                *
*                                                                           *
*   Returns:            Nada                                                *
*                                                                           *
\***************************************************************************/


void cleanup_srtm1_topo ()
{
  if (fp != NULL) fclose (fp);
  if (box != NULL) free (box);
  if (map != NULL) free (map);
  box = NULL;
  map = NULL;
  first = NVTrue;
  prev_size = -1;
  no_file = NVFalse;
  fp = NULL;
}
