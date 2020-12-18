
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



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <zlib.h>


#include "pfm_nvtypes.h"


#include "read_srtm_mask.h"
#include "bit_pack.h"


static int32_t           first = 1, prev_size = -1;
static uint8_t           *box = NULL;
static FILE              *fp;


/***************************************************************************/
/*!

  - Module Name:     check_srtm_mask

  - Programmer(s):   Jan C. Depner

  - Date Written:    October 2009

  - Purpose:         Checks to see if the SRTM land mask files are
                     available.

  - Arguments:       min_res         -   minimum resolution (1, 3, or 30)

  - Returns:         NULL if the file is available, else error string.

****************************************************************************/


char *check_srtm_mask (int32_t min_res)
{
  static char            dir[512], file[512], version[128], zversion[128], return_str[128];
  char                   varin[1024], info[1024];
  int32_t                i, j;


  if (min_res != 1 && min_res != 3 && min_res != 30)
    {
      sprintf (return_str, "Invalid resolution %d, use 1, 3, or 30\n", min_res);
      return (return_str);
    }

  if (getenv ("ABE_DATA") == NULL)
    {
      sprintf (return_str, "Unable to find ABE_DATA environment variable\n");
      return (return_str);
    }

  strcpy (dir, getenv ("ABE_DATA"));


  sprintf (file, "%s%1cland_mask%1csrtm_mask_%02d_second.clm", dir, (char) SEPARATOR, (char) SEPARATOR, min_res);


  if ((fp = fopen (file, "rb")) == NULL)
    {
      sprintf (return_str, "%s - %s\n", file, strerror (errno));
      return (return_str);
    }


  while (fgets (varin, sizeof (varin), fp))
    {
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
              sprintf (return_str, "\n\nZlib library version (%s) is not compatible with version used to build SRTM file (%s)\n\n",
                       zlibVersion (), zversion);
              fclose (fp);
              return (return_str);
            }
        }
    }


  fclose (fp);


  return (NULL);
}



/***************************************************************************/
/*!

  - Module Name:     read_srtm_mask_one_degree

  - Programmer(s):   Jan C. Depner

  - Date Written:    September 2006

  - Purpose:         Reads the SRTM compressed landmask file (*.clm) and
                     returns a one-degree single dimensioned array
                     containing 0 for water and 1 for land.  The
                     width/height of the array is returned.

  - Arguments:
                     - lat             =   degree of latitude, S negative
                     - lon             =   degree of longitude, W negative
                     - array           =   mask array
                     - min_res         =   minimum resolution (1, 3, or 30)

  - Returns:         0 if the cell is all water, 1 if it's all land,
                     2 if it's undefined, or the width/height of array
                     if it's mixed land and water (it's square).  The
                     array will only be populated for mixed land and
                     water.

  - Caveats:         The array is one dimensional so the user/caller
                     must index into the array accordingly.  The data is
                     stored in order from the northwest corner of the
                     cell, west to east, then north to south so the last
                     point in the returned array is the southeast
                     corner.  See pointlat and pointlon in the following
                     example code:

                     <pre>
                     include "read_srtm_mask.h"

                     uint8_t            *array;
                     int32_t            size;
                     double             inc, pointlat, pointlon;

                     size = read_srtm_mask_one_degree (lat, lon, &array, 1);
                     if (size > 2)
                       {
                         inc = 1.0L / size;
                         for (i = 0 ; i < size ; i++)
                           {
                             pointlat = (lat + 1.0L) - (i + 1) * inc;
                             for (j = 0 ; j < size ; j++)
                               {
                                 pointlon = lon + j * inc;

                                 /# DO SOMETHING WITH array[i * size + j] #/
                               }
                           }
                         cleanup_srtm_mask ();
                       }


                    You should call cleanup_srtm_mask after you are
                    finished using the database in order to close the
                    open file and free the associated memory.

                    </pre>

  - Description of the compressed land mask (.clm) file format (look Ma, no endians!)

  <pre>

    Header - 16384 bytes, ASCII

    [HEADER SIZE] = 16384
    [CREATION DATE] = 
    [VERSION] = 
    [ZLIB VERSION] =
    [END OF HEADER]


    One-degree map - 64800 * 4 bytes, binary, stored as characters.
    
        Records start at 90S,180W and proceed west to east then south to north (that is, the second record
        is for 90S,179W and the 361st record is for 89S,180W).
        Record contains 0 if all water, 1 if all land, 2 if undefined, or address if both land and water.


    Data - 1's and 0's (woo hoo)

        3 bits  - resolution, 0 = one second mask, 1 = 3 second mask, 2 = 30 second mask, others TBD
        29 bits - size of the zlib level 9 compressed block (SB)
        SB bytes - data

        The data is stored as a series of single bits for water (0) and land (1).  Each bit represents a 
        one second, three second, or thirty second cell in the block.  The block is a one-degree square.
        It will be 3600 X 3600, 1200 X 1200, 120 X 120, or 60 X 60 depending on the resolution.  It is
        ordered in the same fashion as the srtm3 data, that is, west to east starting in the northwest
        corner and moving southward.  The compression is compliments of the ZLIB compression library which
        can be found at http://www.zlib.net/.  Many thanks to Jean-loup Gailly, Mark Adler, and all others
        associated with that effort.

  </pre>

****************************************************************************/


