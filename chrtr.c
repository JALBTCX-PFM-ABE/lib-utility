
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



#include "chrtr.h"


typedef struct
{
  FILE          *fp;
  uint8_t       swap;
  CHRTR_HEADER  header;
  float         grid_degrees;
} INTERNAL_CHRTR_HEADER;

static INTERNAL_CHRTR_HEADER chrtrh[MAX_CHRTR_FILES];
static uint8_t first = NVTrue;


/***************************************************************************/
/*!

  - Module Name:        bit_set

  - Programmer(s):      Dominic Avery, Jan C. Depner

  - Date Written:       April 1992

  - Purpose:            Set the specified bit of a given floating-point
                        value to one or zero.

  - Arguments:
                        - *value  =   floating point value
                        - bit     =   bit to be set
                        - binary  =   0 or 1

  - Return Value:       None

  - Glossary:           data    -   union of an int and float

  - Method:             The floating point value input is stored in a
                        floating point variable that is in a union with an
                        integer.  The bit in the integer is set or unset
                        and then the float from the union is placed in the
                        input float variable.

****************************************************************************/
                                                                            
void bit_set (float *value, int32_t bit, int32_t binary)
{
    union
    {
        uint32_t             intvalue;
        float                fltvalue;
    } data;

    data.fltvalue = *value;
    if (binary)
    {
        data.intvalue |= binary << bit;
    }
    else
    {
        data.intvalue &= ~(1 << bit);
    }
    *value = data.fltvalue;

    return;
}


/***************************************************************************/
/*!

  - Module Name:        bit_test

  - Programmer(s):      Dominic Avery, Jan C. Depner

  - Date Written:       April 1992

  - Purpose:            Test the specified bit of a given floating-point
                        value.

  - Arguments:
                        - value   =   floating point value
                        - bit     =   bit to be tested

  - Return Value:       int32_t   =   1 if bit is set, otherwise 0

  - Glossary:           data    =   union of an int and float

  - Method:             The floating point value input is stored in a
                       floating point variable that is in a union with an
                       integer.  The bit in the integer is tested and the
                       result is returned.

****************************************************************************/
                                                                            
int32_t bit_test (float value, int32_t bit)
{
    union
    {
        uint32_t             intvalue;
        float                fltvalue;
    } data;

    data.fltvalue = value;

    return ((data.intvalue & (1 << bit)));
}


/********************************************************************/
/*!

  - Function:    open_chrtr

  - Purpose:     Open a chrtr file (read only).
                 - The following is the CHRTR file format as of December 1994 :
                     - First record :
                         - float = Western longitude in degrees decimal
                         - float = Eastern longitude in degrees decimal
                         - float = Southern latitude in degrees decimal
                         - float = Northern latitude in degrees decimal
                         - float = Grid spacing in minutes decimal
                         - int   = Width in grid units
                         - int   = Height in grid units
                         - int   = Endian indicator (0x00010203 if the file was created
                                   on a machine that has the same byte ordering as the
                                   machine it is being read on, otherwise it is 0x03020100.
                                   If this is 0x03020100 then all of the 4 byte values must
                                   be byte swapped after reading and before writing.
                         - float = Minimum value in the file
                         - float = Maximum value in the file
                 - These values are followed by (Width - 10) 4 byte pads.  This was
                   done to accomodate FORTRAN which doesn't like doing direct access
                   to variable record length files.
                     - Subsequent records :
                         - There are 'Height' records of 'Width' length consisting of 4
                           byte floats.  These contain the data values for each grid cell.
                           The low order bit of each value, if set, indicates that the cell
                           contained at least one input value when gridded.  The ordering
                           of these values is West to East starting with the southernmost
                           grid row and continuing to the northernmost grid row.


  - Author:      Jan C. Depner (area.based.editor@gmail.com)

  - Date:        03/21/08

  - Arguments:
                 - path           =    The chrtr file path
                 - header         =    CHRTR_HEADER structure to be populated

  Returns:       int32_t        =    The file handle or -1 on error

********************************************************************/

