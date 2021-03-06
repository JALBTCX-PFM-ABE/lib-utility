
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



/*  nvMapGL class definitions.  */

#ifndef __NVMAPGL_H__
#define __NVMAPGL_H__

#include <cmath>


#include "nvutility.h"
#include "nvutility.hpp"


#include <stdio.h>
#include <stdlib.h>
#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QGLWidget>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include "squat.hpp"


#define NVMAPGL_LEFT             0
#define NVMAPGL_UP               1
#define NVMAPGL_RIGHT            2
#define NVMAPGL_DOWN             3
#define NVMAPGL_UP_LEFT          4
#define NVMAPGL_UP_RIGHT         5
#define NVMAPGL_DOWN_LEFT        6
#define NVMAPGL_DOWN_RIGHT       7

#define NVMAPGL_POLYGON_POINTS   2000
#define NVMAPGL_ZOOM_LEVELS      100


#define DATA_COLORS 128


typedef struct
{
  int32_t projected;                       //!<  0 - not projected (lat/lon), 1 - UTM (meters)
  NV_F64_XYMBC initial_bounds;             //!<  Initial x,y,z boundaries
  float initial_zx_rotation;               //!<  Initial ZX rotation setting
  float initial_y_rotation;                //!<  Initial Y rotation setting
  int32_t zoom_percent;                    //!<  Zoom in/out percentage (0 - 100)
  float exaggeration;                      //!<  Z exaggeration value (which is inverted to get the exag_scale)
  uint8_t auto_scale;                      //!<  Set if we are auto scaling (usually not set for NVMAPGL_BIN_MODE)
  float center_x;                          //!<  Center x value
  uint8_t draw_scale;                      //!<  Set to draw a "box scale"
  float scale_offset;                      //!<  Set to a value other than 0.0 if we want the "box scale" numbers offset and drawn in red
  float scale_factor;                      //!<  Set to a value other than 1.0 if we want the "box scale" numbers scaled by a factor
  int32_t draw_width;                      //!<  Drawing area width in pixels
  int32_t draw_height;                     //!<  Drawing area height in pixels
  int32_t width;                           //!<  Map width in pixels
  int32_t height;                          //!<  Map height in pixels
  uint8_t coasts;                          //!<  Coastlines on or off
  uint8_t coast_thickness;                 //!<  Coast line thickness
  QColor coast_color;                      //!<  Coastline color
  QColor scale_color;                      //!<  Color to be used for "box scale"
  QColor scale_offset_color;               //!<  Color to be used for "box scale" numbers if scale_offset or scale_factor is set (to other than 0.0 or 1.0)
  uint8_t landmask;                        //!<  Landmask on or off
  QColor landmask_color;                   //!<  Landmask color
  QColor background_color;                 //!<  Background color
  int32_t mode;                            /*!<  0 - layer mode, 1 - point mode, 2 - feature mode.  These define
                                                 what information will be handed back via signal.  Either
                                                 the nearest bin vertex, data point, or feature respectively.  */
  float zoom_factor;                       //!<  Amount of current zoom
  float min_z_extents;                     //!<  Minimum Z extents for Z auto scaling
  GLenum light_model;                      //!<  Either GL_LIGHT_MODEL_AMBIENT or GL_LIGHT_MODEL_DIFFUSE;
  int32_t renderMode;                      //!<  Either NVMAPGL_RENDER_POINT_MODE or NVMAPGL_RENDER_SPHERE_MODE)
  float complexObjectWidth;                //!<  Width of the object to be used for the point/sparse data
  int32_t complexDivisionals;
} NVMAPGL_DEF;



//  MOVABLE_OBJECT types

#define NVMAPGL_MOVE_POLYGON       0
#define NVMAPGL_MOVE_POLYLINE      1
#define NVMAPGL_MOVE_RECTANGLE     2
#define NVMAPGL_MOVE_CIRCLE        3
#define NVMAPGL_MOVE_LIST          4
#define NVMAPGL_RUBBER_POLYGON     5
#define NVMAPGL_RUBBER_RECTANGLE   6
#define NVMAPGL_RUBBER_LINE        7


