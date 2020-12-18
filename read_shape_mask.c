

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


#include "pfm_nvtypes.h"
#include "nvdef.h"


#include "read_shape_mask.h"


/***************************************************************************/
/*!

  - Module Name:     check_shape_mask

  - Programmer(s):   Jan C. Depner (PFM Software)

  - Date Written:    November 2014

  - Purpose:         Checks to see if the supplied file name points to a valid
                     mask file that was created by the shape_mask program.

  - Arguments:       path            -   file name of the mask file (*.msk)

  - Returns:         NVTrue if the path points to a valid mask file, else NVFalse.

****************************************************************************/

uint8_t check_shape_mask (const char *file)
{
  FILE                   *fp;
  char                   version[128], varin[1024], info[1024];
  uint8_t                version_flag = NVFalse;


  if ((fp = fopen (file, "rb")) != NULL)
    {
      while (fgets (varin, sizeof (varin), fp))
        {
          if (strstr (varin, "[END OF HEADER]")) break;


          /*  Put everything to the right of the equals sign in 'info'.   */

          if (strchr (varin, '=') != NULL) strcpy (info, (strchr (varin, '=') + 1));

          if (strstr (varin, "[VERSION]")) strcpy (version, info);
        }

      if (strstr (version, "shape_mask")) version_flag = NVTrue;

      fclose (fp);
    }

  return (version_flag);
}



/***************************************************************************/
/*!

  - Module Name:     shape_mask_is_land

  - Programmer(s):   Jan C. Depner (PFM Software)

  - Date Written:    November 2014

  - Purpose:         Checks to see if the supplied position is over land or
                     water based on the mask files created by the shape_mask
                     program.  This function allocates a block of [HEIGHT]
                     by [WIDTH] bytes containing the mask as one byte integers
                     with 1 representing land and 0 representing water.

  - Arguments:       path            -   file name of the mask file (*.msk)
                     lat             -   latitude in degrees (south negative)
                     lon             -   longitude in degrees (west negative)

  - Returns:         1 for land, 0 for water, negative value on error (-9 for
                     point out of area).

  - Caveats:         To clear the allocated memory after using this
                     function, call it with lat argument larger than 180.  You
                     should always do this after you have finished using the
                     function since you may have quite a bit of memory allocated.

                     You can only have one mask file opened at once.

****************************************************************************/

int8_t shape_mask_is_land (const char *file, double lat, double lon)
{
  static int32_t         header_size, width, height, res_meters;
  static uint8_t         first = NVTrue;
  static int8_t          *block = NULL;
  static NV_F64_XYMBR    mbr;
  static double          x_res, y_res, distortion;
  FILE                   *fp;
  char                   version[128], created[128], varin[1024], info[1024];
  int32_t                i, point_index;


  /*  First time through, open the file, read the header, allocate and read the data block.    */
    
  if (first)
    {
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

          if (strstr (varin, "[CREATION DATE]")) strcpy (created, info);

          if (strstr (varin, "[HEADER SIZE]")) sscanf (info, "%d", &header_size);

          if (strstr (varin, "[START LAT]")) sscanf (info, "%lf", &mbr.min_y);
          if (strstr (varin, "[START LON]")) sscanf (info, "%lf", &mbr.min_x);

          if (strstr (varin, "[LAT RESOLUTION]")) sscanf (info, "%lf", &y_res);
          if (strstr (varin, "[LON RESOLUTION]")) sscanf (info, "%lf", &x_res);

          if (strstr (varin, "[HEIGHT]")) sscanf (info, "%d", &height);
          if (strstr (varin, "[WIDTH]")) sscanf (info, "%d", &width);

          if (strstr (varin, "[NOMINAL BIN SIZE IN METERS]")) sscanf (info, "%d", &res_meters);
          if (strstr (varin, "[NORTH SOUTH LON BIN SIZE DIFFERENCE IN METERS]")) sscanf (info, "%lf", &distortion);
        }


      /*  Compute the rest of the MBR.  */

      mbr.max_y = mbr.min_y + (double) height * y_res;
      mbr.max_x = mbr.min_x + (double) width * x_res;


      if ((block = (int8_t *) malloc (width * height * sizeof (int8_t))) == NULL)
        {
          perror ("Allocating block memory in read_shape_mask.c");
          return (-2);
        }


      /*  Skip the remainder of the header.  */

      if (fseek (fp, header_size, SEEK_SET))
        {
          perror ("Seeking past header in read_shape_mask.c");
          return (-3);
        }


      /*  Read the data.  */

      for (i = 0 ; i < height ; i++)
        {
          if (!fread (&block[i * width], width, 1, fp))
            {
              perror ("Reading block row in read_shape_mask.c");
              return (-4);
            }
        }


      /*  Close the file.  */

      fclose (fp);


      first = NVFalse;
    }


  /*  If we're done, free the memory and do an Elvis (Return To Sender).  */

  if (lat > 180.0)
    {
      if (block != NULL)
        {
          free (block);
          block = NULL;
        }

      first = NVTrue;

      return (0);
    }


  /*  Check for a point outside of the area.  */

  if (lat < mbr.min_y || lat > mbr.max_y || lon < mbr.min_x || lon > mbr.max_x) return (-9);


  /*  Find the closest point.  */

  point_index = NINT ((lat - mbr.min_y) / y_res) * width + NINT ((lon - mbr.min_x) / x_res);


  /*  Now, just return the byte that we found.  */

  return (block[point_index]);
}
