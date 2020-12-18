
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



#include "inside_polygon.h"

/***************************************************************************/
/*!
  - Module Name:        inside_polygon

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 2014

  - Purpose:            Based on the CrossingsMultiplyTest method described
                        by E. Haines:
                        http://erich.realtimerendering.com/ptinpoly/


  - Arguments (inside_polygon):
                        - poly     =   Array of NV_F64_COORD2 structures holding
                                       the polygon points
                        - npol     =   Number of points in the polygon
                        - x        =   X value of point to be tested
                        - y        =   Y value of point to be tested

  - Arguments (inside_polygon2):
                        - poly_x   =   Array containing the X coordinates of the 
                                       polygon's vertices
                        - poly_y   =   Array containing the Y coordinates of the 
                                       polygon's vertices
                        - npol     =   Number of points in the polygon
                        - x        =   X value of point to be tested
                        - y        =   Y value of point to be tested

  - Arguments (inside_polygon3):
                        - xs       =   Array containing the integer X coordinates
                                       of the polygon's vertices
                        - ys       =   Array containing the integer Y coordinates
                                       of the polygon's vertices
                        - npol     =   Number of points in the polygon
                        - x        =   integer X value of point to be tested
                        - y        -   integer Y value of point to be tested

  - Return Value:       1 if point is inside polygon, 0 if outside

  - Caveats:            The last point of the polygon must be the same as
                        the first point and the vertices of the polygon must
			be in order around the polygon.

****************************************************************************/

int32_t inside_polygon (NV_F64_COORD2 *poly, int32_t npol, double x, double y)
{
  int32_t    i, j, yflag0, yflag1, inside_flag;

  inside_flag = 0;


  j = npol - 1;
  yflag0 = (poly[j].y >= y);

  for (i = 0 ; i < npol ; i++)
    {
      yflag1 = (poly[i].y >= y);


      if (yflag0 != yflag1)
	{
	  if (((poly[i].y - y) * (poly[j].x - poly[i].x) >= (poly[i].x - x) * (poly[j].y - poly[i].y)) == yflag1)
	    {
	      inside_flag = !inside_flag;
	    }
	}

      yflag0 = yflag1;
      j = i;
    }

  return (inside_flag);
}


int32_t inside_polygon2 (double *poly_x, double *poly_y, int32_t npol, double x, double y)
{
  int32_t    i, j, yflag0, yflag1, inside_flag;

  inside_flag = 0;


  j = npol - 1;
  yflag0 = (poly_y[j] >= y);

  for (i = 0 ; i < npol ; i++)
    {
      yflag1 = (poly_y[i] >= y);


      if (yflag0 != yflag1)
	{
	  if (((poly_y[i] - y) * (poly_x[j] - poly_x[i]) >= (poly_x[i] - x) * (poly_y[j] - poly_y[i])) == yflag1)
	    {
	      inside_flag = !inside_flag;
	    }
	}

      yflag0 = yflag1;
      j = i;
    }

  return (inside_flag);
}


int32_t inside_polygon3 (int32_t *xs, int32_t *ys, int32_t npol, int32_t x, int32_t y)
{
  int32_t    i, j, yflag0, yflag1, inside_flag;

  inside_flag = 0;


  j = npol - 1;
  yflag0 = (ys[j] >= y);

  for (i = 0 ; i < npol ; i++)
    {
      yflag1 = (ys[i] >= y);


      if (yflag0 != yflag1)
	{
	  if (((double) (ys[i] - y) * (double) (xs[j] - xs[i]) >= (double) (xs[i] - x) * (double) (ys[j] - ys[i])) == yflag1)
	    {
	      inside_flag = !inside_flag;
	    }
	}

      yflag0 = yflag1;
      j = i;
    }

  return (inside_flag);
}
