
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



/*  nvMap class definitions.  */

#ifndef __NVMAP_H__
#define __NVMAP_H__

#include <cmath>


#include "pfm_nvtypes.h"
#include "nvdef.h"
#include "rproj.hpp"


#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif


#include "fixpos.h"


#define NVMAP_LEFT             0           //!<  Move map one block to left with overlap
#define NVMAP_UP               1           //!<  Move map one block up with overlap
#define NVMAP_RIGHT            2           //!<  Move map one block to right with overlap
#define NVMAP_DOWN             3           //!<  Move map one block down with overlap
#define NVMAP_UP_LEFT          4           //!<  Move map one block up and left with overlap
#define NVMAP_UP_RIGHT         5           //!<  Move map one block up and right with overlap
#define NVMAP_DOWN_LEFT        6           //!<  Move map one block down and left with overlap
#define NVMAP_DOWN_RIGHT       7           //!<  Move map one block down and right with overlap


#define NO_PROJECTION          -1          //!<  No map projection defined


#define NVMAP_POLYGON_POINTS   2000
#define NVMAP_ZOOM_LEVELS      100


/*  Coastline types  */

#define NVMAP_NO_COAST         0           //!<  Don't draw coastline
#define NVMAP_AUTO_COAST       1           //!<  Automatically determine coastline density based on size
#define NVMAP_NGA_50K_COAST    2           //!<  Use NGA's restricted 1:50,000 coastline
#define NVMAP_WVS_FULL_COAST   3           //!<  Use full resolution WVS coastline
#define NVMAP_WVS_1M_COAST     4           //!<  Use 1:1,000,000 WVS coastline
#define NVMAP_WVS_3M_COAST     5           //!<  Use 1:3,000,000 WVS coastline
#define NVMAP_WVS_12M_COAST    6           //!<  Use 1:12,000,000 WVS coastline
#define NVMAP_WVS_43M_COAST    7           //!<  Use 1:43,000,000 WVS coastline


/*!  nvmap definition structure.  */

typedef struct
{
  NV_F64_XYMBR                  initial_bounds;                   //!<  Initial x,y boundaries
  NV_F64_XYMBR                  bounds[NVMAP_ZOOM_LEVELS];        //!<  All zoom bounds
  int32_t                       projection;                       //!<  Projection number
  int32_t                       zoom_level;                       //!<  Zoom level
  int32_t                       zoom_percent;                     //!<  Zoom in/out percentage (0 - 100)
  double                        center_x;                         //!<  Center x value
  double                        center_y;                         //!<  Center y value
  int32_t                       border;                           //!<  Border in pixels
  int32_t                       overlap_percent;                  //!<  Move overlap percentage (0 - 100)
  double                        grid_inc_y;                       //!<  y grid overlay increment in minutes, 0 for no overlay
  double                        grid_inc_x;                       //!<  x grid overlay increment in minutes, 0 for no overlay
  int32_t                       grid_thickness;                   //!<  grid line thickness in pixels
  uint8_t                       frame;                            //!<  Set to draw a surrounding frame
  int32_t                       draw_width;                       //!<  Drawing area width in pixels
  int32_t                       draw_height;                      //!<  Drawing area height in pixels
  int32_t                       width;                            //!<  Map width in pixels
  int32_t                       height;                           //!<  Map height in pixels
  int32_t                       coasts;                           //!<  Type of coastline to draw (see Coastline types above)
  int32_t                       coasts_used;                      /*!<  Actual coastline last drawn (if coasts is set to NVMAP_AUTO_COAST
                                                                        this won't be NV_AUTO_COAST)  */
  uint8_t                       coast_thickness;                  //!<  Coast line thickness
  QColor                        coast_color;                      //!<  Coastline color
  uint8_t                       landmask;                         //!<  Landmask on or off
  QColor                        landmask_color;                   //!<  Landmask color
  QColor                        grid_color;                       //!<  Grid color
  QColor                        background_color;                 //!<  Background color
  QString                       textFont;                         //!<  Font to be used for text strings
  int32_t                       textWeight;                       //!<  Weight of font to be used for text strings
} NVMAP_DEF;



//  MOVABLE_OBJECT types

