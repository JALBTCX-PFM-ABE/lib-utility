
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

#include "dted.h"


/*  This stuff only works on little endian systems.  */


static double convert_pos (char *string)
{
    int32_t          d[3];
    double           sign;


    /*  Check for and clear hemisphere indicator.          */
        
    sign = 1.0;
    if ((string[6] == 'S') || (string[6] == 'N'))
      {
        if (string[6] == 'S') sign = -1.0;
        string[6] = 0;

        sscanf (string, "%2d%2d%2d", &d[0], &d[1], &d[2]);
      }
    else
      {
        if ((string[7] == 'W') || (string[7] == 'S')) sign = -1.0;
        string[7] = 0;

        sscanf (string, "%3d%2d%2d", &d[0], &d[1], &d[2]);
      }

    return (sign * (double) d[0] + ((double) d[2] / 3600.0) + 
            ((double) d[1] / 60.0));
}


int32_t read_uhl (FILE *fp, UHL *uhl)
{
  uint8_t            uhl_data[80], cut[20];
  int32_t            interval[2];


  if (!fread (uhl_data, sizeof (uhl_data), 1, fp)) return (-1);


  strncpy (uhl->sentinel, (char *) &uhl_data[0], 3);
  uhl->sentinel[3] = 0;


  if (strcmp (uhl->sentinel, "UHL")) return (-1);


  strncpy ((char *) cut, (char *) &uhl_data[4], 8);
  cut[8] = 0;
  uhl->ll_lon = convert_pos ((char *) cut);

  strncpy ((char *) cut, (char *) &uhl_data[12], 8);
  cut[8] = 0;
  uhl->ll_lat = convert_pos ((char *) cut);

  sscanf ((char *) &uhl_data[20], "%4d%4d", &interval[0], &interval[1]);
  uhl->lon_int = (double) interval[0] / 10.0;
  uhl->lat_int = (double) interval[1] / 10.0;

  strncpy ((char *) uhl->vertical_accuracy, (char *) &uhl_data[28], 4);
  uhl->vertical_accuracy[4] = 0;

  strncpy ((char *) uhl->security_code, (char *) &uhl_data[32], 3);
  uhl->security_code[2] = 0;

  strncpy ((char *) uhl->unique_reference, (char *) &uhl_data[35], 12);
  uhl->unique_reference[12] = 0;

  sscanf ((char *) &uhl_data[47], "%4hd%4hd", &uhl->num_lon_lines, &uhl->num_lat_points);

  uhl->multiple_accuracy = uhl_data[55];
 

  return (0);
}


int32_t read_dsi (FILE *fp, DSI *dsi __attribute__ ((unused)))
{
  uint8_t            dsi_data[648];

  if (!fread (dsi_data, sizeof (dsi_data), 1, fp)) return (-1);

  return (0);
}


int32_t read_acc (FILE *fp, ACC *acc __attribute__ ((unused)))
{
  uint8_t            acc_data[2700];

  if (!fread (acc_data, sizeof (acc_data), 1, fp)) return (-1);

  return (0);
}


int32_t read_dted_data (FILE *fp, int32_t count, int32_t block, DTED_DATA *dted_data)
{
  uint8_t            data[7214];
  int32_t            i, j, cpos, position, size;
  uint32_t           checksum[2], sign = 1;



  cpos = 8 + count * 2;
  size = cpos + 4;

  position = 80 + 648 + 2700 + block * size;

  fseek (fp, position, SEEK_SET);


  if (!fread (&data, size, 1, fp)) return (-1);

  if (data[0] != 0xaa) return (-2);

  checksum[0] = data[cpos] * 16777216 + data[cpos + 1] * 65536 + 
    data[cpos + 2] * 256 + data[cpos + 3];
  checksum[1] = 0;
  for (i = 0 ; i < cpos ; i++) checksum[1] += data[i];
  if (checksum[0] != checksum[1]) return (-3);


  dted_data->sentinel = data[0];

  dted_data->block_count = data[1] * 65536 + data[2] * 256 + data[3];
  dted_data->longitude_count = data[4] * 256 + data[5];
  dted_data->latitude_count = data[6] * 256 + data[7];


  for (i = 0 ; i < count ; i++)
    {
      j = i * 2 + 8;
      if (data[j] & 0x80)
        {
          data[j] &=0x7f;
          sign = -1;
        }
      else
        {
          sign = 1;
        }
      dted_data->elev[i] = (data[j] * 256 + data[j + 1]) * sign;
    }


  return (0);
}


int32_t write_dted_data (FILE *fp, int32_t count, int32_t block, DTED_DATA dted_data)
{
  uint8_t            data[7214];
  int32_t            i, j, position, cpos, size;
  int16_t            elev;
  uint32_t           checksum[2], sign = 0;



  cpos = 8 + count * 2;
  size = cpos + 4;

  position = 80 + 648 + 2700 + block * size;

  fseek (fp, position, SEEK_SET);


  if (!fread (&data, size, 1, fp)) return (-1);

  if (data[0] != 0xaa) return (-2);

  checksum[0] = data[cpos] * 16777216 + data[cpos + 1] * 65536 + 
    data[cpos + 2] * 256 + data[cpos + 3];
  checksum[1] = 0;
  for (i = 0 ; i < cpos ; i++) checksum[1] += data[i];
  if (checksum[0] != checksum[1]) return (-3);


  for (i = 0 ; i < count ; i++)
    {
      j = i * 2 + 8;
      if (dted_data.elev[i] < 0)
        {
          sign = 0x80;
          elev = -dted_data.elev[i];
        }
      else
        {
          sign = 0;
          elev = dted_data.elev[i];
        }
      data[j] = ((elev & 0xff00) >> 8) | sign;
      data[j + 1] = elev & 0x00ff;
    }

  checksum[0] = 0;
  for (i = 0 ; i < cpos ; i++) checksum[0] += data[i];

  data[cpos] = (checksum[0] & 0xff000000) >> 24;
  data[cpos + 1] = (checksum[0] & 0x00ff0000) >> 16;
  data[cpos + 2] = (checksum[0] & 0x0000ff00) >> 8;
  data[cpos + 3] = checksum[0] & 0x000000ff;


  fseek (fp, position, SEEK_SET);
  if (!fwrite (&data, size, 1, fp)) return (-4);

  return (0);
}


void dump_uhl (UHL uhl)
{
  fprintf (stderr, "\nSentinel - %s\n", uhl.sentinel);
  fprintf (stderr, "Lower left lon - %f\n", uhl.ll_lon);
  fprintf (stderr, "Lower left lat - %f\n", uhl.ll_lat);
  fprintf (stderr, "Lon interval - %f\n", uhl.lon_int);
  fprintf (stderr, "Lat interval - %f\n", uhl.lat_int);
  fprintf (stderr, "Vertical accuracy - %s\n", uhl.vertical_accuracy);
  fprintf (stderr, "Security code - %s\n", uhl.security_code);
  fprintf (stderr, "Unique reference - %s\n", uhl.unique_reference);
  fprintf (stderr, "Number of lon lines - %d\n", uhl.num_lon_lines);
  fprintf (stderr, "Number of lat points - %d\n", uhl.num_lat_points);
  fprintf (stderr, "Multiple accuracy - %c\n\n", uhl.multiple_accuracy);
  fflush (stderr);
}


void dump_dsi (DSI dsi __attribute__ ((unused)))
{
}


void dump_acc (ACC acc __attribute__ ((unused)))
{
}


void dump_dted_data (DTED_DATA dted_data __attribute__ ((unused)))
{
}