//  Modes.  Used for finding the nearest point and to determine whether to display the full point cloud
//  or the decimated point cloud.

#define NVMAPGL_BIN_MODE           0   //!<  For layers (PFM bins)
#define NVMAPGL_POINT_MODE         1   //!<  For full point cloud.
#define NVMAPGL_SPARSE_MODE        2   //!<  For decimated point cloud
#define NVMAPGL_FEATURE_MODE       3   //!<  For features


//  Number of available movable objects

#define MAX_GL_MOVABLE_OBJECTS     20


//  Modes for rendering point cloud

#define NVMAPGL_RENDER_POINT_MODE  0
#define NVMAPGL_RENDER_SPHERE_MODE 2


/*!
    Movable object section.  A little explanation - in order to avoid a ton of if statements in the 
    paint event I've implemented a movable object table (mot).  The object table is just an array of pointers
    to objects in the movable_object array.  This way we can track the number of the object without
    having to check state for each of MAX_GL_MOVABLE_OBJECTS objects in the paint event.  We will have one
    "for" loop in the paint event that will go from 0 to movable_object_count (which equates to
    "movable_object_count" "if" statements).  That is, we're going to have a count of active objects
    and when we delete an object we'll compress the array.  Otherwise we have to do a "for" loop over
    MAX_GL_MOVABLE_OBJECTS (which equates to MAX_GL_MOVABLE_OBJECTS "if" statements) and an "if" per object.
    This would result in 2 * MAX_GL_MOVABLE_OBJECTS "if" statements every time we had a paint event, even
    if we had NO movable_objects.  Confusing, yes, but much more efficient.  We will be accessing
    mot[object_number] (the pointer to the proper movable_object) when we're in the "for" loop or
    elsewhere in the code.  A pointer of NULL in the object table means the object has been discarded
    and is available for re-use.  I'm sure I could have done this using QGraphicsView but, since I don't
    have the entire scene visible (or in memory) at any given time, it would have been less efficient.
    For most of my stuff I'll only be handling a small number of movable objects.
*/

typedef struct
{
  int32_t                       type;        //!<  Type of object
  uint8_t                       active;      //!<  Whether the object is active
  uint8_t                       filled;      //!<  Whether the object is filled
  uint8_t                       outlined;    //!<  Whether the object is outlined
  uint8_t                       close_polygon; //!<  Whether to automatically close the polygon
  QBrush                        brush;       //!<  Brush to use to fill object
  GLushort                      stipple;
  double                        polygon[NVMAPGL_POLYGON_POINTS][3];  //!<  Not used currently.
  int32_t                       polygon_x[NVMAPGL_POLYGON_POINTS], polygon_y[NVMAPGL_POLYGON_POINTS], polygon_count;
  int32_t                       prev_polygon_x[NVMAPGL_POLYGON_POINTS], prev_polygon_y[NVMAPGL_POLYGON_POINTS], prev_polygon_count;
  int32_t                       width;       //!<  Width of object in screen coordinates
  int32_t                       height;      //!<  Height of object in screen coordinates
  int32_t                       start_angle; //!<  Start angle of object
  int32_t                       span_angle;  //!<  Span angle of object
  GLubyte                       color[4];    //!<  Color of object
  QRect                         rect;        //!<  Definition of rectangle
  QRect                         prev_rect;   //!<  Definition of previous rectangle (if it moved)
  int32_t                       x0;          //!<  Starting X position of a line segment
  int32_t                       y0;          //!<  Starting Y position of a line segment
  int32_t                       x1;          //!<  Ending X position of a line segment
  int32_t                       y1;          //!<  Ending Y position of a line segment
  int32_t                       prev_x0, prev_y0, prev_x1, prev_y1;
  int32_t                       line_width;  //!<  Line width in pixels
  float                         rotation_angle;
  float                         prev_rotation_angle;
  QString                       string;
} NVMAPGL_MOVABLE_OBJECT;


