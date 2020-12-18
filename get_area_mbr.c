
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



#include "get_area_mbr.h"


/*!  Get an NV_F64_XYMBR from the supplied area file (path) and return in polygon X and Y arrays.  Area file may be .ARE, .are, or .afs.  */

uint8_t get_area_mbr (const char *path, int32_t *polygon_count, double *polygon_x, double *polygon_y, NV_F64_XYMBR *mbr)
{
  char                    string[256], tmp[256], lat_hemi, lon_hemi;
  FILE                    *area_fp;
  int32_t                 i, lat_deg, lat_min, lon_deg, lon_min, numShapes, type;
  float                   lat_sec, lon_sec;
  double                  minBounds[4], maxBounds[4];
  uint8_t                 west = NVFalse, east = NVFalse;
  SHPHandle               shpHandle;
  SHPObject               *shape = NULL;


  /*  Don't open the file if it is a shape file.  */

  if (strcmp (&path[strlen (path) - 4], ".shp"))
    {
      if ((area_fp = fopen (path, "r")) == NULL)
        {
          perror (path);
          return (NVFalse);
        }
    }


  *polygon_count = 0;


  /*  ISS-60 format area file.  */

  if (!strcmp (&path[strlen (path) - 4], ".ARE"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
          if (!strncmp (string, "POINT=", 6))
            {
              strtok (string, ";");
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon_y[*polygon_count], POS_LAT);
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon_x[*polygon_count], POS_LON);

              (*polygon_count)++;
            }
        }
    }


  /*  Polygon list format area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".are"))
    {
      while (fget_coord (area_fp, &lat_hemi, &lat_deg, &lat_min, &lat_sec, 
                         &lon_hemi, &lon_deg, &lon_min, &lon_sec))
        {
          polygon_y[*polygon_count] = (double) lat_deg + (double) lat_min / 60.0 + 
            (double) lat_sec / 3600.0;
          if (lat_hemi == 'S') polygon_y[*polygon_count] = -polygon_y[*polygon_count];

          polygon_x[*polygon_count] = (double) lon_deg + (double) lon_min / 60.0 +
            (double) lon_sec / 3600.0;
          if (lon_hemi == 'W') polygon_x[*polygon_count] = -polygon_x[*polygon_count];

          (*polygon_count)++;
        }
    }


  /*  Army Corps area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".afs"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
	  if (strchr (string, ','))
	    {
	      sscanf (string, "%lf,%lf", &polygon_x[*polygon_count], &polygon_y[*polygon_count]);
	    }
	  else
	    {
	      sscanf (string, "%lf %lf", &polygon_x[*polygon_count], &polygon_y[*polygon_count]);
	    }

          (*polygon_count)++;
        }
    }


  /*  Shape file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".shp"))
    {
      //  Open shape file

      shpHandle = SHPOpen (path, "rb");

      if (shpHandle != NULL)
        {
          //  Get shape file header info

          SHPGetInfo (shpHandle, &numShapes, &type, minBounds, maxBounds);


          //  Read all shapes

          for (int32_t k = 0 ; k < numShapes ; k++)
            {
              shape = SHPReadObject (shpHandle, k);


              //  Get all vertices

              if (shape->nVertices >= 2)
                {
                  (*polygon_count) = 0;

                  for (int32_t j = 0 ; j < shape->nVertices ; j++)
                    {
                      polygon_y[*polygon_count] = shape->padfY[j];
                      polygon_x[*polygon_count] = shape->padfX[j];

                      (*polygon_count)++;
                    }
                }


              /*  If this is a PolyLine file instead of a Polygon file we need to dupe the first point and increase the count by one.  */

              if (type == SHPT_ARC || type == SHPT_ARCZ || type == SHPT_ARCM)
                {
                  polygon_y[*polygon_count] = shape->padfY[0];
                  polygon_x[*polygon_count] = shape->padfX[0];

                  (*polygon_count)++;
                }


              SHPDestroyObject (shape);
            }

          SHPClose (shpHandle);
        }
      else
        {
          return (NVFalse);
        }
    }
  else
    {
      if (strcmp (&path[strlen (path) - 4], ".shp")) fclose (area_fp);
      return (NVFalse);
    }


  if (strcmp (&path[strlen (path) - 4], ".shp")) fclose (area_fp);


  /*  Check for dateline crossing.  If you're making an area that goes more than half way around the earth
      you're on your own!  */

  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (polygon_x[i] < -90.0) west = NVTrue;
      if (polygon_x[i] > 90.0) east = NVTrue;
    }


  mbr->min_y = 99999999999.0;
  mbr->min_x = 99999999999.0;
  mbr->max_y = -99999999999.0;
  mbr->max_x = -99999999999.0;
    
  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (east && west && polygon_x[i] < 0.0) polygon_x[i] += 360.0;

      if (polygon_y[i] < mbr->min_y) mbr->min_y = polygon_y[i];
      if (polygon_y[i] > mbr->max_y) mbr->max_y = polygon_y[i];
      if (polygon_x[i] < mbr->min_x) mbr->min_x = polygon_x[i];
      if (polygon_x[i] > mbr->max_x) mbr->max_x = polygon_x[i];
    }

  return (NVTrue);
}



