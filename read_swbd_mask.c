

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

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <zlib.h>
#include <shapefil.h>


#include "pfm_nvtypes.h"
#include "nvdef.h"


#include "read_swbd_mask.h"
#include "inside_polygon.h"
#include "bit_pack.h"


/***************************************************************************/
/*!

  - Module Name:     check_swbd_mask

  - Programmer(s):   Jan C. Depner (PFM Software)

  - Date Written:    October 2013

  - Purpose:         Checks to see if the SWBD land mask files are
                     available.

  - Arguments:       res             -   resolution (1, 3, 10, 30, or 60)

  - Returns:         NULL if the file is available, else error string.

****************************************************************************/

char *check_swbd_mask (int32_t res)
{
  static char            return_str[128];
  char                   dir[512], file[512], version[128], zversion[128];
  FILE                   *fp;
  char                   varin[1024], info[1024];
  int32_t                i, j;


  if (res != 1 && res != 3 && res != 10 && res != 30 && res != 60)
    {
      sprintf (return_str, "Invalid resolution %d, use 1, 3, 10, 30, or 60\n", res);
      return (return_str);
    }


  /*  Check for ABE_DATA environment variable.  */

  if (getenv ("ABE_DATA") == NULL)
    {
      sprintf (return_str, "Unable to find ABE_DATA environment variable\n");
      return (return_str);
    }


  sprintf (dir, "%s", getenv ("ABE_DATA"));

  sprintf (file, "%s%1cland_mask%1cswbd_mask_%02d_second.clm", dir, (char) SEPARATOR, (char) SEPARATOR, res);

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
              sprintf (return_str, "\n\nZlib library version (%s) is not compatible with version used to build SWBD file (%s)\n\n",
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

  - Module Name:     read_swbd_mask_one_degree

  - Programmer(s):   Jan C. Depner

  - Date Written:    September 2006

  - Purpose:         Reads the SWBD compressed landmask file (*.clm) and
                     returns a one-degree single dimensioned array
                     containing 0 for water and 1 for land.  The
                     width/height of the array is returned.

  - Arguments:
                     - latdeg          =   degree of latitude (south negative)
                                           of the southern edge of the one-degree
                                           cell
                     - londeg          =   degree of longitude (west negative)
                                           of the western edge of the one-degree
                                           cell (in other words, if you want the
                                           data for the cell containing 37.5N,75.5W
                                           you need to ask for 37,76)
                     - array           =   2D mask array, uint8_t,
                                           allocated by caller, size :
                                           - 1 second res = 3600 X 3600
                                           - 3 second res = 1200 X 1200
                                           - 10 second res = 360 X 360
                                           - 30 second res = 120 X 120
                                           - 60 second res = 60 X 60
                     - res             =   resolution (1, 3, 10, 30, or 60)

  - Returns:         0 if the cell is undefined, 1 if it's all land,
                     2 if it's all water, or the width/height of array
                     if it's mixed land and water (it's square).  The
                     array will only be populated for mixed land and
                     water.  -1 is returned on error.

  - Caveats:         You probably don't want to use this function unless you really
                     know what you're doing.  The swbd_is_land function handles four
                     adjacent cells without thrashing when you cross the one-degree
                     boundaries.  If you only want a one degree cell you can use the
                     following example.

                     The array is two dimensional so the user/caller
                     must allocate the array and index into it accordingly.
                     The data is stored in order from the southwest corner
                     of the cell, west to east, then south to north so the
                     last point in the returned array is the northeast
                     corner.  The following is an example Qt/C++ program that
                     creates a TIF file for the cell at LAT/LON at resolution
                     RES:

                     <pre>

                     #include <stdio.h>
                     #include <stdlib.h>
                     #include <errno.h>
                     #include <math.h>
                     #include <string.h>

                     #include "nvutility.hpp"
                     #include "nvutility.h"

                     #include "version.h"


                     void usage (char *string)
                       {
                         fprintf (stderr, "Usage: %s LAT LON RES\n\n", string);
                         fprintf (stderr, "Where:\n");
                         fprintf (stderr, "\tLAT = latitude degrees (south negative)\n");
                         fprintf (stderr, "\tLON = longitude degrees (west negative)\n");
                         fprintf (stderr, "\tRES = desired resolution of output tiff in seconds (1, 3, 10, 30, or 60)\n\n");
                         exit (-1);
                       }



                     int32_t main (int32_t argc, char **argv)
                       {
                         int32_t           lat, lon, resolution = 0;
                         char              fname[512];



                         printf ("\n\n%s\n\n", VERSION);


                         if (argc < 4) usage (argv[0]);


                         sscanf (argv[1], "%d", &lat);
                         sscanf (argv[2], "%d", &lon);
                         sscanf (argv[3], "%d", &resolution);


                         if (resolution != 1 && resolution != 3 && resolution != 10 && resolution != 30 && resolution != 60) usage (argv[0]);

                         int32_t dim = 3600 / resolution;


                         //  If you'd rather use "new" and "delete" instead of "calloc" and "free", be my guest.
                         //  I'm old school because I'm old ;-)

                         uint8_t **block = (uint8_t **) calloc (dim, sizeof (uint8_t *));

                         if (block == NULL)
                           {
                             perror ("Allocating block memory");
                             exit (-1);
                           }

                         for (int32_t i = 0 ; i < dim ; i++)
                           {
                             block[i] = (uint8_t *) calloc (dim, sizeof (uint8_t));

                             if (block[i] == NULL)
                               {
                                 perror ("Allocating block[i] memory");
                                 exit (-1);
                               }
                           }


                         int32_t res = read_swbd_mask_one_degree (lat, lon, block, resolution);


                         if (res > 2)
                           {
                             sprintf (fname, "%03d_%03d_%02d.tif", lat + 90, lon + 180, resolution);


                             //  Make a tif.

                             QImage *raster = new QImage (res, res, QImage::Format_ARGB32);


                             int32_t flip_count = dim - 1;


                             //  Latitude loop.

                             for (int32_t i = 0 ; i < dim ; i++)
                               {
                                 //  Longitude loop.

                                 for (int32_t j = 0 ; j < dim ; j++)
                                   {
                                     uint32_t color;

                                     if (block[i][j])
                                       {
                                         color = 0xffff0000;
                                       }
                                     else
                                       {
                                         color = 0xff0000ff;
                                       }


                                     //  Flip the Y axis for TIF.

                                     raster->setPixel (j, flip_count - i, color);
                                   }
                               }


                             //  Write the output file.

                             raster->save (fname);
                           }


                         for (int32_t i = 0 ; i < dim ; i++)
                           {
                             if (block[i]) free (block[i]);
                           }

                         if (block) free (block);


                       return (0);
                     }

                    </pre>


   - Description of the compressed land mask (.clm) file format (look Ma, no endians!)

     <pre>

        Header - 16384 bytes, ASCII

        [HEADER SIZE] = 16384
        [CREATION DATE] = 
        [VERSION] = 
        [ZLIB VERSION] =
        [RESOLUTION] = 1, 3, 10, 30, or 60
        [END OF HEADER]


        One-degree map - 64800 * 7 bytes, binary, stored as unsigned characters starting at [HEADER SIZE] bytes.

            Single record (7 bytes) :

                32 bits - 0 = undefined, 1 = all land, 2 = all water, otherwise the address
                          of the compressed block
                24 bits - 0 or the size of the compressed block (*CBS) 

            Records start at 90S,180W and proceed west to east then south to north (that is,
            the second record is for 90S,179W and the 361st record is for 89S,180W).


        Data - 1's and 0's (woo hoo)

            *CBS bytes - data

            The data is stored as a series of single bits for water (0) and land (1).  Each
            bit represents a one second, three second, or ten second cell in the block.  The
            block is a one-degree square.  It will be 3600 X 3600, 1200 X 1200, 360 X 360,
            120 X 120, or 60 X 60 depending on the resolution.  It is ordered west to east
            starting in the southwest corner and moving northward.  The compression is
            compliments of the ZLIB compression library which can be found at
            http://www.zlib.net/.  Many thanks to Jean-loup Gailly, Mark Adler, and all
            others associated with that effort.

     </pre>

****************************************************************************/


int32_t read_swbd_mask_one_degree (int32_t latdeg, int32_t londeg, uint8_t **array, int32_t res)
{
  static int32_t         header_size, prev_latdeg = -999, prev_londeg = -999, prev_res = -999;
  static uint8_t         first = NVTrue;
  static FILE            *fp;
  char                   dir[512], file[512], version[128], created[128], zversion[128], varin[1024], info[1024];
  uint8_t                add[7], *buf, *bit_box = NULL;
  int32_t                i, j, address, shift_latdeg, shift_londeg, pos, dim = 0, status;
  uLong                  csize;
  uLongf                 bsize;


  /*  If the caller changed resolutions we want to close the old file and open a new one.  */

  if (res != prev_res && prev_res >= 0)
    {
      fclose (fp);
      first = NVTrue;
    }


  /*  First time through, open the file and read the header.    */
    
  if (first)
    {
      if (res != 1 && res != 3 && res != 10 && res != 30 && res != 60)
        {
          fprintf (stderr, "Invalid resolution %d, use 1, 3, 10, 30, or 60\n", res);
          fflush (stderr);
          return (0);
        }


      /*  Check for ABE_DATA environment variable.  */

      if (getenv ("ABE_DATA") == NULL)
        {
          fprintf (stderr, "\n\nEnvironment variable ABE_DATA is not set\n\n");
          fflush (stderr);
          return (0);
        }


      sprintf (dir, "%s", getenv ("ABE_DATA"));

      sprintf (file, "%s%1cland_mask%1cswbd_mask_%02d_second.clm", dir, (char) SEPARATOR, (char) SEPARATOR, res);


      if ((fp = fopen (file, "rb")) == NULL)
        {
          perror (file);
          fflush (stderr);
          return (0);
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
                  fprintf (stderr, "\n\nZlib library version (%s) is not compatible with version used to build SWBD file (%s)\n\n",
                           zlibVersion (), zversion);
                  fclose (fp);
                  exit (-1);
                }
            }

          if (strstr (varin, "[CREATION DATE]")) strcpy (created, info);

          if (strstr (varin, "[HEADER SIZE]")) sscanf (info, "%d", &header_size);
        }

      first = NVFalse;
    }


  prev_res = res;


  /*  If we're working in the real 0-360 world (where 0 to 0 is 0 to 360) we want to turn longitudes greater
      than 180 into negatives before we switch to the bogus 0-360 world (where -180 to 180 is 0 to 360).  */

  if (londeg >= 180) londeg -= 360;


  /*  Shift into a 0 to 180 by 0 to 360 world.  */

  shift_latdeg = latdeg + 90;
  shift_londeg = londeg + 180;


  /*  Set the dimensions of the box.  */

  switch (res)
    {
    case 1:
      dim = 3600;
      break;

    case 3:
      dim = 1200;
      break;

    case 10:
      dim = 360;
      break;

    case 30:
      dim = 120;
      break;

    case 60:
      dim = 60;
      break;
    }


  /*  Only read and unpack a cell if we changed cells since the last access.  */

  if (prev_latdeg != shift_latdeg || prev_londeg != shift_londeg)
    {
      /*  Read the address from the map.  */

      fseek (fp, header_size + (shift_latdeg * 360 + shift_londeg) * 7, SEEK_SET);
      if (!fread (add, 7, 1, fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (0);
	}
      address = (int32_t) bit_unpack (add, 0, 32);
      csize = (int32_t) bit_unpack (add, 32, 24);

      prev_latdeg = shift_latdeg;
      prev_londeg = shift_londeg;


      /*  If the address is 0, 1, or 2 we have undefined data, all land, or all water.  */

      if (address < 3) return (address);


      /*  Move to the address and read/unpack the block.  */

      fseek (fp, address, SEEK_SET);

      pos = 0;


      /*  We have to set an approximate size for unpacking (see the ZLIB documentation).  */

      bsize = (dim * dim) / 8 + 2000;


      /*  Allocate the uncompressed storage area.  */

      bit_box = (uint8_t *) calloc (bsize, sizeof (uint8_t));
      if (bit_box == NULL)
        {
          perror ("Allocating bit_box memory in read_swbd_mask");
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
          free (bit_box);
          free (buf);
	  return (0);
	}

      status = uncompress (bit_box, &bsize, buf, csize);
      if (status)
        {
          fprintf (stderr, "Error %d uncompressing record\n", status);
          exit (-1);
        }

      free (buf);

      
      /*  Unpack the cell.  */

      pos = 0;
      for (i = 0 ; i < dim ; i++)
        {
          for (j = 0 ; j < dim ; j++)
            {
              array[i][j] = (uint8_t) bit_unpack (bit_box, pos, 1);
              pos++;
            }
        }


      free (bit_box);
    }


  return (dim);
}