//  Number of available text objects

#define MAX_GL_TEXT_OBJECTS     8000


typedef struct
{
  QString                       string;
  float                         x, y, z;
  float                         color[4];
  float                         angle;
  QFont                         font;
  int32_t                       size;
} NVMAPGL_TEXT_OBJECT;


typedef struct
{
  float                         v[3];      //!<  X, Y, Z
  uint8_t                       color[4];  //!<  Color
  float                         size;      //!<  Size
  int32_t                       slices;
  int32_t                       stacks;
} NVMAPGL_POINT_OBJECT;


typedef struct
{
  float                         v[3];        //!<  X, Y, Z
  uint8_t                       color[4];    //!<  Color
  float                         size;        //!<  Size
} NVMAPGL_MARKER_OBJECT;


typedef struct
{
  std::vector<NV_F32_COORD3>    v;
  uint8_t                       color[4];    //!<  Color
  int32_t                       width;       //!<  Size
  int32_t                       count;
  GLushort                      stipple;
} NVMAPGL_LINE_OBJECT;


typedef struct
{
  int32_t                       x0;
  int32_t                       x1;
  int32_t                       y0;
  int32_t                       y1;
  uint8_t                       color[4];    //!<  Color
  int32_t                       width;
  GLushort                      stipple;
} NVMAPGL_2DLINE_OBJECT;


class nvMapGL:public QGLWidget
{
  Q_OBJECT 


public:

  nvMapGL (QWidget *parent = 0, NVMAPGL_DEF *mapdef = 0, QString pn = NULL);
  ~nvMapGL ();


  void clearData (uint8_t clear_texture);


  //  Map functions

  void get2DCoords (double x, double y, double z, int32_t *px, int32_t *py, uint8_t *init);
  void get2DCoords (double x, double y, float z, int32_t *px, int32_t *py, uint8_t *init);
  void get2DCoords (double x, double y, double z, int32_t *px, int32_t *py);
  void get2DCoords (double x, double y, float z, int32_t *px, int32_t *py);
  void setExaggeration (float value);
  void setMinZExtents (float value);
  void setZoomPercent (int32_t percent);
  //void stopDrawing ();
  void zoomInPercent (int32_t percent);
  void zoomInPercent ();
  void zoomOutPercent (int32_t percent);
  void zoomOutPercent ();
  void resetZoom ();
  void resetPOV ();
  void setBackgroundColor (QColor color);
  void setScaleColor (QColor color);
  void setScaleOffset (float offset);
  void setScaleFactor (float factor);
  void setScaleOffsetColor (QColor color);
  void setScaleOrientation (uint8_t depth);
  void enableScale (uint8_t enable);
  //void resetBounds (NV_F64_XYMBR bounds);
  void setTextureType (int32_t type);
  void enableTexture (uint8_t enable);
  void setAutoScale (uint8_t auto_scale);
  void setBounds (NV_F64_XYMBC total_mbc);
  void setProjected (int32_t projected);

  void clearGeotiffTexture ();
  void setGeotiffTexture (QImage *subImage, NV_F64_XYMBR mbr, int32_t type);

  void clearDataLayer (int32_t layer);
  uint8_t setData2Layer (int32_t layer, float **data, float **color, float color_min, float color_max, 
                         int32_t rows, int32_t cols, double y_grid, double x_grid, NV_F64_XYMBR mbr, int32_t layer_type);
  uint8_t setDataLayer (int32_t layer, float **data, float **color, float color_min, float color_max, int32_t rows, int32_t cols, double x_grid,
                        double y_grid, float null_val, NV_F64_XYMBR mbr, int32_t layer_type);

  void clearDataPoints ();
  uint8_t setDataPoints (double x, double y, double z, uint8_t *point_color, int32_t point_size, uint8_t done);

  void clearSparsePoints ();
  uint8_t setSparsePoints (double x, double y, double z, uint8_t *point_color, int32_t point_size, uint8_t done);