int32_t open_chrtr (const char *path, CHRTR_HEADER *header)
{
  int32_t          i, hnd;


  /*  The first time through we want to initialize the chrtr header array.  */

  if (first)
    {
      for (i = 0 ; i < MAX_CHRTR_FILES ; i++) 
        {
          memset (&chrtrh[i], 0, sizeof (INTERNAL_CHRTR_HEADER));
          chrtrh[i].fp = NULL;
        }
      first = NVFalse;
    }


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = MAX_CHRTR_FILES;
  for (i = 0 ; i < MAX_CHRTR_FILES ; i++)
    {
      if (chrtrh[i].fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == MAX_CHRTR_FILES)
    {
      fprintf (stderr, "\n\nToo many open chrtr files!\nTerminating!\n\n");
      exit (-1);
    }


  /*  Open the file and read the header.  */

  if ((chrtrh[hnd].fp = fopen (path, "rb+")) != NULL)
    {

      /*  Read the ENDIAN indicator and set the swap flag accordingly.    */

      fseek (chrtrh[hnd].fp, 28, SEEK_SET);
      if (!fread (&i, sizeof (int32_t), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}

      if (i == 0x03020100)
        {
	  chrtrh[hnd].swap = NVTrue;
        }
      else
        {
	  chrtrh[hnd].swap = NVFalse;
        }
      if (!fread (&chrtrh[hnd].header.min_z, sizeof (float), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      if (!fread (&chrtrh[hnd].header.max_z, sizeof (float), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}


      fseek (chrtrh[hnd].fp, 0, SEEK_SET);
      if (!fread (&chrtrh[hnd].header.wlon, sizeof (float), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      if (!fread (&chrtrh[hnd].header.elon, sizeof (float), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      if (!fread (&chrtrh[hnd].header.slat, sizeof (float), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      if (!fread (&chrtrh[hnd].header.nlat, sizeof (float), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      if (!fread (&chrtrh[hnd].header.grid_minutes, sizeof (float), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      if (!fread (&chrtrh[hnd].header.width, sizeof (int32_t), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}
      if (!fread (&chrtrh[hnd].header.height, sizeof (int32_t), 1, chrtrh[hnd].fp))
	{
	  fprintf (stderr, "Read error in file %s, function %s at line %d.", __FILE__, __FUNCTION__, __LINE__ - 2);
          fflush (stderr);
	  return (-1);
	}

      if (chrtrh[hnd].swap)
        {
	  swap_float (&chrtrh[hnd].header.wlon);
	  swap_float (&chrtrh[hnd].header.elon);
	  swap_float (&chrtrh[hnd].header.slat);
	  swap_float (&chrtrh[hnd].header.nlat);
	  swap_float (&chrtrh[hnd].header.grid_minutes);
	  swap_int (&chrtrh[hnd].header.width);
	  swap_int (&chrtrh[hnd].header.height);
	  swap_float (&chrtrh[hnd].header.min_z);
	  swap_float (&chrtrh[hnd].header.max_z);
	}


      *header = chrtrh[hnd].header;


      chrtrh[hnd].grid_degrees = chrtrh[hnd].header.grid_minutes / 60.0;
    }
  else
    {
      hnd = -1;
    }

  return (hnd);
}



/********************************************************************/
/*!

  - Function:    create_chrtr

  - Purpose:     Create a chrtr file.

  - Author:      Jan C. Depner (area.based.editor@gmail.com)

  - Date:        02/24/09

  - Arguments:
                 - path           =    The chrtr file path
                 - header         =    CHRTR_HEADER structure to be populated

  - Returns:     int32_t          =    The file handle or -1 on error

********************************************************************/

int32_t create_chrtr (const char *path, CHRTR_HEADER *header)
{
  int32_t          i, j, hnd;
  uint8_t          zero = 0;


  /*  Find the next available handle and make sure we haven't opened too many.  */

  hnd = MAX_CHRTR_FILES;
  for (i = 0 ; i < MAX_CHRTR_FILES ; i++)
    {
      if (chrtrh[i].fp == NULL)
        {
          hnd = i;
          break;
        }
    }


  if (hnd == MAX_CHRTR_FILES)
    {
      fprintf (stderr, "\n\nToo many open chrtr files!\nTerminating!\n\n");
      exit (-1);
    }


  /*  Open the file and write the header.  */

  if ((chrtrh[hnd].fp = fopen (path, "wb+")) != NULL)
    {
      fwrite (&header->wlon, sizeof (float), 1, chrtrh[hnd].fp);
      fwrite (&header->elon, sizeof (float), 1, chrtrh[hnd].fp);
      fwrite (&header->slat, sizeof (float), 1, chrtrh[hnd].fp);
      fwrite (&header->nlat, sizeof (float), 1, chrtrh[hnd].fp);
      fwrite (&header->grid_minutes, sizeof (float), 1, chrtrh[hnd].fp);
      fwrite (&header->width, sizeof (int32_t), 1, chrtrh[hnd].fp);
      fwrite (&header->height, sizeof (int32_t), 1, chrtrh[hnd].fp);
      i = 0x00010203;
      fwrite (&i, sizeof (int32_t), 1, chrtrh[hnd].fp);
      fwrite (&header->min_z, sizeof (float), 1, chrtrh[hnd].fp);
      fwrite (&header->max_z, sizeof (float), 1, chrtrh[hnd].fp);

      j = ftell (chrtrh[hnd].fp);

      for (i = j ; i < (int32_t) header->width * (int32_t) sizeof (float) ; i++)
	{
	  fwrite (&zero, 1, 1, chrtrh[hnd].fp);
	}

      chrtrh[hnd].header = *header;
      chrtrh[hnd].grid_degrees = header->grid_minutes / 60.0;
    }
  else
    {
      hnd = -1;
    }

  return (hnd);
}



/********************************************************************/
/*!

  - Function:    close_chrtr

  - Purpose:     Close a chrtr file.

  - Author:      Jan C. Depner (area.based.editor@gmail.com)

  - Date:        03/21/08

  - Arguments:   hnd            -    The file handle

  - Returns:     N/A

********************************************************************/

void close_chrtr (int32_t hnd)
{
  fclose (chrtrh[hnd].fp);
  chrtrh[hnd].fp = NULL;
  memset (&chrtrh[hnd], 0, sizeof (INTERNAL_CHRTR_HEADER));
  chrtrh[hnd].fp = NULL;
}



/********************************************************************/
/*!

  - Function:    read_chrtr

  - Purpose:     Retrieve a portion of a chrtr row from a chrtr file.

  - Author:      Jan C. Depner (area.based.editor@gmail.com)

  - Date:        03/21/08

  - Arguments:
                 - hnd            =    The file handle
                 - row            =    The row number of the chrtr
                                       row to be retrieved
                 - start_col      =    starting column number to read
                 - num_cols       =    number of columns to read
                 - data           =    The returned array of floats

  Returns:       uint8_t          =    NVFalse on error

********************************************************************/

uint8_t read_chrtr (int32_t hnd, int32_t row, int32_t start_col, int32_t num_cols, float *data)
{
  int32_t i, pos;


  /*  Compute the record position within the CHRTR file (row + 1 gets us past the header).  */
    
  pos = (row + 1) * chrtrh[hnd].header.width * sizeof (float) + start_col * sizeof (float);

  if (fseek (chrtrh[hnd].fp, pos, SEEK_SET)) return (NVFalse);

  if (!fread (data, sizeof (float), num_cols, chrtrh[hnd].fp)) return (NVFalse);

  if (chrtrh[hnd].swap) for (i = 0 ; i < num_cols ; i++) swap_float (&data[i]);

  return (NVTrue);
}



/********************************************************************/
/*!

  - Function:    write_chrtr

  - Purpose:     write a portion of a chrtr row to a chrtr file.

  - Author:      Jan C. Depner (area.based.editor@gmail.com)

  - Date:        03/21/08

  - Arguments:
                 - hnd            =    The file handle
                 - row            =    The row number of the chrtr
                                       row to be written
                 - start_col      =    starting column number to write
                 - num_cols       =    number of columns to write
                 - data           =    The array of floats to write

  - Returns:     uint8_t          =    NVFalse on error

********************************************************************/

uint8_t write_chrtr (int32_t hnd, int32_t row, int32_t start_col, int32_t num_cols, float *data)
{
  int32_t pos;


  /*  Compute the record position within the CHRTR file (row + 1 gets us past the header).  */
    
  pos = (row + 1) * chrtrh[hnd].header.width * sizeof (float) + start_col * sizeof (float);

  if (fseek (chrtrh[hnd].fp, pos, SEEK_SET)) return (NVFalse);

  if (!fwrite (data, sizeof (float), num_cols, chrtrh[hnd].fp)) return (NVFalse);

  return (NVTrue);
}



/********************************************************************/
/*!

  - Function:    get_chrtr_value

  - Purpose:     Given a lat/lon, compute the cell that contains that
                 position and return the value of that cell.

  - Author:      Jan C. Depner (area.based.editor@gmail.com)

  - Date:        03/21/08

  - Arguments:
                 - hnd            =    The file handle
                 - lat            =    Latitude
                 - lon            =    Longitude
                 - value          =    Returned value

  Returns:       uint8_t          =    NVFalse if position is outside
                                       of area or there is an error on
                                       seek or read

********************************************************************/

uint8_t get_chrtr_value (int32_t hnd, double lat, double lon, float *value)
{
  int32_t pos, row, col;


  if (lat < chrtrh[hnd].header.slat || lat > chrtrh[hnd].header.nlat ||
      lon < chrtrh[hnd].header.wlon || lon > chrtrh[hnd].header.elon)
    return (NVFalse);


  row = (lat - chrtrh[hnd].header.slat) / chrtrh[hnd].grid_degrees;
  col = (lon - chrtrh[hnd].header.wlon) / chrtrh[hnd].grid_degrees;


  /*  Compute the record position within the CHRTR file (row + 1 gets us past the header).  */
    
  pos = (row + 1) * chrtrh[hnd].header.width * sizeof (float) + col * sizeof (float);

  if (fseek (chrtrh[hnd].fp, pos, SEEK_SET)) return (NVFalse);

  if (!fread (value, sizeof (float), 1, chrtrh[hnd].fp)) return (NVFalse);

  if (chrtrh[hnd].swap) swap_float (value);

  return (NVTrue);
}



/********************************************************************/
/*!

  - Function:    dump_chrtr_header

  - Purpose:     Print the header info to stderr

  - Author:      Jan C. Depner (area.based.editor@gmail.com)

  - Date:        03/21/08

  - Arguments:   hnd            -    The file handle

  - Returns:     void

********************************************************************/

void dump_chrtr_header (int32_t hnd)
{
  if (fseek (chrtrh[hnd].fp, 0, SEEK_SET))
    {
      fprintf (stderr, "Unable to seek to location 0 for CHRTR handle %d\n", hnd);
    }
  else
    {
      fprintf (stderr, "\nWest lon            : %.09f\n", chrtrh[hnd].header.wlon);
      fprintf (stderr, "East lon            : %.09f\n", chrtrh[hnd].header.elon);
      fprintf (stderr, "South lat           : %.09f\n", chrtrh[hnd].header.slat);
      fprintf (stderr, "North lat           : %.09f\n", chrtrh[hnd].header.nlat);
      fprintf (stderr, "Grid size (minutes) : %.06f\n", chrtrh[hnd].header.grid_minutes);
      fprintf (stderr, "Width (bins)        : %d\n", chrtrh[hnd].header.width);
      fprintf (stderr, "Height (bins)       : %d\n", chrtrh[hnd].header.height);
      fprintf (stderr, "Swap                : %d\n", chrtrh[hnd].swap);
      fprintf (stderr, "Minimum Z           : %.04f\n", chrtrh[hnd].header.min_z);
      fprintf (stderr, "Maximum Z           : %.04f\n\n", chrtrh[hnd].header.max_z);
    }
}