int32_t read_srtm_mask_one_degree (int32_t lat, int32_t lon, uint8_t **array, int32_t min_res)
{
  static char            dir[512], file[512], version[128], created[128], zversion[128];
  static int32_t         header_size, prev_lat = -999, prev_lon = -999;
  char                   varin[1024], info[1024];
  uint8_t                add[4], *buf, *bit_box = NULL, head[4];
  int32_t                i, j, address, shift_lat, shift_lon, resolution, pos, wsize = 0, hsize = 0, status;
  uLong                  csize;
  uLongf                 bsize;


  /*  First time through, open the file and read the header.    */
    
  if (first)
    {
      if (min_res != 1 && min_res != 3 && min_res != 30)
        {
          fprintf (stderr, "Invalid resolution %d, use 1, 3, or 30\n", min_res);
          exit (-1);
        }

      if (getenv ("ABE_DATA") == NULL)
        {
          fprintf (stderr, "Unable to find ABE_DATA environment variable\n");
          fflush (stderr);
          return (-1);
        }

      strcpy (dir, getenv ("ABE_DATA"));


      sprintf (file, "%s%1cland_mask%1csrtm_mask_%02d_second.clm", dir, (char) SEPARATOR, (char) SEPARATOR, min_res);


      if ((fp = fopen (file, "rb")) == NULL)
        {
          perror (file);
          fflush (stderr);
          return (-1);
        }


      while (fgets (varin, sizeof (varin), fp))
        {
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
                  fclose (fp);
                  exit (-1);
                }
            }

          if (strstr (varin, "[CREATION DATE]")) strcpy (created, info);

          if (strstr (varin, "[HEADER SIZE]")) sscanf (info, "%d", &header_size);
        }

      first = 0;
    }


  /*  If we're working in the real 0-360 world (where 0 to 0 is 0 to 360) we want to turn longitudes greater
      than 180 into negatives before we switch to the bogus 0-360 world (where -180 to 180 is 0 to 360).  */

  if (lon >= 180) lon -= 360;


  /*  Shift into a 0 to 180 by 0 to 360 world.  */

  shift_lat = lat + 90;
  shift_lon = lon + 180;


  /*  Only read a cell if we changed cells since the last access.  */

  if (prev_lat != shift_lat || prev_lon != shift_lon)
    {
      /*  Read the address from the map.  */

      fseek (fp, header_size + (shift_lat * 360 + shift_lon) * 4, SEEK_SET);
      if (!fread (add, 4, 1, fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      address = (int32_t) bit_unpack (add, 0, 32);

      prev_lat = shift_lat;
      prev_lon = shift_lon;


      /*  If the address is 0, 1, or 2 we have all water, land, or undefined data.  */

      if (address < header_size)
        {
          if (address < 3)
            {
              prev_size = address;
              return (address);
            }
        }


      /*  Move to the address and read/unpack the header.  */

      fseek (fp, address, SEEK_SET);
      if (!fread (head, 4, 1, fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}

      pos = 0;
      resolution = (int32_t) bit_unpack (head, pos, 3); pos += 3;
      csize = (uLong) bit_unpack (head, pos, 29);


      switch (resolution)
        {
        case 0:
          wsize = 3600;
          break;

        case 1:
          wsize = 1200;
          break;

        case 2:
          wsize = 120;
          break;

        case 3:
          wsize = 1800;
          break;

        default:
          fprintf (stderr, "Unknown mask resolution - %d\n", resolution);
          exit (-1);
        }

      hsize = wsize;
      if (wsize == 1800) hsize = 3600;


      /*  We have to set an approximate size for unpacking (see the ZLIB documentation).  */

      bsize = (wsize * hsize) / 8 + 2000;


      /*  Allocate the uncompressed storage area.  */

      bit_box = (uint8_t *) calloc (bsize, sizeof (uint8_t));
      if (bit_box == NULL)
        {
          perror ("Allocating bit_box memory in read_srtm_mask");
          exit (-1);
        }


      /*  Allocate the compressed storage area.  */

      buf = (uint8_t *) calloc (csize, sizeof (uint8_t));
      if (buf == NULL)
        {
          perror ("Allocating buf memory");
          exit (-1);
        }


      /*  Read the compressed buffer.  */

      if (!fread (buf, csize, 1, fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  free (buf);
          free (bit_box);
	  return (-1);
	}

      status = uncompress (bit_box, &bsize, buf, csize);
      if (status)
        {
          fprintf (stderr, "Error %d uncompressing record\n", status);
          exit (-1);
        }

      free (buf);

      
      /*  Allocate the cell memory.  */

      if (box != NULL) free (box);

      box = (uint8_t *) calloc (wsize * hsize, sizeof (uint8_t));
      if (box == NULL)
        {
          perror ("Allocating box memory in read_srtm_mask");
          exit (-1);
        }


      /*  Unpack the cell.  */

      pos = 0;
      for (i = 0 ; i < hsize ; i++)
        {
          for (j = 0 ; j < wsize ; j++)
            {
              box[i * wsize + j] = (uint8_t) bit_unpack (bit_box, pos, 1);
              pos++;
            }
        }


      free (bit_box);

      prev_size = wsize;
    }


  /*  We didn't change cells so set the same size as last time.  */

  else
    {
      wsize = prev_size;
    }


  *array = box;


  return (wsize);
}




/***************************************************************************/
/*!

  - Module Name:        read_srtm_mask_min_res

  - Programmer(s):      Jan C. Depner

  - Date Written:       September 2006

  - Purpose:            Reads the SRTM compressed landmask file (*.clm) and
                        returns a value indicating whether the nearest
                        point in the mask is land, water, or undefined.
			This version allows you to set the resolution
			instead of defaulting to 1.

  - Arguments:
                        - lat             =   latitude degrees, S negative
                        - lon             =   longitude degrees, W negative
                        - min_res         =   minimum resolution (1, 3, or 30)

  - Returns:
                        - 0 = water
                        - 1 = land
                        - 2 = undefined (this shouldn't happen).

****************************************************************************/


int32_t read_srtm_mask_min_res (double lat, double lon, int32_t min_res)
{
  static uint8_t      *array;
  static int32_t      prev_ilat = -999, prev_ilon = -999, wsize = 0, hsize = 0;
  static double       winc = 0.0, hinc = 0.0;
  int32_t             ilat, ilon, lat_index, lon_index;

  /*
  int32_t i, j;
  FILE *fp;
  double nlat, nlon;
  */

  /*  If we're working in the real 0-360 world (where 0 to 0 is 0 to 360) we want to turn longitudes greater
      than 180 into negatives before we switch to the bogus 0-360 world (where -180 to 180 is 0 to 360).  */

  if (lon >= 180.0) lon -= 360.0;


  /*  In order to retrieve the correct record for negative longitudes we have to subtract 1.0 degree from the longitude.  */

  if (lon < 0.0) lon -= 1.0;


  /*  In order to retrieve the correct record for negative latitudes we have to subtract 1.0 degree from the latitude.  */

  if (lat < 0.0) lat -= 1.0;


  ilat = (int32_t) lat;
  ilon = (int32_t) lon;


  /*  No point in calling the function if we didn't change cells.  */

  if (ilat != prev_ilat || ilon != prev_ilon) 
    {
      wsize = read_srtm_mask_one_degree (ilat, ilon, &array, min_res);

      if (wsize < 3) return (wsize);

      hsize = wsize;
      if (wsize == 1800) hsize = 3600;

      winc = 1.0L / (double) wsize;
      hinc = 1.0L / (double) hsize;

      prev_ilat = ilat;
      prev_ilon = ilon;
    }


  /*  Get the cell index.  */

  if (lat < 0.0)
    {
      lat_index = (int32_t) ((-(lat - (double) ilat)) / hinc);
    }
  else
    {
      lat_index = (int32_t) ((((double) ilat + 1.0L) - lat) / hinc);
    }

  lon_index = (int32_t) ((lon - (double) ilon) / winc) - 1;
  /*
  if (array[lat_index * wsize + lon_index])
    {
      fp = fopen ("fred.yxz", "w");

      for (i = 0 ; i < hsize ; i++)
        {
          nlat = (double) (ilat) + (double) i * hinc;

          for (j = 0 ; j < wsize ; j++)
            {
              nlon = (double) (ilon) + (double) j * winc;

              fprintf (fp, "%.11f,%.11f,%f\n",nlat,nlon,(float) array[i * wsize + j]);
            }
        }

      fclose (fp);
      exit (-1);
    }
  */          
  return ((int32_t) array[lat_index * wsize + lon_index]);
}



/***************************************************************************/
/*!

  - Module Name:        read_srtm_mask

  - Programmer(s):      Jan C. Depner

  - Date Written:       September 2006

  - Purpose:            Reads the SRTM compressed landmask file (*.clm) and
                        returns a value indicating whether the nearest
                        point in the mask is land, water, or undefined.
			This is the original call using a min_res of 1.

  - Arguments:
                        - lat             =   latitude degrees, S negative
                        - lon             =   longitude degrees, W negative

  - Returns:
                        - 0 = water
                        - 1 = land
                        - 2 = undefined (this shouldn't happen).

****************************************************************************/


int32_t read_srtm_mask (double lat, double lon)
{
  return (read_srtm_mask_min_res (lat, lon, 1));
}




/***************************************************************************/
/*!

  - Module Name:        cleanup_srtm_mask

  - Programmer(s):      Jan C. Depner

  - Date Written:       September 2006

  - Purpose:            Closes the open srtm mask file, frees memory, and
                        sets the first flag back to 1.

  - Arguments:          None

  - Returns:            Nada

****************************************************************************/


void cleanup_srtm_mask ()
{
  fclose (fp);
  if (box != NULL) free (box);
  box = NULL;
  first = 1;
  prev_size = -1;
}