/***************************************************************************/
/*!

  - Module Name:     swbd_is_land

  - Programmer(s):   Jan C. Depner (PFM Software)

  - Date Written:    December 2013

  - Purpose:         Checks to see if the supplied position is over land or
                     water based on the mask files created from SWBD shape
                     files.  This function uses up to four one-degree cells
                     so that we don't thrash when moving between one-degree
                     cells.  If we move to a new cell and one of the current
                     cells is not empty we will use the least recently used
                     cell based on the cell change count stored in the "z"
                     member of the corner array.

  - Arguments:       lat             -   latitude in degrees (south negative)
                     lon             -   longitude in degrees (west negative)
                     res             -   resolution in seconds (1, 3, 10, 30, 60)

  - Returns:         1 for land, 0 for water, negative value on error.

  - Caveats:         To clear the allocated memory after using this
                     function, call it with lat argument larger than 180.  You
                     should always do this after you have finished using the
                     function since you may have up to 52MB of memory allocated.

****************************************************************************/

int32_t swbd_is_land (double lat, double lon, int32_t res)
{
  static int32_t       type = 0, dim = 0, prev_res = 0, cell = -1, cell_count = -1, prev_latdeg = -999, prev_londeg = -999;
  static NV_I32_COORD3 corner[4] = {{-999, -999, -1}, {-999, -999, -1}, {-999, -999, -1}, {-999, -999, -1}};
  static uint8_t       **block[4] = {NULL, NULL, NULL, NULL};
  int32_t              latdeg, londeg, lt, ln, i, j, min_count;


  if (res != 1 && res != 3 && res != 10 && res != 30 && res != 60)
    {
      fprintf (stderr, "\n\nInvalid resolution - %d.  Use 1, 3, 10, 30, or 60!\n\n", res);
      fflush (stderr);
      return (-1);
    }


  /*  If the resolution has changed we need to clear everything.  */

  if (res != prev_res || lat > 180.0)
    {
      for (i = 0 ; i < 4 ; i++)
        {
          if (block[i])
            {
              for (j = 0 ; j < dim ; j++)
                {
                  if (block[i][j] != NULL) free (block[i][j]);
                }

              free (block[i]);

              block[i] = NULL;
            }

          corner[i].x = -999;
          corner[i].y = -999;
          corner[i].z = -1;
        }


      prev_latdeg = -999;
      prev_londeg = -999;
      cell_count = -1;


      /*  If we're closing out, as opposed to changing resolutions, we want to Elvis (Return To Sender).  */

      if (lat > 180.0) return (0);
    }

  prev_res = res;


  if (lat < 0.0)
    {
      latdeg = (int32_t) lat - 1;
    }
  else
    {
      latdeg = (int32_t) lat;
    }

  if (lon < 0.0)
    {
      londeg = (int32_t) lon - 1;
    }
  else
    {
      londeg = (int32_t) lon;
    }


  /*  Check to see if we changed cells.  */

  if (latdeg != prev_latdeg || londeg != prev_londeg)
    {
      /*  Increment the cell change counter every time we change cells.  We use this to determine which cell is the 
          least recently used (LRU) cell.  */

      cell_count++;


      /*  Check to see if we're in one of the already allocated and filled cells (there may be up to 4).  */

      cell = -1;
      for (i = 0 ; i < 4 ; i++)
        {
          if (latdeg == corner[i].y && londeg == corner[i].x)
            {
              cell = i;
              break;
            }
        }


      /*  If we didn't find a match...  */

      if (cell < 0)
        {
          /*  Find the first empty cell (z less than 0) if there is one.  */

          for (i = 0 ; i < 4 ; i++)
            {
              if (corner[i].z < 0)
                {
                  cell = i;
                  break;
                }
            }


          /*  No empty cells.  Find the LRU cell.  In this case we don't have to allocate memory since it was already allocated
              for the previously used cell (and resolution hasn't changed).  */

          if (cell < 0)
            {
              min_count = INT32_MAX;

              for (i = 0 ; i < 4 ; i++)
                {
                  if (corner[i].z < min_count)
                    {
                      cell = i;
                    }
                }
            }


          /*  Here we are.  The lat and lon weren't in a previously allocated and filled cell, we're not reusing a previously 
              allocated and filled cell, or we found an empty cell to fill, we have to allocate the cell.  */

          else
            {
              dim = 3600 / res;


              block[cell] = (uint8_t **) calloc (dim, sizeof (uint8_t *));

              if (block[cell] == NULL)
                {
                  perror ("Allocating block[cell] memory");
                  exit (-1);
                }

              for (i = 0 ; i < dim ; i++)
                {
                  block[cell][i] = (uint8_t *) calloc (dim, sizeof (uint8_t));

                  if (block[cell][i] == NULL)
                    {
                      perror ("Allocating block[cell][i] memory");
                      exit (-1);
                    }
                }
            }


          /*  Finally, we fill the cell.  */

          type = read_swbd_mask_one_degree (latdeg, londeg, block[cell], res);


          if (!type) return (-3);
        }


      /*  Save the previous latitude, longitude, and cell change count.  */

      prev_latdeg = corner[cell].y = latdeg;
      prev_londeg = corner[cell].x = londeg;
      corner[cell].z = cell_count;
    }


  /*  If type is less than 3, the one-degree cell is either all water or all land.  */

  if (type == 1) return (1);

  if (type == 2) return (0);

  lt = NINT (((lat - (double) latdeg) * 3600.0) / (double) res);
  ln = NINT (((lon - (double) londeg) * 3600.0) / (double) res);


  /*  For points closer to the next cell than the current one we're going to use the nearest in-cell point instead.  */

  if (lt == dim) lt = dim - 1;
  if (ln == dim) ln = dim - 1;


  return (block[cell][lt][ln]);
}