#define NVMAP_MOVE_POLYGON       0           //!<  Moveable polygon
#define NVMAP_MOVE_POLYLINE      1           //!<  Moveable polyline
#define NVMAP_MOVE_RECTANGLE     2           //!<  Moveable rectangle
#define NVMAP_MOVE_CIRCLE        3           //!<  Moveable circle
#define NVMAP_MOVE_PATH          4           //!<  Moveable path
#define NVMAP_MOVE_TEXT          5           //!<  Moveable text
#define NVMAP_RUBBER_POLYGON     6           //!<  Rubberband polygon
#define NVMAP_RUBBER_RECTANGLE   7           //!<  Rubberband rectangle
#define NVMAP_RUBBER_LINE        8           //!<  Rubberband line


#define MAX_MOVABLE_OBJECTS     20           //!<  Number of available movable objects


/*!
    Movable object section.  A little explanation - in order to avoid a ton of if statements in the 
    paint event I've implemented an object table (ot).  The object table is just an array of pointers
    to objects in the movable_object array.  This way we can track the number of the object without
    having to check state for each of MAX_MOVABLE_OBJECTS objects in the paint event.  We will have one
    "for" loop in the paint event that will go from 0 to movable_object_count (which equates to
    "movable_object_count" "if" statements).  That is, we're going to have a count of active objects
    and when we delete an object we'll compress the array.  Otherwise we have to do a "for" loop over
    MAX_MOVABLE_OBJECTS (which equates to MAX_MOVABLE_OBJECTS "if" statements) and an "if" per object.
    This would result in 2 * MAX_MOVABLE_OBJECTS "if" statements every time we had a paint event, even
    if we had NO movable_objects.  Confusing, yes, but much more efficient.  We will be accessing
    ot[object_number] (the pointer to the proper movable_object) when we're in the "for" loop or
    elsewhere in the code.  A pointer of NULL in the object table means the object has been discarded
    and is available for re-use.  I'm sure I could have done this using QGraphicsView but, since I don't
    have the entire scene visible (or in memory) at any given time, it would have been less efficient.
    For most of my stuff I'll only be handling a small number of movable objects.
*/

typedef struct
{
  int32_t                       type;                             //!<  Type of object
  uint8_t                       active;                           //!<  Whether the object is active
  uint8_t                       filled;                           //!<  Whether the object is filled
  uint8_t                       outlined;                         //!<  Whether the object is outlined
  uint8_t                       close_polygon;                    //!<  Whether to automatically close the polygon
  QBrush                        brush;                            //!<  Brush to use to fill object
  Qt::PenStyle                  penStyle;                         //!<  Pen style to use
  int32_t                       polygon_x[NVMAP_POLYGON_POINTS];  //!<  The X polygon points
  int32_t                       polygon_y[NVMAP_POLYGON_POINTS];  //!<  The Y polygon points
  int32_t                       polygon_count;                    //!<  The number of points in a polygon
  int32_t                       x;                                //!<  X position of object in screen coordinates
  int32_t                       y;                                //!<  Y position of object in screen coordinates
  int32_t                       width;                            //!<  Width of object in screen coordinates
  int32_t                       height;                           //!<  Height of object in screen coordinates
  int32_t                       start_angle;                      //!<  Start angle of object
  int32_t                       span_angle;                       //!<  Span angle of object
  QColor                        color;                            //!<  Color of object
  QRect                         rect;                             //!<  Definition of rectangle
  QRect                         prev_rect;                        //!<  Definition of previous rectangle (if it moved)
  int32_t                       x0;                               //!<  Starting X position of a line segment
  int32_t                       y0;                               //!<  Starting Y position of a line segment
  int32_t                       x1;                               //!<  Ending X position of a line segment
  int32_t                       y1;                               //!<  Ending Y position of a line segment
  int32_t                       line_width;                       //!<  Line width in pixels
  QPainterPath                  path;                             //!<  QPainterPath object
  double                        rotation_angle;                   //!<  Rotation angle (usually for text)
  QString                       text;                             //!<  Text
  uint8_t                       clear;                            //!<  Clear before drawing (usually for text)
} NVMAP_MOVABLE_OBJECT;


class nvMap:public QWidget
{
  Q_OBJECT 


public:

  nvMap (QWidget *parent = 0, NVMAP_DEF *mapdef = 0);
  ~nvMap ();


  //  Map functions
 
