
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



/*  nvPic class definitions.  */

#ifndef __NVPIC_H__
#define __NVPIC_H__

#include <cmath>


#include "pfm_nvtypes.h"
#include "nvdef.h"


#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include "fixpos.h"


#define PIC_X_SIZE             800
#define PIC_Y_SIZE             675
#define NVPIC_LEFT             0
#define NVPIC_UP               1
#define NVPIC_RIGHT            2
#define NVPIC_DOWN             3


#define NVPIC_POLYGON_POINTS   1000
#define NVPIC_ZOOM_LEVELS      100


typedef struct
{
  int32_t min_y;
  int32_t min_x;
  int32_t max_y;
  int32_t max_x;
} NV_I32_XYMBR;


typedef struct
{
  NV_I32_XYMBR initial_bounds;             //!<  Initial x,y boundaries
  NV_I32_XYMBR bounds[NVPIC_ZOOM_LEVELS];  //!<  All zoom bounds
  int32_t zoom_level;                     //!<  Zoom level
  float zoom_percent;                 //!<  Zoom in/out percentage (0.0 - 1.0)
  int32_t draw_width;                     //!<  Drawing area width in pixels
  int32_t draw_height;                    //!<  Drawing area height in pixels
  int32_t width;                          //!<  Pic width in pixels
  int32_t height;                         //!<  Pic height in pixels
  Qt::TransformationMode transMode;        //!<  Transformation mode for rotation and scaling (smooth or fast)
} NVPIC_DEF;



//  FLOATING_OBJECT types

#define NVPIC_MOVE_POLYGON       0
#define NVPIC_MOVE_RECTANGLE     1
#define NVPIC_MOVE_CIRCLE        2
#define NVPIC_MOVE_PATH          3
#define NVPIC_RUBBER_POLYGON     4
#define NVPIC_RUBBER_RECTANGLE   5
#define NVPIC_RUBBER_LINE        6

//  FLOATING_OBJECT states

#define NVPIC_OBJECT_NOT_PRESENT 0
#define NVPIC_OBJECT_ACTIVE      1
#define NVPIC_OBJECT_CLEAR       2

typedef struct
{
  int32_t type;                 // See above
  int32_t state;                // See above
  uint8_t filled;
  uint8_t outlined;
  uint8_t close_polygon;
  QBrush brush;
  int32_t polygon_x[NVPIC_POLYGON_POINTS], polygon_y[NVPIC_POLYGON_POINTS], polygon_count;
  int32_t x, y, width, height, start_angle, span_angle;
  QColor color;
  QRect rect, prev_rect;
  int32_t x0, y0, x1, y1;
  int32_t line_width;
  QPainterPath path;
  double rotation_angle;
} NVPIC_FLOATING_OBJECT;


class nvPic:public QWidget
{
  Q_OBJECT 


public:

  nvPic (QWidget *parent = 0, NVPIC_DEF *picdef = 0);
  ~nvPic ();

  void setTransformationMode (Qt::TransformationMode mode);
  void setZoomLevel (int32_t level);
  void setZoomPercent (float percent);
  void zoomInPercent ();
  void zoomOutPercent ();
  void zoomIn (NV_I32_XYMBR bounds);
  void zoomOut ();
  void resetBounds (NV_I32_XYMBR bounds);
  NVPIC_DEF getPicdef ();
  void redrawPic ();

  void brighten (int32_t percentage);
  uint8_t OpenFile (char *filename);
  uint8_t OpenData (uint8_t *data, uint32_t len);
  void SaveFile (QString filename, uint8_t scaled);
  void RotatePixmap (double angle);

  void imagePaint (int32_t x, int32_t y, QImage *src, int32_t src_x, int32_t src_y, int32_t src_w, int32_t src_h);
  void pixmapPaint (int32_t x, int32_t y, QPixmap *src, int32_t src_x, int32_t src_y, int32_t src_w, int32_t src_h);
  QPixmap grabPixmap (int32_t x, int32_t y, int32_t w, int32_t h);

  void anchorRubberbandPolygon (int32_t pic_x, int32_t pic_y, QColor color, int32_t line_width, uint8_t close);
  void anchorRubberbandPolygon (int32_t x, int32_t y, QColor color, int32_t line_width, uint8_t close, int8_t q);
  void dragRubberbandPolygon (int32_t pic_x, int32_t pic_y);
  void dragRubberbandPolygon (int32_t x, int32_t y, int8_t q);
  void getRubberbandPolygonCurrentAnchor (int32_t *pic_x, int32_t *pic_y);
  void vertexRubberbandPolygon (int32_t pic_x, int32_t pic_y);
  void vertexRubberbandPolygon (int32_t x, int32_t y, int8_t q);
  void getRubberbandPolygon (int32_t *count, int32_t **px, int32_t **py, int32_t **mx, int32_t **my);
  void closeRubberbandPolygon (int32_t pic_x, int32_t pic_y, int32_t *count, 
                               int32_t **px, int32_t **py, int32_t **mx, int32_t **my);
  void closeRubberbandPolygon (int32_t x, int32_t y, int32_t *count, int32_t **px, int32_t **py, 
                               int32_t **mx, int32_t **my, int8_t q);
  uint8_t rubberbandPolygonIsActive ();
  void discardRubberbandPolygon ();

