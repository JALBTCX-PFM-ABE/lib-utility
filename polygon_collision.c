


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


#include "polygon_collision.h"


/***************************************************************************/
/*!

  - Module Name:        polygon_collision

  - Programmer(s):      Jan C. Depner

  - Date Written:       December 11, 2009

  - Purpose:            Determines if two polygons intersect

  - Arguments:
  - Return Value:       NVTrue if they intersect, otherwise NVFalse

****************************************************************************/

uint8_t polygon_collision (NV_F64_COORD2 *poly1, int32_t npol1, NV_F64_COORD2 *poly2, int32_t npol2)
{
  int32_t i, j;
  double x, y;


  /*  Easy check first.  If any vertex of one polygon lies inside of the other then they intersect.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      /*  First check to see if this vertex of polygon 1 lies inside polygon 2.  */

      if (inside_polygon (poly2, npol2, poly1[i].x, poly1[i].y)) return (NVTrue);


      for (j = 0 ; j < npol2 ; j++)
        {
          /*  Check to see if this vertex of polygon 2 lies inside polygon 1.  */

          if (inside_polygon (poly1, npol1, poly2[j].x, poly2[j].y)) return (NVTrue);
        }
    }


  /*  If we got here then it's time to do the hard check.  If any two lines intersect then there is a collision.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      for (j = 0 ; j < npol2 ; j++)
        {
          if (i)
            {
              if (j)
                {
                  if (line_intersection (poly1[i - 1].x, poly1[i - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[j - 1].x, poly2[j - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1[i - 1].x, poly1[i - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[npol2 - 1].x, poly2[npol2 - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
            }


          /*  If i is 0 then we have to check the last to first point line segment.  */

          else
            {
              if (j)
                {
                  if (line_intersection (poly1[npol1 - 1].x, poly1[npol1 - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[j - 1].x, poly2[j - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1[npol1 - 1].x, poly1[npol1 - 1].y, poly1[i].x, poly1[i].y,
                                         poly2[npol2 - 1].x, poly2[npol2 - 1].y, poly2[j].x, poly2[j].y, &x, &y) == 2) return (NVTrue);
                }
            }
        }
    }

  return (NVFalse);
}



uint8_t polygon_collision2 (double *poly1x, double *poly1y, int32_t npol1, double *poly2x, double *poly2y, int32_t npol2)
{
  int32_t i, j;
  double x, y;


  /*  Easy check first.  If any vertex of one polygon lies inside of the other then they intersect.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      /*  First check to see if this vertex of polygon 1 lies inside polygon 2.  */

      if (inside_polygon2 (poly2x, poly2y, npol2, poly1x[i], poly1y[i])) return (NVTrue);


      for (j = 0 ; j < npol2 ; j++)
        {
          /*  Check to see if this vertex of polygon 2 lies inside polygon 1.  */

          if (inside_polygon2 (poly1x, poly1y, npol1, poly2x[j], poly2y[j])) return (NVTrue);
        }
    }


  /*  If we got here then it's time to do the hard check.  If any two lines intersect then there is a collision.  */

  for (i = 0 ; i < npol1 ; i++)
    {
      for (j = 0 ; j < npol2 ; j++)
        {
          if (i)
            {
              if (j)
                {
                  if (line_intersection (poly1x[i - 1], poly1y[i - 1], poly1x[i], poly1y[i],
                                         poly2x[j - 1], poly2y[j - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1x[i - 1], poly1y[i - 1], poly1x[i], poly1y[i],
                                         poly2x[npol2 - 1], poly2y[npol2 - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
            }


          /*  If i is 0 then we have to check the last to first point line segment.  */

          else
            {
              if (j)
                {
                  if (line_intersection (poly1x[npol1 - 1], poly1y[npol1 - 1], poly1x[i], poly1y[i],
                                         poly2x[j - 1], poly2y[j - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
              else
                {
                  /*  If j is 0 then we have to check the last to first point line segment.  */

                  if (line_intersection (poly1x[npol1 - 1], poly1y[npol1 - 1], poly1x[i], poly1y[i],
                                         poly2x[npol2 - 1], poly2y[npol2 - 1], poly2x[j], poly2y[j], &x, &y) == 2) return (NVTrue);
                }
            }
        }
    }

  return (NVFalse);
}
