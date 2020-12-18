
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



#include "fixpos.h"

/***************************************************************************/
/*!

  - Module :        fixpos

  - Programmer :    Jan C. Depner

  - Date :          04/25/94

  - Purpose :       This routine converts decimal degrees to degrees, minutes,
                    seconds, and hemisphere in the form specified by 'form'.

  - Arguments:
                    - degs        =   decimal degrees
                    - type        =   POS_LAT or POS_LON
                    - form        =   POS_HDMS, POS_HDM, POS_HD, POS_DMS, POS_DM, POS_D
                    - deg         =   degrees
                    - min         =   minutes
                    - sec         =   seconds
                    - hem         =   hemisphere
                    
  - Return Value:
                    - string containing the position in 'form' format

\***************************************************************************/

char *fixpos (double degs, double *deg, double *min, double *sec, char *hem, int32_t type, int32_t form)
{
  double               abs_degs;
  static char          string[132];


  /*  Compute the degrees, minutes, seconds, and hemisphere.  */
    
  abs_degs = fabs (degs) * 3600.0;

  *sec = fmod (abs_degs, 60.0);
  abs_degs -= *sec;
  abs_degs /= 60.0;
  *min = fmod (abs_degs, 60.0);
  abs_degs -= *min;
  abs_degs /= 60.0;
  *deg = abs_degs;

  *deg = (int32_t) (*deg + 0.00001);
  *min = (int32_t) (*min + 0.00001);
  *sec += 0.000000001;

  if (*sec >= 60.0)
    {
      *min += 1.0;
      *sec -= 60.0;
    }

  if (*min >= 60.0)
    {
      *deg += 1.0;
      *min -= 60.0;
    }

  if (type)
    {
      if (degs < 0.0)
        {
          *hem = 'W';
        }
      else
        {
          *hem = 'E';
        }
    }
  else
    {
      if (degs < 0.0)
        {
          *hem = 'S';
        }
      else
        {
          *hem = 'N';
        }
    }


  /*  Build the output string and reset the min, sec, and hem if needed.  */

  switch (form)
    {
    case POS_HDMS:
      if (type)
        {
          sprintf (string, "%1c %03d %02d %05.2f", *hem, (int32_t) *deg, (int32_t) *min, *sec);
        }
      else
        {
          sprintf (string, "%1c %02d %02d %05.2f", *hem, (int32_t) *deg, (int32_t) *min, *sec);
        }
      break;

    case POS_HDM:
      *min += *sec / 60.0;
      *sec = 0.0;

      if (type)
        {
          sprintf (string, "%1c %03d %08.5f", *hem, (int32_t) *deg, *min);
        }
      else
        {
          sprintf (string, "%1c %02d %08.5f", *hem, (int32_t) *deg, *min);
        }
      break;

    case POS_HD:
      *deg += *min / 60.0 + *sec / 3600.0;
      *min = 0.0;
      *sec = 0.0;

      if (type)
        {
          sprintf (string, "%1c %012.8f", *hem, *deg);
        }
      else
        {
          sprintf (string, "%1c %011.8f", *hem, *deg);
        }
      break;

    case POS_DMS:
      if (degs < 0.0)
        {
          *hem = '-';
        }
      else
        {
          *hem = ' ';
        }

      if (type)
        {
          sprintf (string, "%1c%03d %02d %05.2f", *hem, (int32_t) *deg, (int32_t) *min, *sec);
        }
      else
        {
          sprintf (string, "%1c%02d %02d %05.2f", *hem, (int32_t) *deg, (int32_t) *min, *sec);
        }

      if (degs < 0.0)
        {
          if ((int32_t) *deg)
            {
              *deg = -*deg;
            }
          else if ((int32_t) *min)
            {
              *min = -*min;
            }
          else
            {
              *sec = -*sec;
            }
        }
      break;

    case POS_DM:
      if (degs < 0.0)
        {
          *hem = '-';
        }
      else
        {
          *hem = ' ';
        }
      *min += *sec / 60.0;
      *sec = 0.0;

      if (type)
        {
          sprintf (string, "%1c%03d %08.5f", *hem, (int32_t) *deg, *min + *sec / 60.0);
        }
      else
        {
          sprintf (string, "%1c%02d %08.5f", *hem, (int32_t) *deg, *min + *sec / 60.0);
        }

      if (degs < 0.0)
        {
          if ((int32_t) *deg)
            {
              *deg = -*deg;
            }
          else
            {
              *min = -*min;
            }
        }
      break;

    case POS_D:
      if (degs < 0.0)
        {
          *hem = '-';
        }
      else
        {
          *hem = ' ';
        }

      if (type)
        {
          sprintf (string, "%1c%012.8f", *hem, fabs (degs));
        }
      else
        {
          sprintf (string, "%1c%011.8f", *hem, fabs (degs));
        }

      *deg = degs;
      *min = 0.0;
      *sec = 0.0;
      break;
    }

  return (string);
}





/***************************************************************************/
/*!

  - Module :        posfix

  - Programmer :    Jan C. Depner

  - Date :          04/25/94

  - Purpose :       This routine converts a string containing the latitude or
                    longitude in any of the following formats to decimal
                    degrees.

                        Hemisphere Degrees Minutes Seconds.decimal
                        Hemisphere Degrees Minutes.decimal
                        Hemisphere Degrees.decimal
                        SignDegrees Minutes Seconds.decimal
                        SignDegrees Minutes.decimal
                        SignDegrees.decimal

  - Arguments:
                    - *string     =   character string returned
                    - *degs       =   degrees decimal
                    - type        =   POS_LAT or POS_LON
                    
  - Return Value:   void

\***************************************************************************/

void posfix (char *string, double *degs, int32_t type)
{
  int32_t          i, sign;
  double           fdeg, fmin, fsec, f1, f2, f3;


  /*  Check for and clear sign or hemisphere indicators.          */

  sign = 0;
  for (i = 0 ; i < (int32_t) strlen (string) ; i++)
    {
      if (type)
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

      if (string[i] == 'n' || string[i] == 'N' || string[i] == 'e' || string[i] == 'E' || string[i] == '+') string[i] = ' ';
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
  *degs = fdeg;
  if (sign) *degs = - *degs;
}

