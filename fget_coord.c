
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



#include "fget_coord.h"


/***************************************************************************/
/*!
                                                                           *
  - Module Name:        fget_coord

  - Programmer:         Jan C. Depner

  - Date Written:       July 2000

  - Purpose:            Gets a pair of geographic coordinates from a file.

  - Inputs:
                        - lat_hemi            =   latitude hemisphere
                                                  indicator (S or N)
                        - lat_deg             =   latitude degrees
                        - lat_min             =   latitude minutes
                        - lat_sec             =   latitude seconds
                        - lon_hemi            =   longitude hemisphere
                                                  indicator (S or N)
                        - lon_deg             =   longitude degrees
                        - lon_min             =   longitude minutes
                        - lon_sec             =   longitude seconds

  - Outputs:            uint8_t               =   False if end of file

  - Restrictions:       Geographic positions are entered as a lat, lon pair
                        separated by a comma.  A lat or lon may be in any
                        of the following formats (degrees, minutes, and
                        seconds must be separated by a space or tab) :
                           - Degrees decimal                 : S 28.4532
                           - Degrees minutes decimal         : S 28 27.192
                           - Degrees minutes seconds decimal : S 28 27 11.52

                       Hemisphere may be indicated by letter or by sign.
                       West longitude and south latitude are negative :
                           - Ex. : -28 27 11.52 = S28 27 11.52 = s 28 27.192

****************************************************************************/

uint8_t fget_coord (FILE *fp, char *lat_hemi, int32_t *lat_deg, int32_t *lat_min,
                    float *lat_sec, char *lon_hemi, int32_t *lon_deg, int32_t *lon_min,
                    float *lon_sec)
{
  int32_t     i, j, sign;
  char        string[60], lat[30], lon[30];
  double      f1, f2, f3, fdeg, fmin, fsec;


  /*  Get the coordinates and make sure there is a comma between the  */
  /*  lat and lon.                                                    */

  do
    {
      if (fgets (string, sizeof (string), fp) == NULL) return (NVFalse);
    } while (strstr (string, ",") == NULL);


  /*  Break the input into a lat and lon string.                      */
    
  j = strcspn (string, ",");

  strncpy (lat, string, j);
  lat[j] = 0;

  strcpy (lon, (strstr (string, ",") + 1 ));


  /*  Handle the latitude (j = 0) and longitude (j = 1) portions of   */
  /*  the input string.                                               */
    
  for (j = 0 ; j < 2 ; j++)
    {
      if (j)
        {
          strcpy (string, lon);
        }
      else
        {
          strcpy (string, lat);
        }

      sign = 0;

      /*  Check for and clear sign or hemisphere indicators.          */
        
      for (i = 0 ; i < (int32_t) strlen (string) ; i++)
        {
          if (j)
            {
              if (string[i] == 'W' || string[i] == 'w' || string[i] == '-')
                {
                  string[i] = ' ';
                  sign = 1;
                }
            }
          else
            {
              if (string[i] == 'S' || string[i] == 's' || string[i] == '-')
                {
                  string[i] = ' ';
                  sign = 1;
                }
            }

          if (string[i] == 'n' || string[i] == 'N' || string[i] == 'e' ||
              string[i] == 'E' || string[i] == '+') string[i] = ' ';
        }
    
      fdeg = 0.0;
      fmin = 0.0;
      fsec = 0.0;
      f1 = 0.0;
      f2 = 0.0;
      f3 = 0.0;


      /*  Convert the string to degrees, minutes, and seconds.        */
        
      i = sscanf (string, "%lf %lf %lf", &f1, &f2, &f3);


      /*  Based on the number of values scanned, compute the total    */
      /*  degrees.                                                    */
        
      switch (i)
        {
        case 3:
          fsec = f3 / 3600.0;
#ifdef NVLinux
          __attribute__ ((fallthrough));
#endif
    
        case 2:
          fmin = f2 / 60.0;
#ifdef NVLinux
          __attribute__ ((fallthrough));
#endif
    
        case 1:
          fdeg = f1;
          break;
        }

      fdeg += fmin + fsec;


      /*  Get the sign and load the lat or lon values.                */
        
      if (j)
        {
          if (sign)
            {
              *lon_hemi = 'W';
            }
          else
            {
              *lon_hemi = 'E';
            }

          *lon_deg = (int32_t) fdeg;
          fmin = (fdeg - *lon_deg) * 60.0;
          *lon_min = (int32_t) (fmin + 0.00001);
          *lon_sec = (fmin - *lon_min) * 60.0 + 0.00001;
        }
      else
        {
          if (sign)
            {
              *lat_hemi = 'S';
            }
          else
            {
              *lat_hemi = 'N';
            }
            
          *lat_deg = (int32_t) fdeg;
          fmin = (fdeg - *lat_deg) * 60.0;
          *lat_min = (int32_t) (fmin + 0.00001);
          *lat_sec = (fmin - *lat_min) * 60.0 + 0.00001;
        }
    }

  return (NVTrue);
}