  void anchorRubberbandRectangle (int32_t pic_x, int32_t pic_y, QColor color, int32_t line_width);
  void anchorRubberbandRectangle (int32_t x, int32_t y, QColor color, int32_t line_width, int8_t q);
  void dragRubberbandRectangle (int32_t pic_x, int32_t pic_y);
  void dragRubberbandRectangle (int32_t x, int32_t y, int8_t q);
  void getRubberbandRectangle (int32_t **px, int32_t **py, int32_t **mx, int32_t **my);
  void closeRubberbandRectangle (int32_t pic_x, int32_t pic_y, int32_t **px, 
                                 int32_t **py, int32_t **mx, int32_t **my);
  void closeRubberbandRectangle (int32_t x, int32_t y, int32_t **px, int32_t **py, 
                                 int32_t **mx, int32_t **my, int8_t q);
  uint8_t rubberbandRectangleIsActive ();
  uint8_t rubberbandRectangleIsPresent ();
  void discardRubberbandRectangle ();
  void getRubberbandRectangleAnchor (int32_t *pic_x, int32_t *pic_y);

  void anchorRubberbandLine (int32_t pic_x, int32_t pic_y, QColor color, int32_t line_width);
  void anchorRubberbandLine (int32_t x, int32_t y, QColor color, int32_t line_width, int8_t q);
  void dragRubberbandLine (int32_t pic_x, int32_t pic_y);
  void dragRubberbandLine (int32_t x, int32_t y, int8_t q);
  uint8_t rubberbandLineIsActive ();
  void discardRubberbandLine ();


  void setMovingRectangle (int32_t pic_x0, int32_t pic_y0, int32_t pic_x1, int32_t pic_y1, 
                           QColor color, int32_t line_width, uint8_t filled);
  void setMovingRectangle (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width,
                           uint8_t filled, int8_t q);
  void getMovingRectangle (int32_t **px, int32_t **py, int32_t **mx, int32_t **my);
  void closeMovingRectangle ();
  void setMovingPolygon (int32_t count, int32_t pic_x[], int32_t pic_y[], QColor color, int32_t line_width,
                         uint8_t filled);
  void setMovingPolygon (int32_t count, int32_t *x, int32_t *y, QColor color, int32_t line_width, uint8_t filled,
                         int8_t q);
  void getMovingPolygon (int32_t *count, int32_t **px, int32_t **py, int32_t **mx, int32_t **my);
  void closeMovingPolygon ();
  void setMovingCircle (int32_t pic_x, int32_t pic_y, int32_t d, float start_angle, 
                        float span_angle, int32_t line_width, QColor color, uint8_t filled);
  void setMovingCircle (int32_t pic_x, int32_t pic_y, int32_t end_pic_x, int32_t end_pic_y, 
                        float start_angle, float span_angle, int32_t line_width, QColor color, uint8_t filled);
  void setMovingCircle (int32_t x, int32_t y, int32_t d, float start_angle, float span_angle,
                        int32_t line_width, QColor color, uint8_t filled, int8_t q);
  void getMovingCircle (int32_t *pic_x, int32_t *pic_y, int32_t *x, int32_t *y, int32_t *d);
  void closeMovingCircle ();
  void setMovingPath (QPainterPath path, int32_t pic_center_x, int32_t pic_center_y, int32_t line_width,
                      QColor color, QBrush brush, uint8_t filled, double angle);
  void setMovingPath (QPainterPath path, int32_t pic_center_x, int32_t pic_center_y, int32_t line_width,
                      QColor color, QBrush brush, uint8_t filled);
  void setMovingPath (QPainterPath path, int32_t center_x, int32_t center_y, int32_t line_width, QColor color,
                      QBrush brush, uint8_t filled, double angle, int8_t q);
  void closeMovingPath ();