  void clearFeaturePoints ();
  uint8_t setFeaturePoints (double x, double y, double z, QColor feature_color, float feature_size, int32_t slices, int32_t stacks, uint8_t done);

  void clearMarkerPoints ();
  uint8_t setMarkerPoints (double x, double y, double z, QColor marker_color, float marker_size, uint8_t done);

  void clearLines ();
  uint8_t setLines (double x, double y, double z, QColor line_color, int32_t line_width, Qt::PenStyle penStyle, int32_t point_num, uint8_t done);
  uint8_t setLines (float x, float y, float z, QColor line_color, int32_t line_width, Qt::PenStyle penStyle, int32_t point_num, uint8_t done);

  void setRenderMode (int32_t mode); 
  void setRenderModeAmbience (int32_t mode);
  void setComplexObjectWidth (float objWidth);
  void setComplexDivisionals (int32_t divisionals);

  void setMapMode (int32_t mode);
  void enableWindozePaintEvent (uint8_t enable);

  void rotateX (float angle);
  void rotateY (float angle);
  void rotateZ (float angle);
  void rotateZX (float angle);
  void setXRotation (float angle);
  void setYRotation (float angle);
  void setZRotation (float angle);
  void setZXRotation (float angle);
  float getXRotation ();
  float getYRotation ();
  float getZRotation ();
  float getZXRotation ();
  void setMapCenter (double map_x, double map_y, float map_z);
  void setMapCenter (double map_x, double map_y, double map_z);
  void setCenter (float x, float y, float z);
  void getMapCenter (double *map_x, double *map_y, double *map_z);



  //  Rubberband functions

  void anchorRubberbandPolygon (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, uint8_t close, Qt::PenStyle penStyle);
  void dragRubberbandPolygon (int32_t num, int32_t x, int32_t y);
  void getRubberbandPolygonCurrentAnchor (int32_t num, int32_t *x, int32_t *y);
  void vertexRubberbandPolygon (int32_t num, int32_t x, int32_t y);
  void getRubberbandPolygon (int32_t num, int32_t *count, int32_t **px, int32_t **py);
  void closeRubberbandPolygon (int32_t num, int32_t x, int32_t y, int32_t *count, int32_t **px, int32_t **py);
  uint8_t rubberbandPolygonIsActive (int32_t num);
  void discardRubberbandPolygon (int32_t *num);

  void anchorRubberbandRectangle (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, Qt::PenStyle penStyle);
  void dragRubberbandRectangle (int32_t num, int32_t x, int32_t y);
  void getRubberbandRectangle (int32_t num, int32_t **px, int32_t **py);
  void closeRubberbandRectangle (int32_t num, int32_t x, int32_t y, int32_t **px, int32_t **py);
  uint8_t rubberbandRectangleIsActive (int32_t num);
  void discardRubberbandRectangle (int32_t *num);
  void getRubberbandRectangleAnchor (int32_t num, int32_t *px, int32_t *py);

  void anchorRubberbandLine (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, Qt::PenStyle penStyle);
  void dragRubberbandLine (int32_t num, int32_t x, int32_t y);
  uint8_t rubberbandLineIsActive (int32_t num);
  void discardRubberbandLine (int32_t *num);


  //  Moving object functions

  void setMovingList (int32_t *num, NV_I32_COORD2 *list_data, int32_t line_count, int32_t x, int32_t y, double angle,
                      int32_t line_width, NV_C_RGBA color);
  void setMovingList (int32_t *num, NV_I32_COORD2 *list_data, int32_t line_count, int32_t x, int32_t y, double angle,
                      int32_t line_width, QColor color);
  void closeMovingList (int32_t *num);


  //  Drawing functions