/*!  Get an NV_F64_XYMBR from the supplied area file (path) and return in polygon NV_F64_COORD2 array.  Area file may be .ARE, .are, or .afs.  */

uint8_t get_area_mbr2 (const char *path, int32_t *polygon_count, NV_F64_COORD2 *polygon, NV_F64_XYMBR *mbr)
{
  char                    string[256], tmp[256], lat_hemi, lon_hemi;
  FILE                    *area_fp;
  int32_t                 i, lat_deg, lat_min, lon_deg, lon_min, numShapes, type;
  float                   lat_sec, lon_sec;
  double                  minBounds[4], maxBounds[4];;
  uint8_t                 west = NVFalse, east = NVFalse;
  SHPHandle               shpHandle;
  SHPObject               *shape = NULL;


  if ((area_fp = fopen (path, "r")) == NULL)
    {
      perror (path);
      return (NVFalse);
    }
    

  *polygon_count = 0;


  /*  ISS-60 format area file.  */

  if (!strcmp (&path[strlen (path) - 4], ".ARE"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
          if (!strncmp (string, "POINT=", 6))
            {
              strtok (string, ";");
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon[*polygon_count].y, POS_LAT);
              strcpy (tmp, strtok (NULL, ";"));
              posfix (tmp, &polygon[*polygon_count].x, POS_LON);

              (*polygon_count)++;
            }
        }
    }


  /*  Polygon list format area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".are"))
    {
      while (fget_coord (area_fp, &lat_hemi, &lat_deg, &lat_min, &lat_sec, 
                         &lon_hemi, &lon_deg, &lon_min, &lon_sec))
        {
          polygon[*polygon_count].y = (double) lat_deg + (double) lat_min / 60.0 + 
            (double) lat_sec / 3600.0;
          if (lat_hemi == 'S') polygon[*polygon_count].y = -polygon[*polygon_count].y;

          polygon[*polygon_count].x = (double) lon_deg + (double) lon_min / 60.0 +
            (double) lon_sec / 3600.0;
          if (lon_hemi == 'W') polygon[*polygon_count].x = -polygon[*polygon_count].x;

          (*polygon_count)++;
        }
    }


  /*  Army Corps area file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".afs"))
    {
      while (fgets (string, sizeof (string), area_fp) != NULL)
        {
	  if (strchr (string, ','))
	    {
	      sscanf (string, "%lf,%lf", &polygon[*polygon_count].x, &polygon[*polygon_count].y);
	    }
	  else
	    {
	      sscanf (string, "%lf %lf", &polygon[*polygon_count].x, &polygon[*polygon_count].y);
	    }

          (*polygon_count)++;
        }
    }


  /*  Shape file.  */

  else if (!strcmp (&path[strlen (path) - 4], ".shp"))
    {
      //  Open shape file

      shpHandle = SHPOpen (path, "rb");

      if (shpHandle != NULL)
        {
          //  Get shape file header info

          SHPGetInfo (shpHandle, &numShapes, &type, minBounds, maxBounds);


          //  Read all shapes

          for (int32_t k = 0 ; k < numShapes ; k++)
            {
              shape = SHPReadObject (shpHandle, k);


              //  Get all vertices

              if (shape->nVertices >= 2)
                {
                  (*polygon_count) = 0;

                  for (int32_t j = 0 ; j < shape->nVertices ; j++)
                    {
                      polygon[*polygon_count].y = shape->padfY[j];
                      polygon[*polygon_count].x = shape->padfX[j];

                      (*polygon_count)++;
                    }
                }


              /*  If this is a PolyLine file instead of a Polygon file we need to dupe the first point and increase the count by one.  */

              if (type == SHPT_ARC || type == SHPT_ARCZ || type == SHPT_ARCM)
                {
                  polygon[*polygon_count].y = shape->padfY[0];
                  polygon[*polygon_count].x = shape->padfX[0];

                  (*polygon_count)++;
                }


              SHPDestroyObject (shape);
            }

          SHPClose (shpHandle);
        }
      else
        {
          return (NVFalse);
        }
    }
  else
    {
      fclose (area_fp);
      return (NVFalse);
    }


  fclose (area_fp);


  /*  Check for dateline crossing.  If you're making an area that goes more than half way around the earth
      you're on your own!  */

  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (polygon[i].x < -90.0) west = NVTrue;
      if (polygon[i].x > 90.0) east = NVTrue;
    }


  mbr->min_y = 99999999999.0;
  mbr->min_x = 99999999999.0;
  mbr->max_y = -99999999999.0;
  mbr->max_x = -99999999999.0;
    
  for (i = 0 ; i < *polygon_count ; i++)
    {
      if (east && west && polygon[i].x < 0.0) polygon[i].x += 360.0;

      if (polygon[i].y < mbr->min_y) mbr->min_y = polygon[i].y;
      if (polygon[i].y > mbr->max_y) mbr->max_y = polygon[i].y;
      if (polygon[i].x < mbr->min_x) mbr->min_x = polygon[i].x;
      if (polygon[i].x > mbr->max_x) mbr->max_x = polygon[i].x;
    }

  return (NVTrue);
}