  int32_t getZoomLevel ();
  void setZoomLevel (int32_t level);
  void setZoomPercent (int32_t percent);
  void setOverlapPercent (int32_t percent);
  void stopDrawing ();
  void zoomInPercent ();
  void zoomInPercent (uint8_t setLevel);
  void zoomOutPercent ();
  void zoomIn (NV_F64_XYMBR bounds);
  void zoomIn (NV_F64_XYMBR bounds, uint8_t setLevel);
  void zoomOut ();
  void setCoastColor (QColor color);
  void setMaskColor (QColor color);
  void setProjection (int32_t projection);
  void setBackgroundColor (QColor color);
  void setGridColor (QColor color);
  void setGridSpacing (double grid_inc_x, double grid_inc_y);
  void setGridThickness (int32_t thickness);
  void resetBounds (NV_F64_XYMBC bounds);
  void resetBounds (NV_F64_XYMBR bounds);
  void setCoasts (int32_t set);
  void setLandmask (uint8_t set);
  void setCoastThickness (int32_t thickness);
  NVMAP_DEF getMapdef ();
  void moveMap (int32_t direction);
  void moveMap (double map_x, double map_y);
  void redrawMapArea (uint8_t clear);
  void redrawMap (uint8_t clear);



  //  Rubberband functions

  void anchorRubberbandPolygon (int32_t *num, double map_x, double map_y, QColor color, int32_t line_width, uint8_t close,
				Qt::PenStyle penStyle);
  void anchorRubberbandPolygon (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, uint8_t close,
				Qt::PenStyle penStyle);
  void dragRubberbandPolygon (int32_t num, double map_x, double map_y);
  void dragRubberbandPolygon (int32_t num, int32_t x, int32_t y);
  void getRubberbandPolygonCurrentAnchor (int32_t num, double *map_x, double *map_y);
  void vertexRubberbandPolygon (int32_t num, double map_x, double map_y);
  void vertexRubberbandPolygon (int32_t num, int32_t x, int32_t y);
  void getRubberbandPolygon (int32_t num, int32_t *count, int32_t **px, int32_t **py, double **mx,
                             double **my);
  void closeRubberbandPolygon (int32_t num, double map_x, double map_y, int32_t *count, 
                               int32_t **px, int32_t **py, double **mx, double **my);
  void closeRubberbandPolygon (int32_t num, int32_t x, int32_t y, int32_t *count, int32_t **px, int32_t **py, 
                               double **mx, double **my);
  uint8_t rubberbandPolygonIsActive (int32_t num);
  void discardRubberbandPolygon (int32_t *num);

  void anchorRubberbandRectangle (int32_t *num, double map_x, double map_y, QColor color, int32_t line_width,
				  Qt::PenStyle penStyle);
  void anchorRubberbandRectangle (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, Qt::PenStyle penStyle);
  void dragRubberbandRectangle (int32_t num, double map_x, double map_y);
  void dragRubberbandRectangle (int32_t num, int32_t x, int32_t y);
  void getRubberbandRectangle (int32_t num, int32_t **px, int32_t **py, double **mx, double **my);
  void closeRubberbandRectangle (int32_t num, double map_x, double map_y, int32_t **px, 
                                 int32_t **py, double **mx, double **my);
  void closeRubberbandRectangle (int32_t num, int32_t x, int32_t y, int32_t **px, int32_t **py, 
                                 double **mx, double **my);
  uint8_t rubberbandRectangleIsActive (int32_t num);
  void discardRubberbandRectangle (int32_t *num);
  void getRubberbandRectangleAnchor (int32_t num, double *map_x, double *map_y);

  void anchorRubberbandLine (int32_t *num, double map_x, double map_y, QColor color, int32_t line_width, 
			     Qt::PenStyle penStyle);
  void anchorRubberbandLine (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, Qt::PenStyle penStyle);
  void dragRubberbandLine (int32_t num, double map_x, double map_y);
  void dragRubberbandLine (int32_t num, int32_t x, int32_t y);
  uint8_t rubberbandLineIsActive (int32_t num);
  void discardRubberbandLine (int32_t *num);


  //  Moving object functions