  void clearDisplayLists ();
  void drawSphere (double map_x, double map_y, double map_z, double radius, int32_t slices, int32_t stacks, QColor color, uint8_t flush);
  void clearText ();
  void drawText (QString string, double map_x, double map_y, double map_z, float angle, int32_t size, QColor color, uint8_t flush);
  void drawText (QString string, double map_x, double map_y, double map_z, float angle, QString font, int32_t size, QColor color, uint8_t flush);
  void drawLine (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width, Qt::PenStyle penStyle, uint8_t flush);


  //  Utility functions

  void get3DCoords (int32_t x, int32_t y, double *map_x, double *map_y, double *map_z);
  void getMarker3DCoords (int32_t x, int32_t y, double *map_x, double *map_y, double *map_z);
  void getFaux3DCoords (double anchor_x, double anchor_y, double anchor_z, int32_t pixel_x, int32_t pixel_y, NV_F64_COORD3 *coords);
  void map_to_screen (int32_t count, double *map_x, double *map_y, double *map_z, float *vertex_x, float *vertex_y, float *vertex_z);
  void map_to_screen (double map_x, double map_y, double map_z, float *vertex_x, float *vertex_y, float *vertex_z);
  void map_to_screen (double map_x, double map_y, float map_z, float *vertex_x, float *vertex_y, float *vertex_z);
  void z_to_y_triangle2 (float **data, int32_t layer, int32_t row, int32_t col);
  void z_to_y_triangle (float **data, int32_t layer, int32_t row, int32_t col);
  void screen_to_map (int32_t count, float *vertex_x, float *vertex_y, float *vertex_z, double *map_x, double *map_y, double *map_z);
  void screen_to_map (float vertex_x, float vertex_y, float vertex_z, double *map_x, double *map_y, double *map_z);
  void displayLayer (int32_t layer, uint8_t display);
  void resetMap ();
  void enableSignals ();
  void disableSignals ();
  void force_full_res ();
  void flush ();
  void setDepthTest (uint8_t on);
  NVMAPGL_DEF getMapdef ();


signals:

  void enterSignal (QEvent *e);
  void leaveSignal (QEvent *e);
  void mousePressSignal (QMouseEvent *e, double map_x, double map_y, double map_z);
  void mouseDoubleClickSignal (QMouseEvent *e, double map_x, double map_y, double map_z);
  void preliminaryMousePressSignal (QMouseEvent *e);
  void mouseReleaseSignal (QMouseEvent *e, double map_x, double map_y, double map_z);
  void mouseMoveSignal (QMouseEvent *e, double map_x, double map_y, double map_z, NVMAPGL_DEF map);
  void wheelSignal (QWheelEvent *e, double map_x, double map_y, double map_z);
  void closeSignal (QCloseEvent *e);
  void focusInSignal (QFocusEvent *e);
  void focusOutSignal (QFocusEvent *e);
  void resizeSignal (QResizeEvent *e);
  void exaggerationChanged (float exaggeration, float apparent_exaggeration);


protected:

  void drawScale ();
  void enterEvent (QEvent *e);
  void leaveEvent (QEvent *e);
  void boundsCheck (double *map_x, double *map_y);
  void mouseDoubleClickEvent (QMouseEvent *e);
  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  void mouseMoveEvent (QMouseEvent *e);
  void wheelEvent (QWheelEvent *e);
  void closeEvent (QCloseEvent *e);
  void focusInEvent (QFocusEvent *e);
  void focusOutEvent (QFocusEvent *e);
  uint8_t getNearestLayerVertex (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, GLuint *data_list, GLfloat ****data_vertices,
                                 int32_t *data_rows, int32_t *data_cols, int32_t *pfm, int32_t *row, int32_t *col);
  uint8_t getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, NVMAPGL_POINT_OBJECT *data, int32_t count, int32_t *index);
  uint8_t getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, GLfloat *data, int32_t count, int32_t *index);
  uint8_t getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, GLfloat *data, GLubyte *color, int32_t count, int32_t *index);
  uint8_t getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, NVMAPGL_MARKER_OBJECT *data, int32_t count, int32_t *index);

  void normalizedcross (GLfloat* u, GLfloat* v, GLfloat* n);
  void initializeGL ();
  void resizeGL (int w, int h);
  void paintEvent (QPaintEvent *e);
  void paintGL ();

  void generateDataPointsDL (int32_t count);
  void generateSparsePointsDL (int32_t count);

  int32_t create_movable_object (NVMAPGL_MOVABLE_OBJECT mo);
  void discard_movable_object (int32_t *num);
  void get3DCoords (int32_t x, int32_t y, NV_F32_COORD3 *coords);
  GLushort penStyle_to_stipple (Qt::PenStyle penStyle);


  QString                 parentName;
  NVMAPGL_MOVABLE_OBJECT  movable_object[MAX_GL_MOVABLE_OBJECTS];
  NVMAPGL_MOVABLE_OBJECT  *mot[MAX_GL_MOVABLE_OBJECTS];
  int32_t                 movable_object_count;
  NVMAPGL_TEXT_OBJECT     text_object[MAX_GL_TEXT_OBJECTS];
  uint32_t                text_object_count;
  NVMAPGL_DEF             map;


  float                   z_orientation;                        /*  This is set to either 1.0 to draw the "box scale" with depth values
                                                                    or -1.0 based to draw it with elevation numbers.  */

  uint8_t                 glReady;

  GLfloat                 ****data_normals;
  GLfloat                 ****data_vertices;
  GLuint                  data_list[MAX_ABE_PFMS];              //  Display list for PFM layer
  int32_t                 data_cols[MAX_ABE_PFMS];              //  Number of columns in PFM layer
  int32_t                 data_rows[MAX_ABE_PFMS];              //  Number of rows in PFM layer
  uint8_t                 data_display[MAX_ABE_PFMS];           //  NVTrue to display layer
  double                  data_null_val[MAX_ABE_PFMS];          //  Null value for PFM layer

  GLfloat                 ****data2_normals;
  GLfloat                 ****data2_vertices;
  GLuint                  data2_list[MAX_ABE_PFMS];             //  Display list for PFM layer
  int32_t                 data2_cols[MAX_ABE_PFMS];             //  Number of columns in PFM layer
  int32_t                 data2_rows[MAX_ABE_PFMS];             //  Number of rows in PFM layer
  int32_t                 data2_div[MAX_ABE_PFMS];              //  Whether the display needs a subsampled layer

  GLuint                  point_list;
  GLuint                  sparse_list;
  GLuint                  scale_list;
  GLuint                  *display_list;                        //  Array of display lists for primitives
  uint32_t                list_count;                           //  Number of display lists used
  GLuint                  tiffTex;                              //  Texture made from GeoTIFF
  QImage                  *texImage;                            //  Texture image
  std::vector<GLfloat>    point_data;                           //  Point data vector
  int32_t                 point_count;                          //  Number of points loaded in the point data structure
  float                   point_data_size;
  std::vector<GLubyte>    point_color_data;                     //  Point data color vector
  std::vector<GLfloat>    sparse_data;                          //  Sparse data point vector
  int32_t                 sparse_count;                         //  Number of points loaded in the sparse point data structure
  float                   sparse_data_size;
  std::vector<GLubyte>    sparse_color_data;                    //  Spares data point data color vector
  std::vector<NVMAPGL_POINT_OBJECT>     feature_data;           //  Feature data structure vector
  int32_t                 feature_count;                        //  Number of features loaded
  std::vector <NVMAPGL_MARKER_OBJECT>   marker_data;            //  Marker data structure vector
  int32_t                 marker_count;                         //  Number of markers loaded
  std::vector<NVMAPGL_LINE_OBJECT>      line_data;              //  Line data structure vector
  int32_t                 line_count;                           //  Number of lines loaded
  double                  z_scale;                              //  Scale value used to get Z to relate to X and Y as 1 to 1
  float                   aspect_ratio;                         //  Aspect ratio of current view
  float                   near_plane;                           //  Distance to near Z plane of frustum
  float                   far_plane;                            //  Distance to far Z plane of frustum
  float                   exag_scale;                           //  Exaggeration scale (1.0 / exaggeration)
  float                   fovy;                                 //  gluPerspective field of view value
  uint8_t                 texture;                              //  True if displaying a GeoTIFF texture overlay
  int32_t                 texture_type;                         //  -1 for GL_MODULATE, 1 for GL_DECAL
  uint8_t                 depth_test;                           //  Whether we're going to mask for depth (used for slicing);
                                                                //  This is only modifiable with setDepthTest.
  float                   apparent_exaggeration;                //  When Z range exceeds Y and/or X we need to play with the apparent exaggeration
  GLfloat                 data_colors[DATA_COLORS + 3][4];
  GLubyte                 scale_color[4];                       //  "Box scale" color
  GLubyte                 scale_offset_color[4];                //  Color to be used for "box scale" numbers when scale_offset is set
  NV_F64_COORD3           scale_tic[20];                        //  Coordinates for the "box scale" numbers and lines
  int32_t                 scale_tic_count;                      //  Number of tics (will always be less than 20)
  float                   x_rotation, y_rotation, z_rotation, zx_rotation;
  float                   eyeX, eyeY, eyeZ;
  float                   centerX, centerY, centerZ, prev_centerX, prev_centerY, prev_centerZ;
  double                  map_center_x, map_center_y, map_center_z;
  NV_F32_COORD3           cursor_pos;
  uint8_t                 signalsEnabled;
  GLint                   viewport[4];
  GLdouble                mvmatrix[16], projmatrix[16];
  GLfloat                 x_matrix[16], y_matrix[16], z_matrix[16], zx_matrix[16];
  NV_F64_XYMBC            bounds;
  double                  range_x, range_y, range_z;
  double                  tex_ratio_x, tex_ratio_y;
  NV_F64_XYMBR            geotiff_mbr;
  int32_t                 geotiff_width, geotiff_height;
  SQuat                   *rotation;
  int32_t                 update_type;                          /*  This will be 0 for XOR update (when we are drawing 
                                                                    movable objects), 1 for normal update, 2 to erase
                                                                    and redraw both front and back buffers, or -1 to
                                                                    just erase the previous, XORed movable object).  
                                                                    Add 256 to plot decimated surface.  */
  uint8_t                 windoze_paint_event_enabled;


  uint8_t                 rubberbandLineActive[MAX_GL_MOVABLE_OBJECTS];
  int32_t                 rubberbandLineAnchorX[MAX_GL_MOVABLE_OBJECTS], rubberbandLineAnchorY[MAX_GL_MOVABLE_OBJECTS],
                          rubberbandLineLastX[MAX_GL_MOVABLE_OBJECTS], rubberbandLineLastY[MAX_GL_MOVABLE_OBJECTS];


  uint8_t                 rubberbandPolyActive[MAX_GL_MOVABLE_OBJECTS];
  int32_t                 rubberbandPolyLastX[MAX_GL_MOVABLE_OBJECTS], rubberbandPolyLastY[MAX_GL_MOVABLE_OBJECTS];
  double                  rubberbandPolyMapX[MAX_GL_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS], 
                          rubberbandPolyMapY[MAX_GL_MOVABLE_OBJECTS][NVMAP_POLYGON_POINTS];


  uint8_t                 rubberbandRectActive[MAX_GL_MOVABLE_OBJECTS];
  int32_t                 rubberbandRectX[MAX_GL_MOVABLE_OBJECTS][4], rubberbandRectY[MAX_GL_MOVABLE_OBJECTS][4],
                          rubberbandRectLastX[MAX_GL_MOVABLE_OBJECTS], rubberbandRectLastY[MAX_GL_MOVABLE_OBJECTS];
  double                  rubberbandRectMapX[MAX_GL_MOVABLE_OBJECTS][4], rubberbandRectMapY[MAX_GL_MOVABLE_OBJECTS][4];


protected slots:


private:

};

#endif
