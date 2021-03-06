
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



#include "cvtime.h"

/********************************************************************************************/
/*!

 - Function:    cvtime

 - Purpose:     Convert from POSIX time to year, day of year, hour, minute,
                second.

 - Author:      Jan C. Depner (area.based.editor@gmail.com)

 - Arguments:
                - time_t tv_sec  =    POSIX seconds from epoch (Jan. 1, 1970)
                - long tv_nsec   =    POSIX nanoseconds of second
                - year           =    4 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute

 - Returns:
                - void

 - Caveats:     The localtime function will return the year as a 2 digit year (offset from 1900).

*********************************************************************************************/
 
void cvtime (time_t tv_sec, long tv_nsec, int32_t *year, int32_t *jday, int32_t *hour, int32_t *minute, float *second)
{
  static int32_t       tz_set = 0;
  struct tm            time_struct, *time_ptr = &time_struct;

  if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
      putenv ("TZ=GMT");
      tzset ();
  #else
      _putenv ("TZ=GMT");
      _tzset ();
  #endif
#else
      putenv("TZ=GMT");
      tzset();
#endif
      tz_set = 1;
    }
    
  time_ptr = localtime (&tv_sec);

  *year = (int16_t) time_ptr->tm_year;
  *jday = (int16_t) time_ptr->tm_yday + 1;
  *hour = (int16_t) time_ptr->tm_hour;
  *minute = (int16_t) time_ptr->tm_min;
  *second = (float) time_ptr->tm_sec + (float) ((double) tv_nsec / 1000000000.);
}



/********************************************************************************************/
/*!

 - Function:    inv_cvtime

 - Purpose:     Convert from year, day of year, hour, minute, second to microseconds from
                01-01-1970.

 - Author:      Jan C. Depner (area.based.editor@gmail.com)

 - Date:        07/20/11

 - Arguments:
                - year           =    4 digit year - 1900
                - jday           =    day of year
                - hour           =    hour of day
                - minute         =    minute of hour
                - second         =    seconds of minute
                - timestamp      =    Microseconds from epoch (Jan. 1, 1970)

 - Returns:
                - void

 - Caveats:     The mktime function takes a 2 digit year (offset from 1900).

*********************************************************************************************/
 
void inv_cvtime (int32_t year, int32_t jday, int32_t hour, int32_t min, float sec, time_t *tv_sec, long *tv_nsec)
{
    struct tm               tm;
    static int32_t          tz_set = 0;

    void                    jday2mday (int32_t, int32_t, int32_t *, int32_t *);

    
    tm.tm_year = year;

    jday2mday (year, jday, &tm.tm_mon, &tm.tm_mday);

    tm.tm_hour = hour;
    tm.tm_min = min;
    tm.tm_sec = (int32_t) sec;
    tm.tm_isdst = 0;

    if (!tz_set)
    {
#ifdef NVWIN3X
  #ifdef __MINGW64__
        putenv ("TZ=GMT");
        tzset ();
  #else
        _putenv ("TZ=GMT");
        _tzset ();
  #endif
#else
        putenv("TZ=GMT");
        tzset();
#endif
        tz_set = 1;
    }
    
    *tv_sec = mktime (&tm);
    *tv_nsec = (long)(fmod ((double) sec, 1.0) * 1.0e9);
}


/*  Months start at zero, days at 1 (go figure).  */

static int32_t               months[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/********************************************************************************************/
/*!

 - Function:    jday2mday

 - Purpose:     Convert from day of year to month and day.

 - Author:      Jan C. Depner (area.based.editor@gmail.com)

 - Date:        08/30/11

 - Arguments:
                - year           =    4 digit year or offset from 1900
                - jday           =    day of year
                - mon            =    month
                - mday           =    day of month

 - Returns:
                - void

 - Caveats:     The returned month value will start at 0 for January.

*********************************************************************************************/
 
void jday2mday (int32_t year, int32_t jday, int32_t *mon, int32_t *mday)
{
  int32_t l_year;

  l_year = year;

  if (year < 1899) l_year += 1900;


  /*  If the year is evenly divisible by 4 but not by 100, or it's evenly divisible by 400, this is a leap year.  */

  if ((!(l_year % 4) && (l_year % 100)) || !(l_year % 400))
    {
      months[1] = 29;
    }
  else
    {
      months[1] = 28;
    }

  *mday = jday;
  for (*mon = 0 ; *mon < 12 ; (*mon)++)
    {
      if (*mday - months[*mon] <= 0) break;

      *mday -= months[*mon];
    }
}


/********************************************************************************************/
/*!

 - Function:    mday2jday

 - Purpose:     Convert from month and day to day of year.

 - Author:      Jan C. Depner (area.based.editor@gmail.com)

 - Arguments:
                - year           =    4 digit year or offset from 1900
                - mon            =    month
                - mday           =    day of month
                - jday           =    day of year

 - Returns:
                - void

 - Caveats:     The month value must start at 0 for January.

*********************************************************************************************/

void mday2jday (int32_t year, int32_t mon, int32_t mday, int32_t *jday)
{
  int32_t i, l_year;

  l_year = year;

  if (year < 1899) l_year += 1900;


  /*  If the year is evenly divisible by 4 but not by 100, or it's evenly divisible by 400, this is a leap year.  */

  if ((!(l_year % 4) && (l_year % 100)) || !(l_year % 400))
    {
      months[1] = 29;
    }
  else
    {
      months[1] = 28;
    }


  *jday = mday;
  for (i = 0 ; i < mon - 1 ; i++) *jday += months[i];
}