  void setMovingRectangle (int32_t *num, double map_x0, double map_y0, double map_x1, double map_y1, 
                           QColor color, int32_t line_width, uint8_t filled, Qt::PenStyle penStyle);
  void setMovingRectangle (int32_t *num, int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color,
                           int32_t line_width, uint8_t filled, Qt::PenStyle penStyle);
  void getMovingRectangle (int32_t num, int32_t **px, int32_t **py, double **mx, double **my);
  void closeMovingRectangle (int32_t *num);
  void setMovingPolygon (int32_t *num, int32_t count, double map_x[], double map_y[], QColor color,
                         int32_t line_width, uint8_t filled, Qt::PenStyle penStyle);
  void setMovingPolygon (int32_t *num, int32_t count, int32_t *x, int32_t *y, QColor color, int32_t line_width,
                         uint8_t filled, Qt::PenStyle penStyle);
  void getMovingPolygon (int32_t num, int32_t *count, int32_t **px, int32_t **py, double **mx, double **my);
  void closeMovingPolygon (int32_t *num);
  void setMovingPolyline (int32_t *num, int32_t count, double map_x[], double map_y[], QColor color,
                          int32_t line_width, Qt::PenStyle penStyle);
  void setMovingPolyline (int32_t *num, int32_t count, int32_t *x, int32_t *y, QColor color, int32_t line_width,
                          Qt::PenStyle penStyle);
  void getMovingPolyline (int32_t num, int32_t *count, int32_t **px, int32_t **py, double **mx, double **my);
  void closeMovingPolyline (int32_t *num);
  void setMovingCircle (int32_t *num, double map_x, double map_y, int32_t d, float start_angle, 
                        float span_angle, int32_t line_width, QColor color, uint8_t filled, Qt::PenStyle penStyle);
  void setMovingCircle (int32_t *num, double map_x, double map_y, double end_map_x, double end_map_y, 
                        float start_angle, float span_angle, int32_t line_width, QColor color, uint8_t filled,
                        Qt::PenStyle penStyle);
  void setMovingCircle (int32_t *num, int32_t x, int32_t y, int32_t d, float start_angle, float span_angle,
                        int32_t line_width, QColor color, uint8_t filled, Qt::PenStyle penStyle);
  void getMovingCircle (int32_t num, double *map_x, double *map_y, int32_t *x, int32_t *y, int32_t *d);
  void closeMovingCircle (int32_t *num);
  void setMovingPath (int32_t *num, QPainterPath path, double map_center_x, double map_center_y,
                      int32_t line_width, QColor color, QBrush brush, uint8_t filled, double angle,
                      Qt::PenStyle penStyle);
  void setMovingPath (int32_t *num, QPainterPath path, double map_center_x, double map_center_y,
                      int32_t line_width, QColor color, QBrush brush, uint8_t filled, Qt::PenStyle penStyle);
  void setMovingPath (int32_t *num, QPainterPath path, int32_t center_x, int32_t center_y, int32_t line_width,
                      QColor color, QBrush brush, uint8_t filled, double angle, Qt::PenStyle penStyle);
  void setMovingPath (int32_t *num, QPainterPath path, int32_t center_x, int32_t center_y, int32_t line_width,
                      QColor color, QBrush brush, uint8_t filled, Qt::PenStyle penStyle);
  void closeMovingPath (int32_t *num);
  void setMovingText (int32_t *num, QString text, double map_center_x, double map_center_y, QColor color, double angle,
                      int32_t size);
  void setMovingText (int32_t *num, QString text, int32_t center_x, int32_t center_y, QColor color, double angle, int32_t size);
  void closeMovingText (int32_t *num);


  //  Drawing functions