/*  Get an NV_F64_MBR (i.e. lat/lon) instead of an NV_F64_XYMBR.  */

uint8_t get_area_ll_mbr (const char *path, int32_t *polygon_count, double *polygon_x, double *polygon_y, NV_F64_MBR *mbr)
{
  NV_F64_XYMBR l_mbr;
  uint8_t ret;

  ret = get_area_mbr (path, polygon_count, polygon_x, polygon_y, &l_mbr);

  if (ret)
    {
      mbr->slat = l_mbr.min_y;
      mbr->nlat = l_mbr.max_y;
      mbr->wlon = l_mbr.min_x;
      mbr->elon = l_mbr.max_x;
    }

  return (ret);
}



/*!  Get an NV_F64_MBR from the supplied area file (path) and return in polygon NV_F64_COORD2 array.  Area file may be .ARE, .are, or .afs.  */

uint8_t get_area_ll_mbr2 (const char *path, int32_t *polygon_count, NV_F64_COORD2 *polygon, NV_F64_MBR *mbr)
{
  NV_F64_XYMBR l_mbr;
  uint8_t ret;

  ret = get_area_mbr2 (path, polygon_count, polygon, &l_mbr);

  if (ret)
    {
      mbr->slat = l_mbr.min_y;
      mbr->nlat = l_mbr.max_y;
      mbr->wlon = l_mbr.min_x;
      mbr->elon = l_mbr.max_x;
    }

  return (ret);
}



/*!  Determine if a polygon is a rectangle.  */

uint8_t polygon_is_rectangle (int32_t polygon_count, double *polygon_x, double *polygon_y)
{
  int32_t           j, k, lat_count = 0, lon_count = 0;
  double            *unique_lat, *unique_lon;
  uint8_t           unique;


  unique_lat = (double *) malloc (polygon_count * sizeof (double));
  if (unique_lat == NULL)
    {
      perror ("Allocating unique_lat in get_area_mbr.c");
      exit (-1);
    }

  unique_lon = (double *) malloc (polygon_count * sizeof (double));
  if (unique_lon == NULL)
    {
      perror ("Allocating unique_lon in get_area_mbr.c");
      exit (-1);
    }


  for (j = 0 ; j < polygon_count ; j++)
    {
      unique = NVTrue;
      for (k = 0 ; k < lat_count ; k++)
        {
          if (polygon_y[j] == unique_lat[k])
            {
              unique = NVFalse;
              break;
            }
        }

      if (unique)
        {
          unique_lat[lat_count] = polygon_y[j];
          lat_count++;
        }


      unique = NVTrue;
      for (k = 0 ; k < lon_count ; k++)
        {
          if (polygon_x[j] == unique_lon[k])
            {
              unique = NVFalse;
              break;
            }
        }

      if (unique)
        {
          unique_lon[lon_count] = polygon_x[j];
          lon_count++;
        }
    }

  free (unique_lat);
  free (unique_lon);


  //  If there are only two unique latitudes and two unique longitudes then this is a rectangle.

  if (lat_count == 2 && lon_count == 2) return (NVTrue);


  return (NVFalse);
}



/*!  Determine if a polygon is a rectangle (NV_F64_COORD2).  */

uint8_t polygon_is_rectangle2 (int32_t polygon_count, NV_F64_COORD2 *polygon)
{
  double           *polygon_x, *polygon_y;
  int32_t          i;
  uint8_t          ret;


  polygon_x = (double *) malloc (polygon_count * sizeof (double));
  if (polygon_x == NULL)
    {
      perror ("Allocating polygon_x in get_area_mbr.c");
      exit (-1);
    }

  polygon_y = (double *) malloc (polygon_count * sizeof (double));
  if (polygon_y == NULL)
    {
      perror ("Allocating polygon_y in get_area_mbr.c");
      exit (-1);
    }


  for (i = 0 ; i < polygon_count ; i++)
    {
      polygon_x[i] = polygon[i].x;
      polygon_y[i] = polygon[i].y;
    }


  ret = polygon_is_rectangle (polygon_count, polygon_x, polygon_y);


  free (polygon_x);
  free (polygon_y);


  return (ret);
}