  void drawText (QString string, int32_t pic_x, int32_t pic_y, double angle, int32_t size, QColor color);
  void drawText (QString string, int32_t x, int32_t y, double angle, int32_t size, QColor color, int8_t q);
  void drawText (QString string, int32_t pic_x, int32_t pic_y, QColor color);
  void drawText (QString string, int32_t x, int32_t y, QColor color, int8_t q);
  void drawLine (int32_t pic_x0, int32_t pic_y0, int32_t pic_x1, int32_t pic_y1, QColor color,
                 int32_t line_width, uint8_t flush);
  void drawLine (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width, uint8_t flush,
                 int8_t q);
  void fillCircle (int32_t pic_x, int32_t pic_y, int32_t d, float start_angle,
                   float span_angle, QColor color);
  void fillCircle (int32_t pic_x, int32_t pic_y, int32_t end_pic_x, int32_t end_pic_y, 
                   float start_angle, float span_angle, QColor color);
  void fillCircle (int32_t x, int32_t y, int32_t d, float start_angle, float span_angle,
                   QColor color, int8_t q);
  void drawCircle (int32_t pic_x, int32_t pic_y, int32_t d, float start_angle,
                   float span_angle, int32_t line_width, QColor color);
  void drawCircle (int32_t pic_x, int32_t pic_y, int32_t end_pic_x, int32_t end_pic_y, 
                   float start_angle, float span_angle, int32_t line_width, QColor color);
  void drawCircle (int32_t x, int32_t y, int32_t d, float start_angle, float span_angle,
                   int32_t line_width, QColor color, int8_t q);
  void fillRectangle (int32_t pic_x, int32_t pic_y, int32_t w, int32_t h, QColor color, uint8_t flush);
  void fillRectangle (int32_t x, int32_t y, int32_t w, int32_t h, QColor color, uint8_t flush, int8_t q);
  void drawRectangle (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width);
  void drawRectangle (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width, int8_t q);
  void fillPolygon (int32_t count, int32_t pic_x[], int32_t pic_y[], QColor color);
  void fillPolygon (int32_t count, int32_t x[], int32_t y[], QColor color, int8_t q);
  void drawPolygon (int32_t count, int32_t pic_x[], int32_t pic_y[], QColor color, int32_t line_width);
  void drawPolygon (int32_t count, int32_t x[], int32_t y[], QColor color, int32_t line_width, int8_t q);
  void drawPath (QPainterPath path, int32_t pic_center_x, int32_t pic_center_y, int32_t line_width, QColor color,
                 QBrush brush, uint8_t filled);
  void drawPath (QPainterPath path, int32_t center_x, int32_t center_y, int32_t line_width, QColor color,
                 QBrush brush, uint8_t filled, int8_t q);

  void pic_to_screen (int32_t num_points, int32_t pic_x[], int32_t pic_y[], int32_t pix_x[], int32_t pix_y[]);
  void screen_to_pic (int32_t num_points, int32_t pic_x[], int32_t pic_y[], int32_t pix_x[], int32_t pix_y[]);
  void enableSignals ();
  void disableSignals ();
  void flush ();


  signals:

  void mousePressSignal (QMouseEvent *e, int32_t pic_x, int32_t pic_y);
  void mouseDoubleClickSignal (QMouseEvent *e, int32_t pic_x, int32_t pic_y);
  void preliminaryMousePressSignal (QMouseEvent *e);
  void mouseReleaseSignal (QMouseEvent *e, int32_t pic_x, int32_t pic_y);
  void mouseMoveSignal (QMouseEvent *e, int32_t pic_x, int32_t pic_y);
  void mouseWheelSignal (QWheelEvent *e);
  void closeSignal (QCloseEvent *e);
  void focusInSignal (QFocusEvent *e);
  void focusOutSignal (QFocusEvent *e);
  void resizeSignal (QResizeEvent *e);
  void keyPressSignal (QKeyEvent *e);
  void postRedrawSignal (NVPIC_DEF pic);

protected:

  void boundsCheck (int32_t *pic_x, int32_t *pic_y);
  void mouseDoubleClickEvent (QMouseEvent *e);
  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  void mouseMoveEvent (QMouseEvent *e);
  void wheelEvent (QWheelEvent *e);
  void keyPressEvent (QKeyEvent *e);
  void resizeEvent (QResizeEvent *e);
  void paintEvent (QPaintEvent *e);
  void closeEvent (QCloseEvent *e);
  void focusInEvent (QFocusEvent *e);
  void focusOutEvent (QFocusEvent *e);


  QMatrix               matrix, trueMatrix, invertedMatrix;

  double                rotation;
  uint8_t               rotate, box_cleared, pic_loaded;
  float                 xrange, yrange, scale_min_x_scale, scale_min_y_scale;

  QPixmap               buffer, *orig_pixmap, *scaled_pixmap;
  QPainter              painter;
  NVPIC_FLOATING_OBJECT floating_object;

  uint8_t               rubberbandLineActive;
  int32_t               rubberbandLineAnchorX, rubberbandLineAnchorY, rubberbandLineLastX, rubberbandLineLastY;


  uint8_t               rubberbandPolyActive;
  int32_t               rubberbandPolyLastX, rubberbandPolyLastY;
  int32_t               rubberbandPolyPicX[NVPIC_POLYGON_POINTS], rubberbandPolyPicY[NVPIC_POLYGON_POINTS];


  uint8_t               rubberbandRectActive, rubberbandRectPresent;
  int32_t               rubberbandRectCount, rubberbandRectX[4], rubberbandRectY[4], rubberbandRectLastX, 
                        rubberbandRectLastY;
  int32_t               rubberbandRectPicX[4], rubberbandRectPicY[4];


  uint8_t               signalsEnabled;

  NVPIC_DEF             pic;
};

#endif