  void drawText (QString string, double map_x, double map_y, double angle, int32_t size, QColor color,
                 uint8_t flush);
  void drawTextClear (QString string, double map_x, double map_y, double angle, int32_t size, QColor color,
                      uint8_t clear, uint8_t flush);
  void drawText (QString string, int32_t x, int32_t y, double angle, int32_t size, QColor color, uint8_t flush);
  void drawTextClear (QString string, int32_t x, int32_t y, double angle, int32_t size, QColor color, uint8_t clear,
                      uint8_t flush);
  void drawText (QString string, double map_x, double map_y, QColor color, uint8_t flush);
  void drawText (QString string, int32_t x, int32_t y, QColor color, uint8_t flush);
  void setTextFont (QString string, int32_t weight);
  void drawLine (double map_x0, double map_y0, double map_x1, double map_y1, QColor color,
                 int32_t line_width, uint8_t flush, Qt::PenStyle penStyle);
  void drawLine (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width, uint8_t flush,
                 Qt::PenStyle penStyle);
  void fillCircle (double map_x, double map_y, int32_t d, float start_angle,
                   float span_angle, QColor color, uint8_t flush);
  void fillCircle (double map_x, double map_y, double end_map_x, double end_map_y, 
                   float start_angle, float span_angle, QColor color, uint8_t flush);
  void fillCircle (int32_t x, int32_t y, int32_t d, float start_angle, float span_angle,
                   QColor color, uint8_t flush);
  void drawCircle (double map_x, double map_y, int32_t d, float start_angle,
                   float span_angle, int32_t line_width, QColor color, Qt::PenStyle penStyle, uint8_t flush);
  void drawCircle (double map_x, double map_y, double end_map_x, double end_map_y, 
                   float start_angle, float span_angle, int32_t line_width, QColor color,
                   Qt::PenStyle penStyle, uint8_t flush);
  void drawCircle (int32_t x, int32_t y, int32_t d, float start_angle, float span_angle,
                   int32_t line_width, QColor color, Qt::PenStyle penStyle, uint8_t flush);
  void fillRectangle (double map_x, double map_y, int32_t w, int32_t h, QColor color, uint8_t flush);
  void fillRectangle (double map_x, double map_y, double end_map_x, double end_map_y, QColor color,
                      uint8_t flush);
  void fillRectangle (int32_t x, int32_t y, int32_t w, int32_t h, QColor color, uint8_t flush);
  void drawRectangle (double x0, double y0, double x1, double y1, QColor color, int32_t line_width,
                      Qt::PenStyle penStyle, uint8_t flush);
  void drawRectangle (double map_x0, double map_y0, int32_t w, int32_t h,
                      QColor color, int32_t line_width, Qt::PenStyle penStyle, uint8_t flush);
  void drawRectangle (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width,
                      Qt::PenStyle penStyle, uint8_t flush);
  void fillPolygon (int32_t count, double map_x[], double map_y[], QColor color, uint8_t flush);
  void fillPolygon (int32_t count, NV_F64_COORD2 map_xy[], QColor color, uint8_t flush);
  void fillPolygon (int32_t count, int32_t x[], int32_t y[], QColor color, uint8_t flush);
  void fillPolygon (int32_t count, NV_I32_COORD2 xy[], QColor color, uint8_t flush);
  void drawPolygon (int32_t count, double map_x[], double map_y[], QColor color, int32_t line_width,
                    uint8_t close, Qt::PenStyle penStyle, uint8_t flush);
  void drawPolygon (int32_t count, NV_F64_COORD2 map_xy[], QColor color, int32_t line_width,
                    uint8_t close, Qt::PenStyle penStyle, uint8_t flush);
  void drawPolygon (int32_t count, int32_t x[], int32_t y[], QColor color, int32_t line_width, uint8_t close, 
                    Qt::PenStyle penStyle, uint8_t flush);
  void drawPolygon (int32_t count, NV_I32_COORD2 xy[], QColor color, int32_t line_width, uint8_t close, 
                    Qt::PenStyle penStyle, uint8_t flush);
  void drawPath (QPainterPath path, double map_center_x, double map_center_y, int32_t line_width, QColor color,
                 QBrush brush, uint8_t filled, Qt::PenStyle penStyle, uint8_t flush);
  void drawPath (QPainterPath path, int32_t center_x, int32_t center_y, int32_t line_width, QColor color,
                 QBrush brush, uint8_t filled, Qt::PenStyle penStyle, uint8_t flush);
  void drawPixmap (int32_t x, int32_t y, QPixmap *src, int32_t src_x, int32_t src_y, 
                   int32_t src_w, int32_t src_h, uint8_t flush);
  QPixmap grabPixmap (int32_t x, int32_t y, int32_t w, int32_t h);
  QPixmap grabPixmap ();


  //  Utility functions

  void map_to_screen (int32_t num_points, double map_x[], double map_y[], 
                      double map_z[], double pix_x[], double pix_y[], double pix_z[]);
  void map_to_screen (int32_t num_points, double map_x[], double map_y[], 
                      double map_z[], int32_t pix_x[], int32_t pix_y[], int32_t pix_z[]);
  void screen_to_map (int32_t num_points, double map_x[], double map_y[], double map_z[],
                      int32_t pix_x[], int32_t pix_y[], int32_t pix_z[]);
  void screen_to_map (int32_t num_points, double map_x[], double map_y[], double map_z[],
                      double pix_x[], double pix_y[], double pix_z[]);
  uint8_t checkBounds (double map_x, double map_y);
  void getMBR (double *map_x, double *map_y, int32_t num_points, NV_F64_XYMBR *mbr);
  void setZeroToThreeSixty (uint8_t enable);
  void setNoBoundsLimit (uint8_t enable);
  int32_t clipLine (double map_x1, double map_y1, double map_x2, double map_y2, 
                     double *ret_x1, double *ret_y1, double *ret_x2, double *ret_y2);
  void enableSignals ();
  void disableSignals ();
  void enableMouseSignals ();
  void disableMouseSignals ();
  void flush ();
  void dummyBuffer (int32_t width, int32_t height, uint8_t start);


signals:

  void enterSignal (QEvent *e);
  void leaveSignal (QEvent *e);
  void mousePressSignal (QMouseEvent *e, double map_x, double map_y);
  void mouseDoubleClickSignal (QMouseEvent *e, double map_x, double map_y);
  void preliminaryMousePressSignal (QMouseEvent *e);
  void mouseReleaseSignal (QMouseEvent *e, double map_x, double map_y);
  void mouseMoveSignal (QMouseEvent *e, double map_x, double map_y);
  void mouseWheelSignal (QWheelEvent *e);
  void closeSignal (QCloseEvent *e);
  void focusInSignal (QFocusEvent *e);
  void focusOutSignal (QFocusEvent *e);
  void resizeSignal (QResizeEvent *e);
  void preRedrawSignal (NVMAP_DEF map);
  void midRedrawSignal (NVMAP_DEF map);
  void postRedrawSignal (NVMAP_DEF map);

protected:

  void enterEvent (QEvent *e);
  void leaveEvent (QEvent *e);
  void boundsCheck (double *map_x, double *map_y);
  void checkDateline (NV_F64_XYMBR *mbr);
  void mouseDoubleClickEvent (QMouseEvent *e);
  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  void mouseMoveEvent (QMouseEvent *e);
  void wheelEvent (QWheelEvent *e);
  void resizeEvent (QResizeEvent *e);
  void paintEvent (QPaintEvent *e);
  void closeEvent (QCloseEvent *e);
  void focusInEvent (QFocusEvent *e);
  void focusOutEvent (QFocusEvent *e);
  void redrawMapWithCheck (uint8_t clear);

  void init_proj (int32_t x, int32_t y, int32_t width, int32_t height, int32_t border, 
                  double xmin, double ymin, double xmax, double ymax);

  int32_t lineIntersection (double x1, double y1, double x2, double y2,
                             double x3, double y3, double x4, double y4,
                             double *x, double *y);

  int32_t create_movable_object (NVMAP_MOVABLE_OBJECT mo);
  void discard_movable_object (int32_t *num);


  rproj                 *proj;

  int16_t               last_map_called;

  uint8_t               invertY;

  uint8_t               stopFlag;

  QPixmap               buffer;

  QPainter              painter;

  NVMAP_MOVABLE_OBJECT  movable_object[MAX_MOVABLE_OBJECTS];
  NVMAP_MOVABLE_OBJECT  *ot[MAX_MOVABLE_OBJECTS];
  int32_t               movable_object_count;

  uint8_t               rubberbandLineActive[MAX_MOVABLE_OBJECTS];
  int32_t               rubberbandLineAnchorX[MAX_MOVABLE_OBJECTS], rubberbandLineAnchorY[MAX_MOVABLE_OBJECTS],
                        rubberbandLineLastX[MAX_MOVABLE_OBJECTS], rubberbandLineLastY[MAX_MOVABLE_OBJECTS];


  uint8_t               rubberbandPolyActive[MAX_MOVABLE_OBJECTS];
  int32_t               rubberbandPolyLastX[MAX_MOVABLE_OBJECTS], rubberbandPolyLastY[MAX_MOVABLE_OBJECTS];
  double                rubberbandPolyMapX[MAX_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS], 
                        rubberbandPolyMapY[MAX_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS];


  uint8_t               rubberbandRectActive[MAX_MOVABLE_OBJECTS];
  int32_t               rubberbandRectX[MAX_MOVABLE_OBJECTS][4], rubberbandRectY[MAX_MOVABLE_OBJECTS][4],
                        rubberbandRectLastX[MAX_MOVABLE_OBJECTS], rubberbandRectLastY[MAX_MOVABLE_OBJECTS];
  double                rubberbandRectMapX[MAX_MOVABLE_OBJECTS][4], rubberbandRectMapY[MAX_MOVABLE_OBJECTS][4];


  uint8_t               signalsEnabled, mouseSignalsEnabled, dateline, zero_to_360, no_bounds_limit, dummy_buffer;
  int32_t               dum_z[NVMAP_POLYGON_POINTS];
  double                dum_map_z[NVMAP_POLYGON_POINTS];

  NVMAP_DEF             map;
};

#endif
