
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


#include "nvMapGL.hpp"

/***************************************************************************/
/*!

   - Module :        nvMapGL

   - Programmer :    Jan C. Depner

   - Date :          05/21/09

   - Purpose :       This is a generic 3D map class for use in Qt graphical user
                     interfaces.

   - Note :          Don't forget that colors can be transparent in Qt4.  You can
                     set the transparency by setting the alpha value to a number
                     between 0 and 255 with 0 being invisible and 255 being opaque.
                     For example - QColor (255, 255, 0, 127) will be a transparent
                     yellow.

    
  - Don't forget to set the defaults that you want in the NVMAP_DEF structure (*init).  A lot
    of these can be set or reset later with the map functions.  Here's an example of a pretty
    standard setup:

  <pre>

  mapdef.draw_width = 640;
  mapdef.draw_height = 480;
  mapdef.zoom_percent = 5.0;
  mapdef.exaggeration = 3.0;
  mapdef.min_z_extents = 1.0;
  mapdef.mode = NVMAPGL_BIN_MODE;
  mapdef.light_model = GL_LIGHT_MODEL_TWO_SIDE;
  mapdef.auto_scale = NVFalse;
  mapdef.projected = 0;

  mapdef.background_color = Qt::black;
  mapdef.scale_color = Qt::white;
  mapdef.draw_scale = NVTrue;
  mapdef.initial_zx_rotation = 0.0;
  mapdef.initial_y_rotation = 0.0;

  </pre>

****************************************************************************/

nvMapGL::nvMapGL (QWidget *parent, NVMAPGL_DEF *init, QString pn):
  QGLWidget (parent, 0, (Qt::WindowFlags) Qt::WA_DeleteOnClose)
{
  parentName = pn;


  //  We want to handle buffer swapping manually since this offers much better control than
  //  letting the system do auto swapping (trust me on this ;-)  ALL drawing and swapping is
  //  done in the paintgl method.

  setAutoBufferSwap (false);


  //  Track mouse movement with no button pressed.

  setMouseTracking (true);


  map = *init;


  //  Never offset or multiply the scale numbers on instantiation.  Call setScaleOffset and/or setScaleFactor instead.

  map.scale_offset = 0.0;
  map.scale_factor = 1.0;
  map.scale_offset_color = Qt::red;


  for (int32_t layer = 0 ; layer < MAX_ABE_PFMS ; layer++)
    {
      data_list[layer] = 0;
      data2_list[layer] = 0;
      data_display[layer] = NVFalse;
      data2_div[layer] = NVFalse;
    }


  glReady = NVFalse;
  data_normals = (GLfloat ****) malloc (sizeof (GLfloat ***) * MAX_ABE_PFMS);
  data_vertices = (GLfloat ****) malloc (sizeof (GLfloat ***) * MAX_ABE_PFMS);
  data2_normals = (GLfloat ****) malloc (sizeof (GLfloat ***) * MAX_ABE_PFMS);
  data2_vertices = (GLfloat ****) malloc (sizeof (GLfloat ***) * MAX_ABE_PFMS);
  display_list = NULL;
  point_list = 0;
  sparse_list = 0;
  scale_list = 0;
  list_count = 0;
  texImage = NULL;
  point_count = 0;
  sparse_count = 0;
  feature_count = 0;
  marker_count = 0;
  line_count = 0;
  update_type = 1;
  scale_tic_count = 0;
  windoze_paint_event_enabled = NVTrue;
  z_orientation = 1.0;

  y_rotation = map.initial_y_rotation;
  zx_rotation = map.initial_zx_rotation;

  x_rotation = z_rotation = 0.0;
  eyeX = eyeY = 0.0;
  eyeZ = 2.0;
  centerX = centerY = centerZ = prev_centerX = prev_centerY = prev_centerZ = 0.0;
  movable_object_count = 0;
  text_object_count = 0;


  rotation = new SQuat (0.0, 1.0, 0.0, 0.0);


  for (int32_t i = 0 ; i < MAX_MOVABLE_OBJECTS ; i++)
    {
      movable_object[i].active = NVFalse;
      rubberbandPolyActive[i] = NVFalse;
      rubberbandRectActive[i] = NVFalse;
    }



  float hue_inc = 315.0 / (float) (DATA_COLORS + 1);

  for (int32_t i = 0 ; i < DATA_COLORS ; i++)
    {
      QColor color;

      color.setHsv ((int32_t) (((DATA_COLORS + 1) - i) * hue_inc), 255, 255, 255);

      data_colors[i][0] = (GLfloat) color.red () / 255.0;
      data_colors[i][1] = (GLfloat) color.green () / 255.0;
      data_colors[i][2] = (GLfloat) color.blue () / 255.0;
      data_colors[i][3] = 1.0;
    }


  //  White

  data_colors[DATA_COLORS][0] = 1.0;
  data_colors[DATA_COLORS][1] = 1.0;
  data_colors[DATA_COLORS][2] = 1.0;
  data_colors[DATA_COLORS][3] = 1.0;


  //  Black

  data_colors[DATA_COLORS + 1][0] = 0.0;
  data_colors[DATA_COLORS + 1][1] = 0.0;
  data_colors[DATA_COLORS + 1][2] = 0.0;
  data_colors[DATA_COLORS + 1][3] = 1.0;


  //  Transparent

  data_colors[DATA_COLORS + 2][0] = 0.0;
  data_colors[DATA_COLORS + 2][1] = 0.0;
  data_colors[DATA_COLORS + 2][2] = 0.0;
  data_colors[DATA_COLORS + 2][3] = 0.0;


  //  Make sure we have some decent values if they weren't set.

  if (map.draw_width <= 0) map.draw_width = 600;
  if (map.draw_height <= 0) map.draw_height = 600;
  if (map.zoom_percent <= 0 || map.zoom_percent >= 100) map.zoom_percent = 10;
  if (map.exaggeration < 1.0 || map.exaggeration > 100.0) map.exaggeration = 3.0;
  if (map.mode < 0 || map.mode > 2) map.mode = NVMAPGL_BIN_MODE;
  if (map.draw_scale > 1) map.draw_scale = NVTrue;
  if (map.min_z_extents < 1.0 || map.min_z_extents > 100.0) map.min_z_extents = 1.0;
  if (map.projected < 0 || map.projected > 1) map.projected = 0;


  apparent_exaggeration = map.exaggeration;


  scale_color[0] = map.scale_color.red ();
  scale_color[1] = map.scale_color.green ();
  scale_color[2] = map.scale_color.blue ();
  scale_color[3] = map.scale_color.alpha ();
  scale_offset_color[0] = map.scale_offset_color.red ();
  scale_offset_color[1] = map.scale_offset_color.green ();
  scale_offset_color[2] = map.scale_offset_color.blue ();
  scale_offset_color[3] = map.scale_offset_color.alpha ();



  if (map.light_model != GL_LIGHT_MODEL_AMBIENT && map.light_model != GL_LIGHT_MODEL_TWO_SIDE) map.light_model = GL_LIGHT_MODEL_AMBIENT;

  depth_test = NVTrue;
  texture_type = GL_DECAL;
  texture = NVFalse;
  fovy = 40.0;
  exag_scale = 1.0 / map.exaggeration;
  map.zoom_factor = 1.0;


  setBounds (map.initial_bounds);


  //  Initialize the coastline thickness if it hasn't been set

  if (map.coast_thickness <= 0 || map.coast_thickness > 5) map.coast_thickness = 2;


  signalsEnabled = NVFalse;


#ifndef QT_NO_CURSOR
  setCursor (Qt::WaitCursor);
#endif
}



nvMapGL::~nvMapGL ()
{
  clearData (NVTrue);
}



/*!  Clear all 3D constructs.  The scale, data points, sparse data points, feature list, marker list, line list,
     data layers, display lists, and textures.  */

void 
nvMapGL::clearData (uint8_t clear_texture)
{
  //  Delete the scale display list.

  glDeleteLists (scale_list, 1);


  //  Clear the data points.

  clearDataPoints ();


  //  Clear the sparse data points.

  clearSparsePoints ();


  //  Clear the feature list.

  clearFeaturePoints ();


  //  Clear the marker list.

  clearMarkerPoints ();


  //  Clear the line list.

  clearLines ();


  //  Clear data layers.

  for (int32_t pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++) clearDataLayer (pfm);


  //  Clear any displaylists.

  for (uint32_t i = 0 ; i < list_count ; i++) glDeleteLists (display_list[i], 1);

  if (display_list != NULL) free (display_list);


  //  Clear textures.

  if (clear_texture) clearGeotiffTexture ();
}



//  *************************************************************************************
//
//  Map functions
//
//  *************************************************************************************


/*!  Get the 2D screen coordinates of a 3D point.  The *init2D argument is a boolean that
     you set to true to start a run of points when you know the viewport, mvmatrix, and
     projmatrix aren't going to change.  Like when you want to find which 3D points are
     in a 2D polygon.  The overhead of the glGet* calls is worse than the map_to_screen
     and gluProject calls and we don't need to call them over and over if we're not
     changing those values.  Remember, don't set it right before the call, set it outside
     of the loop because this function sets it to false so it won't call the glGet*
     functions again for the external loop.  If you're just calling this once and then
     moving on then you don't need the *init2D value.  */

void 
nvMapGL::get2DCoords (double x, double y, double z, int32_t *px, int32_t *py, uint8_t *init2D)
{
  if (*init2D)
    {
      glGetIntegerv (GL_VIEWPORT, viewport);
      glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
      glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);

      *init2D = NVFalse;
    }

  GLfloat wx, wy, wz;
  GLdouble sx, sy, sz;
  map_to_screen (x, y, z, &wx, &wy, &wz);

  gluProject (wx, wy, wz, mvmatrix, projmatrix, viewport, &sx, &sy, &sz);


  //  viewport[3] is height of window in pixels.  OpenGL renders with (0,0) on bottom,
  //  mouse reports with (0,0) on top.

  *py = viewport[3] - NINT (sy) - 1;
  *px = NINT (sx);
}



//!  Get the 2D screen coordinates of a 3D point.  This version accepts a 32 bit float for the Z value.

void 
nvMapGL::get2DCoords (double x, double y, float z, int32_t *px, int32_t *py, uint8_t *init)
{
  get2DCoords (x, y, (double) z, px, py, init);
}


//!  Without the init argument for single, isolated calls.

void 
nvMapGL::get2DCoords (double x, double y, double z, int32_t *px, int32_t *py)
{
  uint8_t init = NVTrue;

  get2DCoords (x, y, z, px, py, &init);
}


//!  Float Z version without the init argument for single, isolated calls.

void 
nvMapGL::get2DCoords (double x, double y, float z, int32_t *px, int32_t *py)
{
  uint8_t init = NVTrue;

  get2DCoords (x, y, (double) z, px, py, &init);
}


//!  Set the background color.

void
nvMapGL::setBackgroundColor (QColor color)
{
  float r, g, b, a;

  r = (float) color.red () / 255.0;
  g = (float) color.green () / 255.0;
  b = (float) color.blue () / 255.0;
  a = (float) (255 - color.alpha ()) / 255.0;

  map.background_color = color;

  glClearColor (r, g, b, a);

  update_type = 1;
  updateGL ();
}



//!  Set the "box scale" color.

void
nvMapGL::setScaleColor (QColor color)
{
  scale_color[0] = color.red ();
  scale_color[1] = color.green ();
  scale_color[2] = color.blue ();
  scale_color[3] = color.alpha ();

  map.scale_color = color;

  update_type = 1;
  updateGL ();
}



//!  Set the "box scale" offset.

void
nvMapGL::setScaleOffset (float offset)
{
  map.scale_offset = offset;

  update_type = 1;
  updateGL ();
}



//!  Set the "box scale" multiplication factor.

void
nvMapGL::setScaleFactor (float factor)
{
  map.scale_factor = factor;

  update_type = 1;
  updateGL ();
}



//!  Set the "box scale" number color for use when scale_offset and/or scale_factor is set (to other than 0.0 or 1.0 respectively).

void
nvMapGL::setScaleOffsetColor (QColor color)
{
  scale_offset_color[0] = color.red ();
  scale_offset_color[1] = color.green ();
  scale_offset_color[2] = color.blue ();
  scale_offset_color[3] = color.alpha ();

  map.scale_offset_color = color;

  update_type = 1;
  updateGL ();
}



//!  Set the "box scale" orientation.  Set depth to NVTrue for depth or NVFalse for elevation.

void
nvMapGL::setScaleOrientation (uint8_t depth)
{
  if (depth)
    {
      z_orientation = 1.0;
    }
  else
    {
      z_orientation = -1.0;
    }

  update_type = 1;
  updateGL ();
}



//!  Enable/disable drawing the "box scale".

void
nvMapGL::enableScale (uint8_t enable)
{
  map.draw_scale = enable;

  update_type = 1;
  updateGL ();
}



/*!  Set the Z exaggeration value.  This will require a reload of the data layers and we're not
     doing it internally.  This means that the caller will have to call setDataLayer, setDataPoints,
     setSparsePoints, and/or setFeaturePoints again.  In addition, if you're scaling any display lists
     you probably want to do that again as well.  IMPORTANT NOTE: When you do this, don't call setBounds
     again unless you actually changed the X, Y, and Z bounds since setBounds will try to auto scale the
     exaggeration.  */

void
nvMapGL::setExaggeration (float value)
{
  if (map.auto_scale && apparent_exaggeration < 1.0) return;


  map.exaggeration = value;
  exag_scale = 1.0 / map.exaggeration;

  if (z_scale < 1.0) apparent_exaggeration = map.exaggeration;


  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane); 

  glMatrixMode (GL_MODELVIEW);


  //  Build the "scale" display list just in case we want to plot it.

  if (glReady) drawScale ();


  emit exaggerationChanged (map.exaggeration, apparent_exaggeration);
}



//!  Set the minimum Z extents auto scale limit.

void
nvMapGL::setMinZExtents (float value)
{
  map.min_z_extents = value;
  if (map.min_z_extents < 1.0 || map.min_z_extents > 100.0) map.min_z_extents = 1.0;
}



//!  Set the zoom percentage for percent zoom in or out.

void
nvMapGL::setZoomPercent (int32_t percent)
{
  map.zoom_percent = percent;
}



//!  Zoom in by the input "percent".

void 
nvMapGL::zoomInPercent (int32_t percent)
{
  map.zoom_factor -= (((float) percent / 100.0) * map.zoom_factor);


  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane); 

  glMatrixMode (GL_MODELVIEW);


  update_type = 257;
  updateGL ();
}



//!  Zoom in by the map.zoom_percent.

void 
nvMapGL::zoomInPercent ()
{
  zoomInPercent (map.zoom_percent);
}



//!  Zoom out by the input "percent".

void 
nvMapGL::zoomOutPercent (int32_t percent)
{
  map.zoom_factor += (((float) percent / 100.0) * map.zoom_factor);


  if (map.zoom_factor > far_plane) map.zoom_factor = far_plane;


  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane); 

  glMatrixMode (GL_MODELVIEW);


  update_type = 257;
  updateGL ();
}



//!  Zoom out by the map.zoom_percent.

void 
nvMapGL::zoomOutPercent ()
{
  zoomOutPercent (map.zoom_percent);
}



//!  Reset the zoom factor to the original setting.

void 
nvMapGL::resetZoom ()
{
  map.zoom_factor = 1.0;

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane); 

  glMatrixMode (GL_MODELVIEW);


  update_type = 1;
  updateGL ();
}



//!  Reset POV and zoom factor to the original settings.

void 
nvMapGL::resetPOV ()
{
  map.zoom_factor = 1.0;

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane); 

  glMatrixMode (GL_MODELVIEW);

  rotateY (-y_rotation);
  rotateZX (-zx_rotation);


  update_type = 1;
  updateGL ();
}



//!  Return a snapshot of the complete NVMAP_DEF structure.

NVMAPGL_DEF 
nvMapGL::getMapdef ()
{
  return (map);
}



//!  Set the type of texture overlay.  -1 = GL_MODULATE, 1 = GL_DECAL

void 
nvMapGL::setTextureType (int32_t type)
{
  switch (type)
    {
    case -1:
      texture_type = GL_MODULATE;
      break;

    default:
    case 1:
       texture_type = GL_DECAL;
       break;
    }
}



//!  Enable or diable texture display.

void 
nvMapGL::enableTexture (uint8_t enable)
{
  texture = enable;
}



//!  Turn auto scaling on or off.

void 
nvMapGL::setAutoScale (uint8_t auto_scale)
{
  map.auto_scale = auto_scale;
}



/*!  Set the minimum bounding cube (MBC) bounds.  These can be in longitude, latitude, elevation but can also be
     in UTM X and Y coordinates.  See setProjected.  */

void 
nvMapGL::setBounds (NV_F64_XYMBC total_mbc)
{
  bounds = total_mbc;


  range_x = bounds.max_x - bounds.min_x;
  range_y = bounds.max_y - bounds.min_y;
  range_z = bounds.max_z - bounds.min_z;


  //  Compute the center position

  map_center_x = bounds.min_x + range_x / 2.0;
  map_center_y = bounds.min_y + range_y / 2.0;
  map_center_z = bounds.min_z + range_z / 2.0;


  //  Limit the Z window extents if we've asked for it.

  if (map.auto_scale && range_z < map.min_z_extents)
    {
      range_z = map.min_z_extents;
      bounds.min_z = map_center_z - range_z / 2.0;
      bounds.max_z = map_center_z + range_z / 2.0;
    }


  //  Figure out the z_scale (depth) based on the size of the area in meters.  This gets us to a 1 to 1 to 1
  //  perspective.  That is, 1 meter in X will be equal to 1 meter in Y and 1 meter in Z so that, if you rotate the
  //  view to have either the Y or X axis in map coordinates (-Z and X coordinates in OpenGL coordinates) parallel
  //  to the screen face, 1 meter left and right will cover the same number of pixels as 1 meter up and down.
  //  This scale is not to be confused with Z exaggeration (in OpenGL actually Y) which is applied by scaling the X and Z
  //  axes down so that we are always in a 1X1X1 cube centered on 0,0,0.  So, X and Y values are converted to a value in
  //  the -0.5 to 0.5 range and then scaled down based on the inverse of the Z exaggeration.

  double dist, az;


  //  If we're doing lat/lon we need to compute the distance in meters from max_y to min_y.  For UTM we just subtract.

  if (!map.projected)
    {
      invgp (NV_A0, NV_B0, bounds.min_y, bounds.min_x, bounds.max_y, bounds.min_x, &dist, &az);
    }
  else
    {
      dist = bounds.max_y - bounds.min_y; 
    }

  z_scale = range_z / dist;


  //  We want to try to auto-scale the data to fit in a 1X1X1 cube.  If the depth/elevation range (meters) is less than the Y and
  //  X range (meters) this is easy.  We just set the exaggeration to stretch it to the max allowable size.  If the depth/elevation
  //  range is larger than the Y/X range we have some problems.  We can't scale the OpenGL X and Z (map Y/X) axes out because stuff
  //  starts to disappear.  In this case we have to squash the z_scale down as well as setting exaggeration to 1.0.  We will be showing
  //  an apparent depth/elevation exaggeration of less than 1.0 but we have to try to hang on to what it was set to prior to blowing out
  //  the z_scale.  IMPORTANT NOTE: We usually don't auto scale for NVMAPGL_BIN_MODE (surfaces).

  if (map.auto_scale)
    {
      if (z_scale >= 1.0)
        {
          float save_exag = map.exaggeration;

          setExaggeration (1.0);

          apparent_exaggeration = map.exaggeration / z_scale;

          z_scale = 1.0;

          emit exaggerationChanged (save_exag, apparent_exaggeration);
        }
      else
        {
          apparent_exaggeration = map.exaggeration;


          //  Limit the exaggeration to 100 times.

          float exag = 1.0 / z_scale;
          if (exag > 100.0)
            {
              exag = 100.0;
              z_scale = 1.0 / exag;
            }

          setExaggeration (exag);
        }
    }


  //  Build the "scale" display list just in case we want to plot it.

  if (glReady) drawScale ();
}



//!  Set to 0 for UTM (meters) or 1 for lat/lon.
 
void 
nvMapGL::setProjected (int32_t projected)
{
  if (projected < 0 || projected > 1) return;

  map.projected = projected;
}



//!  Clear the geotiff texture overlay.

void 
nvMapGL::clearGeotiffTexture ()
{
  if (tiffTex) glDeleteTextures (1, &tiffTex);
  if (texImage) delete texImage;

  //updateGL ();
}



//!  If you are going to display a GeoTIFF texture you must always call this prior to calling setDataLayer

void 
nvMapGL::setGeotiffTexture (QImage *subImage, NV_F64_XYMBR mbr, int32_t type)
{
  if (subImage != NULL)
    {
      geotiff_mbr = mbr;

      if (tiffTex) glDeleteTextures (1, &tiffTex);


      setTextureType (type);


      texture = NVTrue;


      glGenTextures (1, &tiffTex);

      if (texImage) delete texImage;

      texImage = new QImage (convertToGLFormat (*subImage));


      geotiff_width = texImage->width ();
      geotiff_height = texImage->height ();


      glBindTexture (GL_TEXTURE_2D, tiffTex);

      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
}



//!  Clear the data layer.

void 
nvMapGL::clearDataLayer (int32_t layer)
{
  if (data2_list[layer])
    {
      glDeleteLists (data2_list[layer], 1);

      for (int32_t i = 0 ; i < data2_rows[layer] ; i++)
        {
          for (int32_t j = 0 ; j < data2_cols[layer] ; j++)
            {
              free (data2_vertices[layer][i][j]);
              free (data2_normals[layer][i][j]);
            }
          free (data2_vertices[layer][i]);
          free (data2_normals[layer][i]);
        }
      free (data2_vertices[layer]);
      free (data2_normals[layer]);
    }

  data2_list[layer] = 0;


  if (data_list[layer])
    {
      glDeleteLists (data_list[layer], 1);

      for (int32_t i = 0 ; i < data_rows[layer] ; i++)
        {
          for (int32_t j = 0 ; j < data_cols[layer] ; j++)
            {
              free (data_vertices[layer][i][j]);
              free (data_normals[layer][i][j]);
            }
          free (data_vertices[layer][i]);
          free (data_normals[layer][i]);
        }
      free (data_vertices[layer]);
      free (data_normals[layer]);
    }

  data_list[layer] = 0;
}



//!  Normalized cross product.

void 
nvMapGL::normalizedcross (GLfloat* u, GLfloat* v, GLfloat* n)
{
  // compute the cross product (u x v for right-handed [ccw])

  n[0] = u[1] * v[2] - u[2] * v[1];
  n[1] = u[2] * v[0] - u[0] * v[2];
  n[2] = u[0] * v[1] - u[1] * v[0];


  // normalize

  GLfloat norm = (GLfloat) sqrt (n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
  n[0] /= norm;
  n[1] /= norm;
  n[2] /= norm;
}



//!  This is used by setDataLayer2.

void 
nvMapGL::z_to_y_triangle2 (float **array, int32_t layer, int32_t row, int32_t col)
{
  static float prev_z = 0.0;


  if (array[row][col] == data_null_val[layer])
    {
      //  Look for non-null points within 3 points of this one 

      int32_t start_y = qMax (0, row - 3);
      int32_t end_y = qMin (data2_rows[layer] - 3, row + 3);
      int32_t start_x = qMax (0, col - 3);
      int32_t end_x = qMin (data2_cols[layer] - 3, col + 3);
      double sum = 0.0;
      int32_t cnt = 0;

      for (int32_t i = start_y ; i < end_y ; i++)
        {
          for (int32_t j = start_x ; j < end_x ; j++)
            {
              if (array[i][j] != data_null_val[layer])
                {
                  sum += (((array[i][j] - bounds.min_z) / range_z) - 0.5) * z_scale;
                  cnt++;
                }
            }
        }


      if (cnt)
        {
          data2_vertices[layer][row][col][1] = (float) (sum / (double) cnt);
        }
      else
        {
          data2_vertices[layer][row][col][1] = (float) prev_z;
        }
    }
  else
    {
      data2_vertices[layer][row][col][1] = (float) ((((array[row][col] - bounds.min_z) / range_z) - 0.5) * z_scale);
    }

  prev_z = data2_vertices[layer][row][col][1];
}



//!  This is used by setDataLayer.

void 
nvMapGL::z_to_y_triangle (float **array, int32_t layer, int32_t row, int32_t col)
{
  static float prev_z = 0.0;


  if (array[row][col] == data_null_val[layer])
    {
      //  Look for non-null points within 3 points of this one 

      int32_t start_y = qMax (0, row - 3);
      int32_t end_y = qMin (data_rows[layer] - 3, row + 3);
      int32_t start_x = qMax (0, col - 3);
      int32_t end_x = qMin (data_cols[layer] - 3, col + 3);
      double sum = 0.0;
      int32_t cnt = 0;

      for (int32_t i = start_y ; i < end_y ; i++)
        {
          for (int32_t j = start_x ; j < end_x ; j++)
            {
              if (array[i][j] != data_null_val[layer])
                {
                  sum += (((array[i][j] - bounds.min_z) / range_z) - 0.5) * z_scale;
                  cnt++;
                }
            }
        }


      if (cnt)
        {
          data_vertices[layer][row][col][1] = (float) (sum / (double) cnt);
        }
      else
        {
          data_vertices[layer][row][col][1] = (float) prev_z;
        }
    }
  else
    {
      data_vertices[layer][row][col][1] = (float) ((((array[row][col] - bounds.min_z) / range_z) - 0.5) * z_scale);
    }

  prev_z = data_vertices[layer][row][col][1];
}



//!  This constructs the "sparse" data layer from the input data and color arrays.  This is called from setDataLayer.

uint8_t 
nvMapGL::setData2Layer (int32_t layer, float **data, float **color, float color_min, float color_max,
                        int32_t rows, int32_t cols, double y_grid, double x_grid, NV_F64_XYMBR mbr, int32_t layer_type)
{
  float color_range = color_max - color_min;

  data2_div[layer] = 0;

  if (rows > cols)
    {
      if (rows > 750) data2_div[layer] = rows / 750 + 1;
    }
  else
    {
      if (cols > 750) data2_div[layer] = cols / 750 + 1;
    }


  if (!data2_div[layer]) return (NVTrue);


  int32_t rows2 = rows / data2_div[layer];
  int32_t cols2 = cols / data2_div[layer];

  double x_grid2 = x_grid * (double) data2_div[layer];
  double y_grid2 = y_grid * (double) data2_div[layer];

  float **data2 = NULL;
  data2 = (float **) malloc (rows2 * sizeof (float *));
  if (data2 == NULL)
    {
      perror ("data2");
      exit (-1);
    }

  for (int32_t i = 0 ; i < rows2 ; i++)
    {
      data2[i] = (float *) malloc (cols2 * sizeof (float));

      if (data2[i] == NULL)
        {
          perror ("data2[i]");
          exit (-1);
        }
    }


  float **color2 = NULL;
  if (color)
    {
      color2 = (float **) malloc (rows2 * sizeof (float *));
      if (color2 == NULL)
        {
          perror ("color2");
          exit (-1);
        }

      for (int32_t i = 0 ; i < rows2 ; i++)
        {
          color2[i] = (float *) malloc (cols2 * sizeof (float));

          if (color2[i] == NULL)
            {
              perror ("color2[i]");
              exit (-1);
            }
        }
    }


  int32_t k = 0, m = 0;
  for (int32_t i = 0 ; i < rows2 ; i++)
    {
      k = i * data2_div[layer];
      for (int32_t j = 0 ; j < cols2 ; j++)
        {
          m = j * data2_div[layer];

          int32_t cnt = 0;
          float sum = 0.0, col_sum = 0.0, min = 999999999.0, max = -999999999.0;

          for (int32_t n = 0 ; n < data2_div[layer] ; n++)
            {
              for (int32_t p = 0 ; p < data2_div[layer] ; p++)
                {
                  if (data[k + n][m + p] != data_null_val[layer])
                    {
                      switch (layer_type)
                        {
                        case -1:
                          max = qMax (max, data[k + n][m + p]);
                          cnt = 1;
                          break;

                        case 0:
                          sum += data[k + n][m + p];
                          cnt++;
                          break;

                        case 1:
                          min = qMin (min, data[k + n][m + p]);
                          cnt = 1;
                          break;
                        }
                      if (color) col_sum += color[k + n][m + p];
                    }
                }
            }

          if (cnt)
            {
              //  We flip the layers since we're working with elevation in openGL.  In other words, -1 was minimum
              //  layer type but we actually look for the max value.

              switch (layer_type)
                {
                case -1:
                  data2[i][j] = max;
                  break;

                case 0:
                  data2[i][j] = sum / (float) cnt;
                  break;

                case 1:
                  data2[i][j] = min;
                  break;
                }
              if (color) color2[i][j] = col_sum / (float) cnt;
            }
          else
            {
              data2[i][j] = data_null_val[layer];
              if (color) color2[i][j] = data_null_val[layer];
            }
        }
    }


  data2_cols[layer] = cols2;
  data2_rows[layer] = rows2;


  data2_vertices[layer] = (GLfloat ***) malloc (sizeof (GLfloat **) * (rows2 + 0));

  if (data2_vertices[layer] == NULL) 
    {
      perror ("data2_vertices");
      exit (-1);
    }


  data2_normals[layer] = (GLfloat ***) malloc (sizeof (GLfloat **) * (rows2 + 0));

  if (data2_normals[layer] == NULL) 
    {
      perror ("data2_normals");
      exit (-1);
    }


  for (int32_t i = 0 ; i < rows2 + 0 ; i++)
    {
      data2_vertices[layer][i] = (GLfloat **) malloc (sizeof (GLfloat *) * (cols2 + 0));

      if (data2_vertices[layer][i] == NULL) 
        {
          perror ("data2_vertices[i]");
          exit (-1);
        }


      data2_normals[layer][i] = (GLfloat **) malloc (sizeof (GLfloat *) * (cols2 + 0));

      if (data2_normals[layer][i] == NULL) 
        {
          perror ("data2_normals[i]");
          exit (-1);
        }


      for (int32_t j = 0 ; j < cols2 + 0 ; j++)
        {
          data2_vertices[layer][i][j] = (GLfloat *) malloc (sizeof (GLfloat) * 3);

          if (data2_vertices[layer][i][j] == NULL) 
            {
              perror ("data2_vertices[i][j]");
              exit (-1);
            }


          data2_normals[layer][i][j] = (GLfloat *) malloc (sizeof (GLfloat) * 3);

          if (data2_normals[layer][i][j] == NULL) 
            {
              perror ("data2_normals[i][j]");
              exit (-1);
            }
        }
    }



  GLfloat u[3], v[3], n[3], prev_n[3] = {0.0, 0.0, 0.0};


  //  We're normalizing all of the vertex data to -0.5 to 0.5 in X, Y, and Z so that the axes pass through the center of the area.
  //  You can think of the viewed area as a 1 * 1 * 1 cube.  Note that we invert the map Y since OpenGL positive Z is towards the
  //  viewer and we stored the data from bottom to top.

  for (int32_t i = 0 ; i < rows2 - 1 ; i++)
    {
      double y = mbr.min_y + (double) i * y_grid2;

      double y_vertex[2];
      y_vertex[0] = (((bounds.max_y - y) / range_y) - 0.5) * exag_scale;
      y_vertex[1] = (((bounds.max_y - (y + y_grid2)) / range_y) - 0.5) * exag_scale;

      for (int32_t j = 0 ; j < cols2 - 1 ; j++)
        {
          double x = mbr.min_x + (double) j * x_grid2;


          //  Assign the data to vertices.  Some of the vertices will be overwritten in subsequent iterations of the
          //  loop, but this is okay, since they will be identical.

          data2_vertices[layer][i][j][0] = (((x - bounds.min_x) / range_x) - 0.5) * exag_scale;
          data2_vertices[layer][i][j][2] = y_vertex[0];
          z_to_y_triangle2 (data2, layer, i, j);


          data2_vertices[layer][i][j + 1][0] = ((((x + x_grid2) - bounds.min_x) / range_x) - 0.5) * exag_scale;
          data2_vertices[layer][i][j + 1][2] = data2_vertices[layer][i][j][2];
          z_to_y_triangle2 (data2, layer, i, j + 1);


          data2_vertices[layer][i + 1][j][0] = data2_vertices[layer][i][j][0];
          data2_vertices[layer][i + 1][j][2] = y_vertex[1];
          z_to_y_triangle2 (data2, layer, i + 1, j);


          //  Get two vectors to cross.

          if (data2_vertices[layer][i][j + 1][1] > 1.0 || data2_vertices[layer][i + 1][j][1] > 1.0)
            {
              n[0] = prev_n[0];
              n[1] = prev_n[1];
              n[2] = prev_n[2];
            }
          else
            {
              u[0] = data2_vertices[layer][i][j + 1][0] - data2_vertices[layer][i][j][0];
              u[1] = data2_vertices[layer][i][j + 1][1] - data2_vertices[layer][i][j][1];
              u[2] = data2_vertices[layer][i][j + 1][2] - data2_vertices[layer][i][j][2];

              v[0] = data2_vertices[layer][i + 1][j][0] - data2_vertices[layer][i][j][0];
              v[1] = data2_vertices[layer][i + 1][j][1] - data2_vertices[layer][i][j][1];
              v[2] = data2_vertices[layer][i + 1][j][2] - data2_vertices[layer][i][j][2];


              //  Get the normalized cross product

              normalizedcross (u, v, n);
            }


          //  Put the facet normal in the i, j position.

          data2_normals[layer][i][j][0] = n[0];
          data2_normals[layer][i][j][1] = n[1];
          data2_normals[layer][i][j][2] = n[2];

          prev_n[0] = n[0];
          prev_n[1] = n[1];
          prev_n[2] = n[2];
        }
    }


  //  Fill in the last vertex and it's facet normal

  data2_vertices[layer][rows2 - 1][cols2 - 1][0] = (((bounds.max_x - bounds.min_x - x_grid2) / range_x) - 0.5) * exag_scale;
  data2_vertices[layer][rows2 - 1][cols2 - 1][2] = y_grid2 / range_y - 0.5;
  z_to_y_triangle2 (data2, layer, rows2 - 1, cols2 - 1);


  data2_normals[layer][rows2 - 1][cols2 - 1][0] = n[0];
  data2_normals[layer][rows2 - 1][cols2 - 1][1] = n[1];
  data2_normals[layer][rows2 - 1][cols2 - 1][2] = n[2];


  //  Fill in the normals on the top/bottom edge of the data (simply copy the one below/above it).

  for (int32_t i = 0 ; i < rows2 ; i++)
    {
      data2_normals[layer][i][0][0] = data2_normals[layer][i][1][0];
      data2_normals[layer][i][0][1] = data2_normals[layer][i][1][1];
      data2_normals[layer][i][0][2] = data2_normals[layer][i][1][2];

      data2_normals[layer][i][cols2 - 1][0] = data2_normals[layer][i][cols2 - 2][0];
      data2_normals[layer][i][cols2 - 1][1] = data2_normals[layer][i][cols2 - 2][1];
      data2_normals[layer][i][cols2 - 1][2] = data2_normals[layer][i][cols2 - 2][2];
    }


  //  Fill in the normals on the left/right edge of the data (simply copy the one right/left of it).

  for (int32_t j = 0 ; j < cols2 ; j++)
    {
      data2_normals[layer][0][j][0] = data2_normals[layer][1][j][0];
      data2_normals[layer][0][j][1] = data2_normals[layer][1][j][1];
      data2_normals[layer][0][j][2] = data2_normals[layer][1][j][2];

      data2_normals[layer][rows2 - 1][j][0] = data2_normals[layer][rows2 - 2][j][0];
      data2_normals[layer][rows2 - 1][j][1] = data2_normals[layer][rows2 - 2][j][1];
      data2_normals[layer][rows2 - 1][j][2] = data2_normals[layer][rows2 - 2][j][2];
    }


  //  Generate a display list for the data

  data2_list[layer] = glGenLists (1);

  if (data2_list[layer] == 0)
    {
      fprintf (stderr, "Unable to generate display list\n");
      exit (-1);
    }


  //  I realize that I don't need the brackets after the glNewList statement and before the glEndList statement 
  //  but it sets off the code and allows XEmacs to indent it nicely.  Same for glBegin and glEnd.  JCD

  glNewList (data2_list[layer], GL_COMPILE);
  {
    if (texture)
      {
        glEnable (GL_TEXTURE_2D);
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texture_type);
        glBindTexture (GL_TEXTURE_2D, tiffTex);


#warning Why do I have to call glTexImage2D here?????

        //  I really shouldn't have to reload the texture to attach it to the points.  I should only have to 
        //  bind it again but it doesn't work that way for some unknown reason.

        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, geotiff_width, geotiff_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage->bits ());
      }

    int32_t c_index[2] = {0, 0};
    float tex_x = 0.0, tex_y = 0.0;
    double map_x, map_y, map_z;


    for (int32_t i = 0 ; i < rows2 - 1 ; i++)
      {
        glBegin (GL_TRIANGLE_STRIP);
        {
          for (int32_t j = 0 ; j < cols2 ; j++)
            {
              uint8_t trans = NVFalse;


              if (data2[i][j] == data_null_val[layer] || data2[i + 1][j] == data_null_val[layer] ||
                  (j != cols2 - 1 && data2[i][j + 1] == data_null_val[layer])) trans = NVTrue;

              if (trans)
                {
                  c_index[0] = c_index[1] = DATA_COLORS + 2;
                }
              else
                {
                  if (color)
                    {
                      if (color2[i + 1][j] == data_null_val[layer] || color2[i][j] == data_null_val[layer])
                        {
                          trans = NVTrue;
                          c_index[0] = c_index[1] = DATA_COLORS + 2;
                        }
                      else
                        {
                          c_index[0] = NINT (DATA_COLORS * ((color2[i + 1][j] - color_min) / color_range));
                          c_index[1] = NINT (DATA_COLORS * ((color2[i][j] - color_min) / color_range));
                        }
                    }
                  else
                    {
                      c_index[0] = NINT (DATA_COLORS * (data2_vertices[layer][i + 1][j][1] / z_scale + 0.5));
                      c_index[1] = NINT (DATA_COLORS * (data2_vertices[layer][i][j][1] / z_scale + 0.5));
                    }
                }

              glColor4fv (data_colors[c_index[0]]);

              if (texture)
                {
                  //  2D texture coordinates are from 0.0 to 1.0 in both directions so we have to convert our
                  //  3D vertices (-0.5 to 0.5 in X and Z) to a 0.0 to 1.0 number.

                  screen_to_map (data2_vertices[layer][i + 1][j][0], data2_vertices[layer][i + 1][j][1], 
                                 data2_vertices[layer][i + 1][j][2], &map_x, &map_y, &map_z);

                  if (map_x >= geotiff_mbr.min_x && map_x <= geotiff_mbr.max_x &&
                      map_y >= geotiff_mbr.min_y && map_y <= geotiff_mbr.max_y)
                    {
                      tex_x = (float) ((data2_vertices[layer][i + 1][j][0] / tex_ratio_x) + 0.5);
                      tex_y = (float) ((data2_vertices[layer][i + 1][j][2] / tex_ratio_y) + 0.5);

                      glTexCoord2f (tex_x, tex_y);
                    }
                }

              glNormal3fv (data2_normals[layer][i + 1][j]);
              glVertex3fv (data2_vertices[layer][i + 1][j]);


              glColor4fv (data_colors[c_index[1]]);

              if (texture)
                {
                  screen_to_map (data2_vertices[layer][i][j][0], data2_vertices[layer][i][j][1], 
                                 data2_vertices[layer][i][j][2], &map_x, &map_y, &map_z);

                  if (map_x >= geotiff_mbr.min_x && map_x <= geotiff_mbr.max_x &&
                      map_y >= geotiff_mbr.min_y && map_y <= geotiff_mbr.max_y)
                    {
                      tex_x = (float) ((data2_vertices[layer][i][j][0] / tex_ratio_x) + 0.5);
                      tex_y = (float) ((data2_vertices[layer][i][j][2] / tex_ratio_y) + 0.5);

                      glTexCoord2f (tex_x, tex_y);
                    }
                }

              glNormal3fv (data2_normals[layer][i][j]);
              glVertex3fv (data2_vertices[layer][i][j]);
            }
        }
        glEnd ();
      }

    if (texture) glDisable (GL_TEXTURE_2D);
  }
  glEndList ();


  for (int32_t i = 0 ; i < rows2 ; i++) free (data2[i]);
  free (data2);
  if (color)
    {
      for (int32_t i = 0 ; i < rows2 ; i++) free (color2[i]);
      free (color2);
    }

  signalsEnabled = NVTrue;


  update_type = 257;
  updateGL ();


  return (NVTrue);
}



/*!

   - This function creates a 3D surface from the 2D "data" elevation array.

   - Inputs:
                    - layer              =    Layer number
                    - data               =    2D data array of elevations.  If projected, the 0 row and 0 column
                                              will be in the southwest corner (contrary to most screen based coordinate
                                              systems ;-)
                    - color              =    Set to NULL to color by "data" values.  You can load a 2D array of values
                                              to color the surface here.  You must then set color_min and color_max to
                                              define the colors used (HSV color wheel from 0 to 315).  For example,
                                              store uncertainty values in the "color" array and then put the min and
                                              max uncertainty values in color_min and color_max to color by uncertainty.
                    - color_min          =    Minimum "color" value (or 0)
                    - color_max          =    Maximum "color" value (or 0)
                    - rows               =    Number of rows in "data" and "color" (if present) arrays
                    - cols               =    Number of columns in "data" and "color" (if present) arrays
                    - y_grid             =    Y grid spacing (if projected this could be in decimal degrees)
                    - x_grid             =    X grid spacing (if projected this could be in decimal degrees)
                    - null_val           =    "data" value considered to be a NULL (a hole in your data surface)
                    - mbr                =    Minimum bounding rectangle (if projected this will be SW lat/lon
                                              and NE lat/lon)
                    - layer_type         =    Layer type:
                                              -    0     =    average surface
                                              -    -1    =    minimum surface
                                              -    1     =    maximum surface

   - Returns:       NVFalse on error, else NVTrue

*/

uint8_t 
nvMapGL::setDataLayer (int32_t layer, float **data, float **color, float color_min, float color_max, 
                       int32_t rows, int32_t cols, double y_grid, double x_grid, float null_val,
                       NV_F64_XYMBR mbr, int32_t layer_type)
{
  tex_ratio_x = (range_x / (geotiff_mbr.max_x - geotiff_mbr.min_x)) * exag_scale;
  tex_ratio_y = (range_y / (geotiff_mbr.max_y - geotiff_mbr.min_y)) * exag_scale;


  //  Seems redundant but it won't work right otherwise.

  drawScale ();


  data_display[layer] = NVTrue;
  data_null_val[layer] = null_val;
  data_cols[layer] = cols;
  data_rows[layer] = rows;
  float color_range = color_max - color_min;


  if (!setData2Layer (layer, data, color, color_min, color_max, rows, cols, y_grid, x_grid, mbr, layer_type)) return (NVFalse);


  GLfloat ***fnormals;


  data_vertices[layer] = (GLfloat ***) malloc (sizeof (GLfloat **) * rows);

  if (data_vertices[layer] == NULL) 
    {
      perror ("data_vertices");
      exit (-1);
    }


  data_normals[layer] = (GLfloat ***) malloc (sizeof (GLfloat **) * rows);

  if (data_normals[layer] == NULL) 
    {
      perror ("data_normals");
      exit (-1);
    }


  fnormals = (GLfloat ***) malloc (sizeof (GLfloat **) * rows);

  if (fnormals == NULL) 
    {
      perror ("fnormals");
      exit (-1);
    }


  for (int32_t i = 0 ; i < rows ; i++)
    {
      data_vertices[layer][i] = (GLfloat **) malloc (sizeof (GLfloat *) * cols);

      if (data_vertices[layer][i] == NULL) 
        {
          perror ("data_vertices[i]");
          exit (-1);
        }


      data_normals[layer][i] = (GLfloat **) malloc (sizeof (GLfloat *) * cols);

      if (data_normals[layer][i] == NULL) 
        {
          perror ("data_normals[i]");
          exit (-1);
        }


      fnormals[i] = (GLfloat **) malloc (sizeof (GLfloat *) * cols);

      if (fnormals[i] == NULL) 
        {
          perror ("fnormals[i]");
          exit (-1);
        }


      for (int32_t j = 0 ; j < cols ; j++)
        {
          data_vertices[layer][i][j] = (GLfloat *) malloc (sizeof (GLfloat) * 3);

          if (data_vertices[layer][i][j] == NULL) 
            {
              perror ("data_vertices[i][j]");
              exit (-1);
            }


          data_normals[layer][i][j] = (GLfloat *) malloc (sizeof (GLfloat) * 3);

          if (data_normals[layer][i][j] == NULL) 
            {
              perror ("data_normals[i][j]");
              exit (-1);
            }


          fnormals[i][j] = (GLfloat *) malloc (sizeof (GLfloat) * 3);

          if (fnormals[i][j] == NULL) 
            {
              perror ("fnormals[i][j]");
              exit (-1);
            }
        }
    }



  GLfloat u[3], v[3], n[3];


  //  We're normalizing all of the vertex data to -0.5 to 0.5 in X, Y (elevation), and Z (map Y) so
  //  that the axes pass through the center of the area.  You can think of the viewed area as a 1 * 1 * 1 cube.
  //  Note that we invert the map Y since positive Z is towards the viewer and we stored the data from bottom to top.

  for (int32_t i = 0 ; i < rows - 1 ; i++)
    {
      double y = mbr.min_y + (double) i * y_grid;

      double y_vertex[2];
      y_vertex[0] = (((bounds.max_y - y) / range_y) - 0.5) * exag_scale;
      y_vertex[1] = (((bounds.max_y - (y + y_grid)) / range_y) - 0.5) * exag_scale;

      for (int32_t j = 0 ; j < cols - 1 ; j++)
        {
          double x = mbr.min_x + (double) j * x_grid;


          //  Assign the data to vertices.  Some of the vertices will be overwritten in subsequent iterations of the
          //  loop, but this is okay, since they will be identical.

          data_vertices[layer][i][j][0] = (((x - bounds.min_x) / range_x) - 0.5) * exag_scale;
          data_vertices[layer][i][j][2] = y_vertex[0];
          z_to_y_triangle (data, layer, i, j);


          data_vertices[layer][i][j + 1][0] = ((((x + x_grid) - bounds.min_x) / range_x) - 0.5) * exag_scale;
          data_vertices[layer][i][j + 1][2] = data_vertices[layer][i][j][2];
          z_to_y_triangle (data, layer, i, j + 1);


          data_vertices[layer][i + 1][j][0] = data_vertices[layer][i][j][0];
          data_vertices[layer][i + 1][j][2] = y_vertex[1];
          z_to_y_triangle (data, layer, i + 1, j);


          //  Get two vectors to cross.

          u[0] = data_vertices[layer][i][j + 1][0] - data_vertices[layer][i][j][0];
          u[1] = data_vertices[layer][i][j + 1][1] - data_vertices[layer][i][j][1];
          u[2] = data_vertices[layer][i][j + 1][2] - data_vertices[layer][i][j][2];

          v[0] = data_vertices[layer][i + 1][j][0] - data_vertices[layer][i][j][0];
          v[1] = data_vertices[layer][i + 1][j][1] - data_vertices[layer][i][j][1];
          v[2] = data_vertices[layer][i + 1][j][2] - data_vertices[layer][i][j][2];


          //  Get the normalized cross product

          normalizedcross (u, v, n);


          //  Put the facet normal in the i, j position for later averaging with other normals.

          fnormals[i][j][0] = n[0];
          fnormals[i][j][1] = n[1];
          fnormals[i][j][2] = n[2];
        }
    }


  //  Fill in the last vertex and it's facet normal

  data_vertices[layer][rows - 1][cols - 1][0] = (((bounds.max_x - bounds.min_x - x_grid) / range_x) - 0.5) * exag_scale;
  data_vertices[layer][rows - 1][cols - 1][2] = y_grid / range_y - 0.5;
  z_to_y_triangle (data, layer, rows - 1, cols - 1);


  fnormals[rows - 1][cols - 1][0] = n[0];
  fnormals[rows - 1][cols - 1][1] = n[1];
  fnormals[rows - 1][cols - 1][2] = n[2];


  //  Calculate normals for the data

  for (int32_t i = 1 ; i < rows - 1 ; i++)
    {
      for (int32_t j = 1 ; j < cols - 1 ; j++)
        {
          //  Average all the neighboring normals.

          n[0] = fnormals[i - 1][j - 1][0];
          n[1] = fnormals[i - 1][j - 1][1];
          n[2] = fnormals[i - 1][j - 1][2];

          n[0] += fnormals[i][j - 1][0];
          n[1] += fnormals[i][j - 1][1];
          n[2] += fnormals[i][j - 1][2];

          n[0] += fnormals[i - 1][j][0];
          n[1] += fnormals[i - 1][j][1];
          n[2] += fnormals[i - 1][j][2];

          n[0] += fnormals[i][j][0];
          n[1] += fnormals[i][j][1];
          n[2] += fnormals[i][j][2];
      
          GLfloat norm = (GLfloat) sqrt (n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
          data_normals[layer][i][j][0] = n[0] /= norm;
          data_normals[layer][i][j][1] = n[1] /= norm;
          data_normals[layer][i][j][2] = n[2] /= norm;
        }
    }


  //  Fill in the normals on the top/bottom edge of the data (simply copy the one below/above it).

  for (int32_t i = 0 ; i < rows ; i++)
    {
      data_normals[layer][i][0][0] = data_normals[layer][i][1][0];
      data_normals[layer][i][0][1] = data_normals[layer][i][1][1];
      data_normals[layer][i][0][2] = data_normals[layer][i][1][2];

      data_normals[layer][i][cols - 1][0] = data_normals[layer][i][cols - 2][0];
      data_normals[layer][i][cols - 1][1] = data_normals[layer][i][cols - 2][1];
      data_normals[layer][i][cols - 1][2] = data_normals[layer][i][cols - 2][2];
    }


  //  Fill in the normals on the left/right edge of the data (simply copy the one right/left of it).

  for (int32_t j = 0 ; j < cols ; j++)
    {
      data_normals[layer][0][j][0] = data_normals[layer][1][j][0];
      data_normals[layer][0][j][1] = data_normals[layer][1][j][1];
      data_normals[layer][0][j][2] = data_normals[layer][1][j][2];

      data_normals[layer][rows - 1][j][0] = data_normals[layer][rows - 2][j][0];
      data_normals[layer][rows - 1][j][1] = data_normals[layer][rows - 2][j][1];
      data_normals[layer][rows - 1][j][2] = data_normals[layer][rows - 2][j][2];
    }


  for (int32_t i = 0 ; i < rows ; i++)
    {
      for (int32_t j = 0 ; j < cols ; j++)
        {
          free (fnormals[i][j]);
        }
      free (fnormals[i]);
    }
  free (fnormals);


  //  Generate a display list for the data

  data_list[layer] = glGenLists (1);

  if (data_list[layer] == 0)
    {
      fprintf (stderr, "Unable to generate display list\n");
      exit (-1);
    }


  //  I realize that I don't need the brackets after the glNewList statement and before the glEndList statement 
  //  but it sets off the code and allows XEmacs to indent it nicely.  Same for glBegin and glEnd.  JCD

  glNewList (data_list[layer], GL_COMPILE);
  {
    if (texture)
      {
        glEnable (GL_TEXTURE_2D);
        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texture_type);
        glBindTexture (GL_TEXTURE_2D, tiffTex);


#warning Why do I have to call glTexImage2D here?????

        //  I really shouldn't have to reload the texture to attach it to the points.  I should only have to 
        //  bind it again but it doesn't work that way for some unknown reason.

        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, geotiff_width, geotiff_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texImage->bits ());
      }

    int32_t c_index[2] = {0, 0};
    float tex_x = 0.0, tex_y = 0.0;
    double map_x, map_y, map_z;


    for (int32_t i = 0 ; i < rows - 1 ; i++)
      {
        glBegin (GL_TRIANGLE_STRIP);
        {
          for (int32_t j = 0 ; j < cols ; j++)
            {
              uint8_t trans = NVFalse;


              if (data[i][j] == data_null_val[layer] || data[i + 1][j] == data_null_val[layer] ||
                  (j != cols - 1 && data[i][j + 1] == data_null_val[layer])) trans = NVTrue;


              if (trans)
                {
                  c_index[0] = c_index[1] = DATA_COLORS + 2;
                }
              else
                {
                  if (color)
                    {
                      if (color[i + 1][j] == data_null_val[layer] || color[i][j] == data_null_val[layer])
                        {
                          trans = NVTrue;
                          c_index[0] = c_index[1] = DATA_COLORS + 2;
                        }
                      else
                        {
                          c_index[0] = NINT (DATA_COLORS * ((color[i + 1][j] - color_min) / color_range));
                          c_index[1] = NINT (DATA_COLORS * ((color[i][j] - color_min) / color_range));
                        }
                    }
                  else
                    {
                      c_index[0] = NINT (DATA_COLORS * (data_vertices[layer][i + 1][j][1] / z_scale + 0.5));
                      c_index[1] = NINT (DATA_COLORS * (data_vertices[layer][i][j][1] / z_scale + 0.5));
                    }
                }


              glColor4fv (data_colors[c_index[0]]);

              if (texture)
                {
                  //  2D texture coordinates are from 0.0 to 1.0 in both directions so we have to convert our
                  //  3D vertices (-0.5 to 0.5 in X and Z) to a 0.0 to 1.0 number.

                  screen_to_map (data_vertices[layer][i + 1][j][0], data_vertices[layer][i + 1][j][1], 
                                 data_vertices[layer][i + 1][j][2], &map_x, &map_y, &map_z);

                  if (map_x >= geotiff_mbr.min_x && map_x <= geotiff_mbr.max_x &&
                      map_y >= geotiff_mbr.min_y && map_y <= geotiff_mbr.max_y)
                    {
                      tex_x = (float) ((data_vertices[layer][i + 1][j][0] / tex_ratio_x) + 0.5);
                      tex_y = (float) ((data_vertices[layer][i + 1][j][2] / tex_ratio_y) + 0.5);

                      glTexCoord2f (tex_x, tex_y);
                    }
                }

              glNormal3fv (data_normals[layer][i + 1][j]);
              glVertex3fv (data_vertices[layer][i + 1][j]);


              glColor4fv (data_colors[c_index[1]]);

              if (texture)
                {
                  screen_to_map (data_vertices[layer][i][j][0], data_vertices[layer][i][j][1], 
                                 data_vertices[layer][i][j][2], &map_x, &map_y, &map_z);

                  if (map_x >= geotiff_mbr.min_x && map_x <= geotiff_mbr.max_x &&
                      map_y >= geotiff_mbr.min_y && map_y <= geotiff_mbr.max_y)
                    {
                      tex_x = (float) ((data_vertices[layer][i][j][0] / tex_ratio_x) + 0.5);
                      tex_y = (float) ((data_vertices[layer][i][j][2] / tex_ratio_y) + 0.5);

                      glTexCoord2f (tex_x, tex_y);
                    }
                }

              glNormal3fv (data_normals[layer][i][j]);
              glVertex3fv (data_vertices[layer][i][j]);
            }
        }
        glEnd ();
      }

    if (texture) glDisable (GL_TEXTURE_2D);
  }
  glEndList ();


  signalsEnabled = NVTrue;


  update_type = 1;
  updateGL ();


  return (NVTrue);
}



//!  Clears data points (not to be confused with layers).

void 
nvMapGL::clearDataPoints ()
{
  if (point_list) glDeleteLists (point_list, 1);

  if (point_count)
    {
      point_data.clear ();
      point_color_data.clear ();
    }

  point_count = 0;
  point_list = 0;
}



/*!

   - Makes a 3D point cloud from the points loaded (one at a time).

   - Inputs:
                    - x             =    X position of a point (if projected this would be a longitude).
                    - y             =    Y position of a point (if projected this would be a latitude).
                    - z             =    Z position of a point (elevation).
                    - point_color   =    The color of the point.  This is an array of one byte (0-255) RGBA
                                         values.  For example, mostly transparent red would contain 255, 0,
                                         0, 64 in the [0], [1], [2], and [3] array values respectively.
                    - point_size    =    Size of the point in OpenGL units (the displayed OpenGL block is
                                         1.0 by 1.0 by 1.0 so this should be a pretty small number).
                                         This value is ignored until "done" is set!  You can't have different
                                         sized points.  Sorry.
                    - done          =    Set this to NVFalse until you run out of points to load.  At that
                                         time set it to NVTrue (with a reasonable point size) to display the 
                                         data.

   - Returns:       NVTrue

*/

uint8_t 
nvMapGL::setDataPoints (double x, double y, double z, uint8_t *point_color, int32_t point_size, uint8_t done)
{
  //  Use this as the counter so that we don't try to draw the points (due to an update from something else)
  //  while we're still loading the array.

  static int32_t pnt_cnt = 0;


  if (!done)
    {
      try
        {
          point_data.resize ((pnt_cnt + 1) * 3);
        }
      catch (std::bad_alloc&)
        {
          perror ("point_data");
          exit (-1);
        }

      try
        {
          point_color_data.resize ((pnt_cnt + 1) * 4);
        }
      catch (std::bad_alloc&)
        {
          perror ("point_color_data");
          exit (-1);
        }

      map_to_screen (x, y, z, &point_data[pnt_cnt * 3], &point_data[pnt_cnt * 3 + 1], &point_data[pnt_cnt * 3 + 2]);

      memcpy (&point_color_data[pnt_cnt * 4], point_color, 4);

      pnt_cnt++;
    }
  else
    {
      point_data_size = point_size;


      signalsEnabled = NVTrue;


      //  Generate a display list for the points.  We'll set point_count to pnt_cnt in generateDataPointsDL AFTER we've loaded the display list
      //  so that we don't try to draw a partial set of points due to a call to paintGL while we're loading the display list.

      generateDataPointsDL (pnt_cnt);


      pnt_cnt = 0;


      update_type = 1;
      updateGL ();
    }

  return (NVTrue);
}



//!  Clear "sparse" data points.

void 
nvMapGL::clearSparsePoints ()
{
  if (sparse_list) glDeleteLists (sparse_list, 1);

  if (sparse_count)
    {
      sparse_data.clear ();
      sparse_color_data.clear ();
    }

  sparse_count = 0;
  sparse_list = 0;
}



/*!

   - Makes a 3D point cloud from a subset of the points loaded (one at a time).  This is used to allow you
     to only display a sparse set of points instead of the entire set of points (from setDataPoints) when
     rotating or zooming.  For example, if you exceed a certain threshold number of points you would load
     both the full set of points (in setDataPoints) and a sparse set (like every 100th point) with this
     function.

   - Inputs:
                    - x             =    X position of a point (if projected this would be a longitude).
                    - y             =    Y position of a point (if projected this would be a latitude).
                    - z             =    Z position of a point (elevation).
                    - point_color   =    The color of the point.  This is an array of one byte (0-255) RGBA
                                         values.  For example, mostly transparent red would contain 255, 0,
                                         0, 64 in the [0], [1], [2], and [3] array values respectively.
                    - point_size    =    Size of the point in OpenGL units (the displayed OpenGL block is
                                         1.0 by 1.0 by 1.0 so this should be a pretty small number).
                                         This value is ignored until "done" is set!  You can't have different
                                         sized points.  Sorry.
                    - done          =    Set this to NVFalse until you run out of points to load.  At that
                                         time set it to NVTrue (with a reasonable point size) to display the 
                                         data.

   - Returns:       NVTrue

*/

uint8_t 
nvMapGL::setSparsePoints (double x, double y, double z, uint8_t *point_color, int32_t point_size, uint8_t done)
{
  //  Use this as the counter so that we don't try to draw the points (due to an update from something else)
  //  while we're still loading the array.

  static int32_t spr_cnt = 0;

  if (!done)
    {
      try
        {
          sparse_data.resize ((spr_cnt + 1) * 3);
        }
      catch (std::bad_alloc&)
        {
          perror ("sparse_data");
          exit (-1);
        }

      try
        {
          sparse_color_data.resize ((spr_cnt + 1) * 4);
        }
      catch (std::bad_alloc&)
        {
          perror ("sparse_color_data");
          exit (-1);
        }

      map_to_screen (x, y, z, &sparse_data[spr_cnt * 3], &sparse_data[spr_cnt * 3 + 1], &sparse_data[spr_cnt * 3 + 2]);

      memcpy (&sparse_color_data[spr_cnt * 4], point_color, 4);

      spr_cnt++;
    }
  else
    {
      sparse_data_size = point_size;


      signalsEnabled = NVTrue;


      //  Generate a display list for the points.  We'll set point_count to pnt_cnt in generateSparsePointsDL AFTER we've loaded the display list
      //  so that we don't try to draw a partial set of points due to a call to paintGL while we're loading the display list.

      generateSparsePointsDL (spr_cnt);
      spr_cnt = 0;


      update_type = 1;
      updateGL ();
    }

  return (NVTrue);
}



//!  Clear "feature" points.

void 
nvMapGL::clearFeaturePoints ()
{
  if (feature_count)
    {
      feature_data.clear ();
    }

  feature_count = 0;
}



/*!

   - Load "features" in to OpenGL.  These are 3D spheres at present.

   - Inputs:
                    - x             =    X position of a feature (if projected this would be a longitude).
                    - y             =    Y position of a feature (if projected this would be a latitude).
                    - z             =    Z position of a feature (elevation).
                    - feature_color =    The color of the feature.
                    - feature_size  =    Size of the point in OpenGL units (the displayed OpenGL block is
                                         1.0 by 1.0 by 1.0 so this should be a pretty small number).
                    - slices        =    Number of vertical slices used to define the feature (more makes it slower)
                    - stacks        =    Number of horizontal slices used to define the feature (more makes it slower)
                    - done          =    Set this to NVFalse until you run out of features to load.  At that
                                         time set it to NVTrue to display the features.

   - Returns:       NVTrue

*/

uint8_t 
nvMapGL::setFeaturePoints (double x, double y, double z, QColor feature_color, float feature_size,
                          int32_t slices, int32_t stacks, uint8_t done)
{
  //  Use this as the counter so that we don't try to draw the points (due to an update from something else)
  //  while we're still loading the array.

  static int32_t ftr_cnt = 0;


  if (!done)
    {
      try
        {
          feature_data.resize ((ftr_cnt + 1));
        }
      catch (std::bad_alloc&)
        {
          perror ("feature_data");
          exit (-1);
        }

      map_to_screen (x, y, z, &feature_data[ftr_cnt].v[0], &feature_data[ftr_cnt].v[1], &feature_data[ftr_cnt].v[2]);

      feature_data[ftr_cnt].color[0] = feature_color.red ();
      feature_data[ftr_cnt].color[1] = feature_color.blue ();
      feature_data[ftr_cnt].color[2] = feature_color.green ();
      feature_data[ftr_cnt].color[3] = feature_color.alpha ();
      feature_data[ftr_cnt].size = feature_size;
      feature_data[ftr_cnt].slices = slices;
      feature_data[ftr_cnt].stacks = stacks;

      ftr_cnt++;
    }
  else
    {
      feature_count = ftr_cnt;
      ftr_cnt = 0;

      update_type = 1;
      updateGL ();
    }

  return (NVTrue);
}



//!  Clear "marker" points.

void 
nvMapGL::clearMarkerPoints ()
{
  if (marker_count)
    {
      marker_data.clear ();
    }

  marker_count = 0;
}



/*!

   - Sets "marker" points.  These are little 3D crosses (like jacks) used to mark data points.

   - Inputs:
                    - x             =    X position of a marker (if projected this would be a longitude).
                    - y             =    Y position of a marker (if projected this would be a latitude).
                    - z             =    Z position of a marker (elevation).
                    - marker_color  =    The color of the marker.
                    - marker_size   =    Size of the point in OpenGL units (the displayed OpenGL block is
                                         1.0 by 1.0 by 1.0 so this should be a pretty small number).
                    - done          =    Set this to NVFalse until you run out of markers to load.  At that
                                         time set it to NVTrue to display the markers.

   - Returns:       NVTrue

*/

uint8_t 
nvMapGL::setMarkerPoints (double x, double y, double z, QColor marker_color, float marker_size, uint8_t done)
{
  //  Use this as the counter so that we don't try to draw the points (due to an update from something else)
  //  while we're still loading the array.

  static int32_t mrk_cnt = 0;


  if (!done)
    {
      try
        {
          marker_data.resize ((mrk_cnt + 1));
        }
      catch (std::bad_alloc&)
        {
          perror ("marker_data");
          exit (-1);
        }

      map_to_screen (x, y, z, &marker_data[mrk_cnt].v[0], &marker_data[mrk_cnt].v[1], &marker_data[mrk_cnt].v[2]);

      marker_data[mrk_cnt].color[0] = marker_color.red ();
      marker_data[mrk_cnt].color[1] = marker_color.blue ();
      marker_data[mrk_cnt].color[2] = marker_color.green ();
      marker_data[mrk_cnt].color[3] = marker_color.alpha ();
      marker_data[mrk_cnt].size = marker_size;

      mrk_cnt++;
    }
  else
    {
      marker_count = mrk_cnt;
      mrk_cnt = 0;

      update_type = 1;
      updateGL ();
    }

  return (NVTrue);
}



//!  Clear "lines".

void 
nvMapGL::clearLines ()
{
  if (line_count)
    {
      for (int32_t i = 0 ; i < line_count ; i++)
        {
          line_data[i].v.clear ();
        }

      line_data.clear ();
    }

  line_count = 0;
}



/*!

   - Sets "lines".  These are 3D lines drawn in the OpenGL space.

   - Inputs:
                    - x             =    X position of a line vertex (if projected this would be a longitude).
                    - y             =    Y position of a line vertex (if projected this would be a latitude).
                    - z             =    Z position of a line vertex (elevation).
                    - line_color    =    The color of the line segment.
                    - line_width    =    Width of the line in pixels.
                    - penStyle      =    Qt pen style (for example, Qt::SolidLine)
                    - point_num     =    The point number in the line (starting at 0).
                    - done          =    Set this to NVFalse until you run out of line vertices.  At that
                                         time set it to NVTrue to display the line.

   - Returns:       NVTrue

*/

uint8_t 
nvMapGL::setLines (double x, double y, double z, QColor line_color, int32_t line_width, Qt::PenStyle penStyle, int32_t point_num, uint8_t done)
{
  float sx, sy, sz;

  map_to_screen (x, y, z, &sx, &sy, &sz);

  return (setLines (sx, sy, sz, line_color, line_width, penStyle, point_num, done));
}



/*!

   - Sets "lines".  These are 3D lines drawn in the OpenGL space.

   - Inputs:
                    - x             =    X position of a line vertex (if projected this would be a longitude).
                    - y             =    Y position of a line vertex (if projected this would be a latitude).
                    - z             =    Z position of a line vertex (elevation).  Note that this is a 32 bit value.
                    - line_color    =    The color of the line segment.
                    - line_width    =    Width of the line in pixels.
                    - penStyle      =    Qt pen style (for example, Qt::SolidLine)
                    - point_num     =    The point number in the line (starting at 0).
                    - done          =    Set this to NVFalse until you run out of line vertices.  At that
                                         time set it to NVTrue to display the line.

   - Returns:       NVTrue

*/

uint8_t 
nvMapGL::setLines (float x, float y, float z, QColor line_color, int32_t line_width, Qt::PenStyle penStyle, int32_t point_num, uint8_t done)
{
  //  Use this as the counter so that we don't try to draw the lines (due to an update from something else)
  //  while we're still loading the array.

  static int32_t lin_cnt = -1;


  if (!done)
    {
      //  First point in a line, we need to allocate another line.

      if (!point_num)
        {
          lin_cnt++;

          try
            {
              line_data.resize (lin_cnt + 1);
            }
          catch (std::bad_alloc&)
            {
              perror ("line_data");
              exit (-1);
            }

          line_data[lin_cnt].count = 0; 
          line_data[lin_cnt].color[0] = line_color.red ();
          line_data[lin_cnt].color[1] = line_color.blue ();
          line_data[lin_cnt].color[2] = line_color.green ();
          line_data[lin_cnt].color[3] = line_color.alpha ();
          line_data[lin_cnt].width = line_width;
          line_data[lin_cnt].stipple = penStyle_to_stipple (penStyle);
        }


      try
        {
          line_data[lin_cnt].v.resize (line_data[lin_cnt].count + 1);
        }
      catch (std::bad_alloc&)
        {
          perror ("line_data[lin_cnt].v");
          exit (-1);
        }

      line_data[lin_cnt].v[line_data[lin_cnt].count].x = x;
      line_data[lin_cnt].v[line_data[lin_cnt].count].y = y;
      line_data[lin_cnt].v[line_data[lin_cnt].count].z = z;

      line_data[lin_cnt].count++;
    }
  else
    {
      line_count = lin_cnt + 1;
      lin_cnt = -1;

      update_type = 1;
      updateGL ();
    }

  return (NVTrue);
}



//!  Set to either NVMAPGL_BIN_MODE, NVMAPGL_POINT_MODE, NVMAPGL_SPARSE_MODE, or NVMAPGL_FEATURE_MODE.

void 
nvMapGL::setMapMode (int32_t mode)
{
  map.mode = mode;
}



//!  For some reason the parent widget's focusInEvent causes a paintGL call in Windoze.

void 
nvMapGL::enableWindozePaintEvent (uint8_t enable)
{
  windoze_paint_event_enabled = enable;
}



//!  Rotate the view around the OpenGL X axis.

void 
nvMapGL::rotateX (float angle)
{
  //  Translate to the center of the view prior to rotating

  glTranslatef (-centerX, -centerY, -centerZ);


  //  Save the angle for use in other places.

  x_rotation = fmod (x_rotation + angle, 360.0);


  //  Using quaternions to handle rotation (no gimbal lock and faster than matrices).  
  //  Maybe I'll understand this someday ;-)

  float a = angle * NV_DEG_TO_RAD;
  SQuat q1 = SQuat (1.0, 0.0, 0.0, a);

  Matrix3 m3 = Matrix3 (q1);

  m3.Get1DMatrix4 (x_matrix);


  /*  This is the old fashioned way using matrices.

    Rotation about the X axis by an angle a:

    |   1        0     0        0|
    |   0     cos(a)  sin(a)    0|
    |   0    -sin(a)  cos(a)    0|
    |   0        0     0        1|

  m[0] = 1.0;
  m[1] = 0.0;
  m[2] = 0.0;
  m[3] = 0.0;
  m[4] = 0.0;
  m[5] = cos (angle * NV_DEG_TO_RAD);
  m[6] = -sin (angle * NV_DEG_TO_RAD);
  m[7] = 0.0;
  m[8] = 0.0;
  m[9] = -m[6];
  m[10] = m[5];
  m[11] = 0.0;
  m[12] = 0.0;
  m[13] = 0.0;
  m[14] = 0.0;
  m[15] = 1.0;
  */

  glMultMatrixf (x_matrix);


  //  Translate back after rotating

  glTranslatef (centerX, centerY, centerZ);


  update_type = 257;
  updateGL ();
}



//!  Rotate the view around the OpenGL Y axis.

void 
nvMapGL::rotateY (float angle)
{
  //  Translate to the center of the view prior to rotating

  glTranslatef (-centerX, -centerY, -centerZ);


  //  Save the angle for use in other places.

  y_rotation = fmod (y_rotation + angle, 360.0);


  //  Using quaternions to handle rotation (no gimbal lock and faster than matrices).  
  //  Maybe I'll understand this someday ;-)

  float a = angle * NV_DEG_TO_RAD;
  SQuat q1 = SQuat (0.0, 1.0, 0.0, a);

  Matrix3 m3 = Matrix3 (q1);

  m3.Get1DMatrix4 (y_matrix);


  /*  This is the old fashioned way using matrices.

    Rotation about the Y axis by an angle a:

    |cos(a)      0   sin(a)    0|
    |   0        1     0       0|
    |-sin(a)     0   cos(a)    0|
    |   0        0     0       1|

  m[0] = cos (angle * NV_DEG_TO_RAD);
  m[1] = 0.0;
  m[2] = -sin (angle * NV_DEG_TO_RAD);
  m[3] = 0.0;
  m[4] = 0.0;
  m[5] = 1.0;
  m[6] = 0.0;
  m[7] = 0.0;
  m[8] = -m[2];
  m[9] = 0.0;
  m[10] = m[0];
  m[11] = 0.0;
  m[12] = 0.0;
  m[13] = 0.0;
  m[14] = 0.0;
  m[15] = 1.0;
  */

  glMultMatrixf (y_matrix);


  //  Translate back after rotating

  glTranslatef (centerX, centerY, centerZ);


  drawScale ();


  update_type = 257;
  updateGL ();
}



//!  Rotate the view around the OpenGL Z axis.

void 
nvMapGL::rotateZ (float angle)
{
  //  Translate to the center of the view prior to rotating

  glTranslatef (-centerX, -centerY, -centerZ);


  //  Save the angle for use in other places.

  z_rotation = fmod (z_rotation + angle, 360.0);


  //  Using quaternions to handle rotation (no gimbal lock and faster than matrices).  
  //  Maybe I'll understand this someday ;-)

  Matrix3 m3 = Matrix3 (*rotation);

  m3.Get1DMatrix4 (z_matrix);


  /*  This is the old fashioned way using matrices.

    Rotation about the Z axis by an angle a:

    |cos(a)  -sin(a)   0       0|
    |sin(a)   cos(a)   0       0|
    |   0        0     1       0|
    |   0        0     0       1|

  m[0] = cos (angle * NV_DEG_TO_RAD);
  m[1] = sin (angle * NV_DEG_TO_RAD);
  m[2] = 0.0;
  m[3] = 0.0;
  m[4] = -m[1];
  m[5] = m[0];
  m[6] = 0.0;
  m[7] = 0.0;
  m[8] = 0.0;
  m[9] = 0.0;
  m[10] = 1.0;
  m[11] = 0.0;
  m[12] = 0.0;
  m[13] = 0.0;
  m[14] = 0.0;
  m[15] = 1.0;
  */

  glMultMatrixf (z_matrix);


  //  Translate back after rotating

  glTranslatef (centerX, centerY, centerZ);


  update_type = 257;
  updateGL ();
}



//!  This rotates around an axis that is parallel to the screen face and passes through the point 0,0,0.

void 
nvMapGL::rotateZX (float angle)
{
  //  Translate to the center of the view prior to rotating

  glTranslatef (-centerX, -centerY, -centerZ);


  //  Save the rotation so we can figure out which side of the surface is visible (in paintGL).

  zx_rotation = fmod (zx_rotation + angle, 360.0);


  //  This is where we define the vector u.  There is (obviously) no Y component.
  //  The X and Z components are based on the amount of rotation of the Y axis.

  Vector3 u = Vector3 (cos (y_rotation * NV_DEG_TO_RAD), 0.0, sin (y_rotation * NV_DEG_TO_RAD));


  //  Using quaternions to handle rotation (no gimbal lock and faster than matrices).  
  //  Maybe I'll understand this someday ;-)

  float a = angle * NV_DEG_TO_RAD;
  SQuat q1 = SQuat (u, a);

  Matrix3 m3 = Matrix3 (q1);

  m3.Get1DMatrix4 (zx_matrix);


  /*  This is the old fashioned way using matrices.

  //  This is where we define the vector u.  There is (obviously) no Y component.
  //  The X and Z components are based on the amount of rotation of the Y axis.

  double u[3];

  u[0] = cos (y_rotation * NV_DEG_TO_RAD);
  u[1] = 0.0;
  u[2] = sin (y_rotation * NV_DEG_TO_RAD);


  //  The following matrix allows rotation about an arbitrary axis.
  //  It requires a unit vector, (ux,uy,uz) pointing in the direction of the axis.

  //  Rotation about an arbitrary axis (described by vector u) by an angle a:

  //  | m[00]   m[04]   m[08]   m[12] |
  //  | m[01]   m[05]   m[09]   m[13] |
  //  | m[02]   m[06]   m[10]   m[14] |
  //  | m[03]   m[07]   m[11]   m[15] |

  //  Where:

  //  m[00] = 1.0 + (1.0 - cos_theta) * (u[x] * u[x] - 1.0)
  //  m[01] = (1.0 - cos_theta) * u[x] * u[y] + u[z] * sin_theta
  //  m[02] = (1.0 - cos_theta) * u[x] * u[z] - u[y] * sin_theta
  //  m[03] = 0.0
  //  m[04] = (1.0 - cos_theta) * u[x] * u[y] - u[z] * sin_theta
  //  m[05] = 1.0 + (1.0 - cos_theta) * (u[y] * u[y] - 1.0)
  //  m[06] = (1.0 - cos_theta) * u[y] * u[z] + u[x] * sin_theta
  //  m[07] = 0.0
  //  m[08] = (1.0 - cos_theta) * u[x] * u[z] + u[y] * sin_theta
  //  m[09] = (1.0 - cos_theta) * u[y] * u[z] - u[x] * sin_theta
  //  m[10] = 1.0 + (1.0 - cos_theta) * (u[z] * u[z] - 1.0)
  //  m[11] = 0.0
  //  m[12] = 0.0
  //  m[13] = 0.0
  //  m[14] = 0.0
  //  m[15] = 1.0


  //  Pre-compute these so we don't have to do it over and over.

  double sin_theta = sin (angle * NV_DEG_TO_RAD);
  double cos_theta = cos (angle * NV_DEG_TO_RAD);


  //  Define the matrix.  Note that we could have simplified this because all of the
  //  u[1] components (i.e. u[y]) are 0.0.  It's left in the normal form so that I can
  //  easily steal the code later ;-)

  m[0] = 1.0 + (1.0 - cos_theta) * (u[0] * u[0] - 1.0);
  m[1] = (1.0 - cos_theta) * u[0] * u[1] + u[2] * sin_theta;
  m[2] = (1.0 - cos_theta) * u[0] * u[2] - u[1] * sin_theta;
  m[3] = 0.0;
  m[4] = (1.0 - cos_theta) * u[0] * u[1] - u[2] * sin_theta;
  m[5] = 1.0 + (1.0 - cos_theta) * (u[1] * u[1] - 1.0);
  m[6] = (1.0 - cos_theta) * u[1] * u[2] + u[0] * sin_theta;
  m[7] = 0.0;
  m[8] = (1.0 - cos_theta) * u[0] * u[2] + u[1] * sin_theta;
  m[9] = (1.0 - cos_theta) * u[1] * u[2] - u[0] * sin_theta;
  m[10] = 1.0 + (1.0 - cos_theta) * (u[2] * u[2] - 1.0);
  m[11] = 0.0;
  m[12] = 0.0;
  m[13] = 0.0;
  m[14] = 0.0;
  m[15] = 1.0;
  */

  glMultMatrixf (zx_matrix);


  //  Translate back after rotating

  glTranslatef (centerX, centerY, centerZ);


  drawScale ();

  update_type = 257;
  updateGL ();
}



//!  Set the X axis rotation.

void 
nvMapGL::setXRotation (float angle __attribute__ ((unused)))
{
  rotateX (-x_rotation + angle);
}



//!  Set the Y axis rotation.

void 
nvMapGL::setYRotation (float angle __attribute__ ((unused)))
{
  rotateY (-y_rotation + angle);
}



//!  Set the Z axis rotation.

void 
nvMapGL::setZRotation (float angle __attribute__ ((unused)))
{
  rotateZ (-z_rotation + angle);
}



//!  Set the ZX axis rotation (around a line parallel to the screen that passes through 0,0,0).

void 
nvMapGL::setZXRotation (float angle __attribute__ ((unused)))
{
  rotateZX (-zx_rotation + angle);
}



//!  Return the current X axis rotation value.

float 
nvMapGL::getXRotation ()
{
  return (x_rotation);
}



//!  Return the current Y axis rotation value.

float 
nvMapGL::getYRotation ()
{
  return (y_rotation);
}



//!  Return the current Z axis rotation value.

float 
nvMapGL::getZRotation ()
{
  return (z_rotation);
}



//!  Return the current ZX axis rotation value (around a line parallel to the screen that passes through 0,0,0).

float 
nvMapGL::getZXRotation ()
{
  return (zx_rotation);
}



//!  Set the map center point using map coordinates.

void 
nvMapGL::setMapCenter (double map_x, double map_y, float map_z)
{
  setMapCenter (map_x, map_y, (double) map_z);
}



//!  Set the map center point using map coordinates.

void 
nvMapGL::setMapCenter (double map_x, double map_y, double map_z)
{
  float vertex_x, vertex_y, vertex_z;

  map_center_x = map_x;
  map_center_y = map_y;
  map_center_z = map_z;

  map_to_screen (map_x, map_y, map_z, &vertex_x, &vertex_y, &vertex_z);

  setCenter (vertex_x, vertex_y, vertex_z);
}



//!  Set the map center point using OpenGL coordinates.

void 
nvMapGL::setCenter (float x, float y, float z)
{
  centerX = -x;
  centerY = -y;
  centerZ = -z;


  glTranslatef (centerX - prev_centerX, centerY - prev_centerY, centerZ - prev_centerZ);


  prev_centerX = centerX;
  prev_centerY = centerY;
  prev_centerZ = centerZ;


  update_type = 1;
  updateGL ();
}



//!  Return the map center in map coordinates.

void 
nvMapGL::getMapCenter (double *map_x, double *map_y, double *map_z)
{
  screen_to_map (centerX, centerY, centerZ, map_x, map_y, map_z);
}



/*******************************  IMPORTANT NOTE  ***********************************************

    There can only be MAX_MOVABLE_OBJECTS of the following rubberband or move operations active
    at any given time.  It is best to "discard" or "close" rubberband or moving objects prior
    to starting new ones.  Note that redrawMap and redrawMapArea will automatically discard any
    active rubberband or moving objects.

************************************************************************************************/

//!  Create a "movable" object.  See NVMAPGL_MOVABLE_OBJECT for details.

int32_t nvMapGL::create_movable_object (NVMAPGL_MOVABLE_OBJECT mo)
{
  for (int32_t i = 0 ; i < MAX_MOVABLE_OBJECTS ; i++)
    {
      if (!movable_object[i].active)
        {
          movable_object[i] = mo;
          mot[movable_object_count] = &movable_object[i];
          movable_object_count++;
          return (i);
        }
    }

  return (-1);
}



//!  Get rid of a movable object.

void nvMapGL::discard_movable_object (int32_t *num)
{
  if (*num < 0 || *num >= MAX_MOVABLE_OBJECTS) return;


  //  This clears the old movable object without drawing a new one (see paintGL).

  update_type = -1;
  updateGL ();


  for (int32_t i = 0 ; i < MAX_MOVABLE_OBJECTS ; i++)
    {
      if (mot[i] == &movable_object[*num])
        {
          movable_object[*num].active = NVFalse;

          for (int32_t j = i ; j < movable_object_count - 1 ; j++) mot[j] = mot[j + 1];

          movable_object_count--;

          *num = -1;
        }
    }
}


//  *************************************************************************************
//
//  Rubberband functions
//
//  *************************************************************************************

/*!  Starting a rubberband polygon in screen space (2D).  Use for first point only.  If you want to draw
     a polyline instead of a closed polygon, set close to NVFalse.  */

void 
nvMapGL::anchorRubberbandPolygon (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, uint8_t close,
                                  Qt::PenStyle penStyle)
{
  NVMAPGL_MOVABLE_OBJECT mo;


  mo.color[0] = color.red ();
  mo.color[1] = color.green ();
  mo.color[2] = color.blue ();
  mo.color[3] = color.alpha ();
  mo.line_width = line_width;
  mo.stipple = penStyle_to_stipple (penStyle);
  mo.type = NVMAPGL_RUBBER_POLYGON;
  mo.close_polygon = close;
  mo.filled = NVFalse;
  mo.outlined = NVTrue;
  mo.active = NVTrue;
  mo.prev_polygon_count = 0;

  *num = create_movable_object (mo);


  if (*num >= 0)
    {
      movable_object[*num].polygon_x[0] = movable_object[*num].polygon_x[1] = rubberbandPolyLastX[*num] = x;
      movable_object[*num].polygon_y[0] = movable_object[*num].polygon_y[1] = rubberbandPolyLastY[*num] = y;

      movable_object[*num].polygon_count = 2;

      rubberbandPolyActive[*num] = NVTrue;
    }


  return;
}



//!  Dragging a line (Tommy James and the Shondells) from the previous vertex in a rubberband polygon (screen space).

void 
nvMapGL::dragRubberbandPolygon (int32_t num, int32_t x, int32_t y)
{
  if (rubberbandPolyActive[num] && (rubberbandPolyLastX[num] != x || rubberbandPolyLastY[num] != y))
    {
      //  Save for previous last point.

      movable_object[num].polygon_x[movable_object[num].polygon_count - 1] = rubberbandPolyLastX[num] = x;
      movable_object[num].polygon_y[movable_object[num].polygon_count - 1] = rubberbandPolyLastY[num] = y;

      update_type = 0;
      updateGL ();
    }
}



//!  Return the current polygon line segment anchor position in screen coordinates.

void 
nvMapGL::getRubberbandPolygonCurrentAnchor (int32_t num, int32_t *x, int32_t *y)
{
  if (num < 0) return;

  *x = movable_object[num].polygon_x[movable_object[num].polygon_count - 1];
  *y = movable_object[num].polygon_y[movable_object[num].polygon_count - 1];
}



//!  Add a new vertex to a rubberband polygon in screen space.

void 
nvMapGL::vertexRubberbandPolygon (int32_t num, int32_t x, int32_t y)
{
  movable_object[num].polygon_x[movable_object[num].polygon_count] = rubberbandPolyLastX[num] = x;
  movable_object[num].polygon_y[movable_object[num].polygon_count] = rubberbandPolyLastY[num] = y;

  movable_object[num].polygon_count++;

  if (movable_object[num].polygon_count >= NVMAPGL_POLYGON_POINTS)
    {
      fprintf (stderr, "You have exceeded the maximum number of polygon points (%d).", NVMAPGL_POLYGON_POINTS);
      fflush (stderr);
      discard_movable_object (&num);
      return;
    }
}



//!  Return the points in the current polygon (in screen space) without closing the polygon.

void 
nvMapGL::getRubberbandPolygon (int32_t num, int32_t *count, int32_t **px, int32_t **py)
{
  if (num < 0) return;

  *px = movable_object[num].polygon_x;
  *py = movable_object[num].polygon_y;
  *count = movable_object[num].polygon_count;
}



/*!  Close a rubberband polygon in screen space adding the input point (x, y).  Returns the
     points in the current polygon (in screen space).  The arguments
     px and py should be declared in your calling function as int32_t *px and 
     int32_t *py.  They should be passed to this function as &px, &py.  Remember to "discard"
     the polygon to free the associated allocated memory (returned as px, py).    */

void 
nvMapGL::closeRubberbandPolygon (int32_t num, int32_t x, int32_t y, int32_t *count, int32_t **px, int32_t **py)
{
  if (num < 0 || !rubberbandPolyActive[num]) return;


  rubberbandPolyActive[num] = NVFalse;

  movable_object[num].polygon_x[movable_object[num].polygon_count - 1] = rubberbandPolyLastX[num] = x;
  movable_object[num].polygon_y[movable_object[num].polygon_count - 1] = rubberbandPolyLastY[num] = y;
  //vertexRubberbandPolygon (num, x, y);


  getRubberbandPolygon (num, count, px, py);
}



//!  Returns whether a rubberband polygon is active (being created) at present.

uint8_t
nvMapGL::rubberbandPolygonIsActive (int32_t num)
{
  if (num < 0) return (NVFalse);

  return (rubberbandPolyActive[num]);
}



//!  Erase the last rubberband polygon and free the associated allocated memory.

void 
nvMapGL::discardRubberbandPolygon (int32_t *num)
{
  if (*num < 0 || !movable_object[*num].active) return;


  rubberbandPolyActive[*num] = NVFalse;


  discard_movable_object (num);
}



//!  Anchor a rubberband rectangle in screen space (2D).  Starting corner point.

void 
nvMapGL::anchorRubberbandRectangle (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, Qt::PenStyle penStyle)
{
  NVMAPGL_MOVABLE_OBJECT mo;

  mo.color[0] = color.red ();
  mo.color[1] = color.green ();
  mo.color[2] = color.blue ();
  mo.color[3] = color.alpha ();
  mo.line_width = line_width;

  mo.stipple = penStyle_to_stipple (penStyle);
  mo.type = NVMAPGL_RUBBER_RECTANGLE;
  mo.filled = NVFalse;
  mo.outlined = NVTrue;
  mo.active = NVTrue;
  mo.prev_rect.setCoords (-1, -1, -1, -1);


  *num = create_movable_object (mo);

  if (*num >= 0)
    {
      rubberbandRectX[*num][0] = rubberbandRectLastX[*num] = x;
      rubberbandRectY[*num][0] = rubberbandRectLastY[*num] = y;


      rubberbandRectActive[*num] = NVTrue;
    }

  return;
}



//!  Drag a rubberband rectangle in screen space.

void 
nvMapGL::dragRubberbandRectangle (int32_t num, int32_t x, int32_t y)
{
  if (rubberbandRectActive[num] && (rubberbandRectLastX[num] != x || rubberbandRectLastY[num] != y))
    {
      rubberbandRectLastX[num] = x;
      rubberbandRectLastY[num] = y;

      movable_object[num].rect.setCoords (rubberbandRectX[num][0], rubberbandRectY[num][0], x, y);

      update_type = 0;
      updateGL ();
    }
}



//!  Return the points in the current rectangle (in screen space) without closing the rectangle.

void 
nvMapGL::getRubberbandRectangle (int32_t num, int32_t **px, int32_t **py)
{
  if (num < 0) return;

  *px = rubberbandRectX[num];
  *py = rubberbandRectY[num];
}



/*!  Close a rubberband rectangle in screen space.  The ending corner is specified by the point
     x,y.  Returns the points in the rectangle as a 4 point polygon in screen space (px, py).
     The arguments px, and should be declared in your calling function as int32_t *px.
     They should be passed to this function as &px.  */

void 
nvMapGL::closeRubberbandRectangle (int32_t num, int32_t x, int32_t y, int32_t **px, int32_t **py)
{
  if (num < 0 || !rubberbandRectActive[num]) return;


  rubberbandRectActive[num] = NVFalse;


  rubberbandRectX[num][1] = rubberbandRectX[num][0];
  rubberbandRectY[num][1] = y;
  rubberbandRectX[num][2] = x;
  rubberbandRectY[num][2] = y;
  rubberbandRectX[num][3] = x;
  rubberbandRectY[num][3] = rubberbandRectY[num][0];


  *px = rubberbandRectX[num];
  *py = rubberbandRectY[num];
}



//!  Return whether a rubberband rectangle is active.

uint8_t 
nvMapGL::rubberbandRectangleIsActive (int32_t num)
{
  if (num < 0) return (NVFalse);

  return (rubberbandRectActive[num]);
}



//!  Return the map anchor position of the current active rectangle.

void 
nvMapGL::getRubberbandRectangleAnchor (int32_t num, int32_t *px, int32_t *py)
{
  if (num < 0) return;

  *px = rubberbandRectX[num][0];
  *py = rubberbandRectY[num][0];
}



//!  Erase the rubberband rectangle.

void 
nvMapGL::discardRubberbandRectangle (int32_t *num)
{
  if (*num < 0 || !movable_object[*num].active) return;


  rubberbandRectActive[*num] = NVFalse;


  discard_movable_object (num);
}



//!  Anchor a rubberband line in screen space.  Start point.

void 
nvMapGL::anchorRubberbandLine (int32_t *num, int32_t x, int32_t y, QColor color, int32_t line_width, Qt::PenStyle penStyle)
{
  NVMAPGL_MOVABLE_OBJECT mo;


  mo.color[0] = color.red ();
  mo.color[1] = color.green ();
  mo.color[2] = color.blue ();
  mo.color[3] = color.alpha ();
  mo.line_width = line_width;
  mo.stipple = penStyle_to_stipple (penStyle);
  mo.type = NVMAPGL_RUBBER_LINE;
  mo.filled = NVFalse;
  mo.outlined = NVTrue;
  mo.active = NVTrue;
  mo.prev_x0 = -1;

  *num = create_movable_object (mo);


  if (*num >= 0)
    {
      rubberbandLineAnchorX[*num] = x;
      rubberbandLineAnchorY[*num] = y;

      rubberbandLineLastX[*num] = x;
      rubberbandLineLastY[*num] = y;

      rubberbandLineActive[*num] = NVTrue;
    }


  return;
}



//!  Drag a rubberband line in screen space.

void 
nvMapGL::dragRubberbandLine (int32_t num, int32_t x, int32_t y)
{
  if (rubberbandLineActive[num] && (rubberbandLineLastX[num] != x || rubberbandLineLastY[num] != y))
    {
      movable_object[num].x0 = rubberbandLineAnchorX[num];
      movable_object[num].y0 = rubberbandLineAnchorY[num];
      movable_object[num].x1 = x;
      movable_object[num].y1 = y;

      update_type = 0;
      updateGL ();

      rubberbandLineLastX[num] = x;
      rubberbandLineLastY[num] = y;
    }
}



//!  Return whether a rubberband line is active.

uint8_t 
nvMapGL::rubberbandLineIsActive (int32_t num)
{
  if (num < 0) return (NVFalse);

  return (rubberbandLineActive[num]);
}



//!  Erase the rubberband line.

void 
nvMapGL::discardRubberbandLine (int32_t *num)
{
  if (*num < 0 || !movable_object[*num].active) return;


  rubberbandLineActive[*num] = NVFalse;


  discard_movable_object (num);
}



//  *************************************************************************************
//
//  Moving object functions
//
//  *************************************************************************************


/*!

    - Move a display list made up of line segments in screen space.  Here's an example of how you
      would set up a list:

    <pre>

    marker[0].x = -MARKER_W;
    marker[0].y = -MARKER_H;
    marker[1].x = MARKER_W;
    marker[1].y = -MARKER_H;
    marker[2].x = MARKER_W;
    marker[2].y = -MARKER_H;
    marker[3].x = MARKER_W;
    marker[3].y = MARKER_H;
    marker[4].x = MARKER_W;
    marker[4].y = MARKER_H;
    marker[5].x = -MARKER_W;
    marker[5].y = MARKER_H;
    marker[6].x = -MARKER_W;
    marker[6].y = MARKER_H;
    marker[7].x = -MARKER_W;
    marker[7].y = -MARKER_H;
    marker[8].x = -MARKER_W;
    marker[8].y = 0.0;
    marker[9].x = -3.0;
    marker[9].y = 0.0;
    marker[10].x = 3.0;
    marker[10].y = 0.0;
    marker[11].x = MARKER_W;
    marker[11].y = 0.0;
    marker[12].x = 0.0;
    marker[12].y = -MARKER_H;
    marker[13].x = 0.0;
    marker[13].y = -3.0;
    marker[14].x = 0.0;
    marker[14].y = 3.0;
    marker[15].x = 0.0;
    marker[15].y = MARKER_H;


    This creates a little target kind of thing that looks something like this:

    _______________
    |      |      |
    |      |      |
    |----     ----|
    |      |      |
    |      |      |
    ---------------

    </pre>


    - The list will be drawn with its center at x,y.  You can rotate the list by
      setting the angle value.

    - Use an NV_C_RGBA (instead of a QColor)

*/

void 
nvMapGL::setMovingList (int32_t *num, NV_I32_COORD2 *list_data, int32_t line_count, int32_t x, int32_t y, double angle,
                        int32_t line_width, NV_C_RGBA color)
{
  QColor c = QColor (color.r, color.g, color.b, color.a);

  setMovingList (num, list_data, line_count, x, y, angle, line_width, c);
}



/*!  Move a list in screen space.  See above for example.  The list will be drawn with its center
     at x,y.  You can rotate the list by setting the angle value.  */

void 
nvMapGL::setMovingList (int32_t *num, NV_I32_COORD2 *list_data, int32_t line_count, int32_t x, int32_t y, double angle,
                        int32_t line_width, QColor color)
{
  //  If the object number is less than zero it has been discarded and, thus, needs to be created.

  if (*num < 0)
    {
      NVMAPGL_MOVABLE_OBJECT mo;

      mo.line_width = line_width;
      mo.type = NVMAPGL_MOVE_LIST;
      mo.rotation_angle = angle;
      mo.polygon_count = line_count;
      mo.color[0] = color.red ();
      mo.color[1] = color.green ();
      mo.color[2] = color.blue ();
      mo.color[3] = color.alpha ();
      mo.prev_polygon_count = 0;

      for (int32_t i = 0 ; i < line_count ; i++)
        {
          mo.polygon_x[i] = list_data[i].x + x;
          mo.polygon_y[i] = list_data[i].y + y;
        }

      mo.active = NVTrue;

      *num = create_movable_object (mo);
    }
  else
    {
      for (int32_t i = 0 ; i < line_count ; i++)
        {
          movable_object[*num].polygon_x[i] = list_data[i].x + x;
          movable_object[*num].polygon_y[i] = list_data[i].y + y;
        }
      movable_object[*num].rotation_angle = angle;
    }

  update_type = 0;
  updateGL ();
}



//!  Close a moving list.

void 
nvMapGL::closeMovingList (int32_t *num)
{
  if (*num < 0 || !movable_object[*num].active) return;

  discard_movable_object (num);
}



//  *************************************************************************************
//
//  Normal drawing functions
//
//  *************************************************************************************


//!  Clear all text objects.

void 
nvMapGL::clearText ()
{
  text_object_count = 0;

  //updateGL ();
}



//!  Draw text with a default font of "Charter"

void 
nvMapGL::drawText (QString string, double map_x, double map_y, double map_z, float angle, 
                   int32_t size, QColor color, uint8_t flush)
{
  drawText (string, map_x, map_y, map_z, angle, "Charter", size, color, flush);
}



//!  Draw text with a caller specified font (see QFont docs for info on setting font family names)

void 
nvMapGL::drawText (QString string, double map_x, double map_y, double map_z, float angle, 
                   QString font, int32_t size, QColor color, uint8_t flush)
{
  float vertex_x, vertex_y, vertex_z;


  if (text_object_count > MAX_GL_TEXT_OBJECTS) 
    {
      fprintf (stderr, "Too many text objects!\n");
      return;
    }


  map_to_screen (map_x, map_y, map_z, &vertex_x, &vertex_y, &vertex_z);


  QFont qfont (font, size);

  text_object[text_object_count].string = "   " + string;
  text_object[text_object_count].x = vertex_x;
  text_object[text_object_count].y = vertex_y;
  text_object[text_object_count].z = vertex_z;
  text_object[text_object_count].color[0] = (float) color.red () / 255.0;
  text_object[text_object_count].color[1] = (float) color.green () / 255.0;
  text_object[text_object_count].color[2] = (float) color.blue () / 255.0;
  text_object[text_object_count].color[3] = (float) color.alpha () / 255.0;
  text_object[text_object_count].angle = angle;
  text_object[text_object_count].font = qfont;


  text_object_count++;


  if (flush)
    {
      update_type = 1;
      updateGL ();
    }
}



/*!
     - Pay attention troops!!!  Nothing actually happens here until you set the "flush" argument to NVTrue.  We just keep
       loading up an array in memory.  If you don't flush it when you're done making line segments you'll just crash.
       The purpose of this function is to allow you to overlay a bunch of markers on the data but have them show up 
       in 2D in front of the 3D data.  By just allocating memory and then dumping it in one fell swoop (or swell foop if
       you will) we save shitloads (that's a technical term) of time.  Note that the last call you make, with "flush" set
       to NVTrue, isn't loaded up.  That is, you don't have to make sure you set "flush" to NVTrue on the last iteration
       of whatever loop you're using to do the drawing.  Just call it with a bunch of bogus values and "flush" set to
       NVTrue.  Also, this stuff is drawn into the buffers right here.  All other drawing is done in paintGL so, the next
       time paintGl gets called, this stuff will be erased.  You have to draw it again to make it show back up.  I use
       this to put marks on specific data points after I have rotated, zoomed, or redrawn the scene.

     - IMPORTANT NOTE:  Normally we would use this to mark data points in a point cloud but now we use setMarkerPoints
       for that since the points will rotate.  This isn't really used now AFAIK.
*/

void
nvMapGL::drawLine (int32_t x0, int32_t y0, int32_t x1, int32_t y1, QColor color, int32_t line_width, Qt::PenStyle penStyle, uint8_t flush)
{
  static NVMAPGL_2DLINE_OBJECT *line = NULL;
  static int32_t line_count = 0;

  if (!flush)
    {
      line = (NVMAPGL_2DLINE_OBJECT *) realloc (line, (line_count + 1) * sizeof (NVMAPGL_2DLINE_OBJECT));

      if (line == NULL)
        {
          perror ("Allocating line memory in drawLine");
          exit (-1);
        }

      line[line_count].x0 = x0;
      line[line_count].x1 = x1;
      line[line_count].y0 = y0;
      line[line_count].y1 = y1;
      line[line_count].width = line_width;
      line[line_count].color[0] = color.red ();
      line[line_count].color[1] = color.green ();
      line[line_count].color[2] = color.blue ();
      line[line_count].color[3] = color.alpha ();
      line[line_count].stipple = penStyle_to_stipple (penStyle);

      line_count++;
    }
  else
    {
      if (!line_count) return;


      //  Must use front buffer on Windoze except for the rotate and zoom stuff.

#ifdef NVWIN3X
      glDrawBuffer (GL_FRONT);
#else
      glDrawBuffer (GL_BACK);
#endif


      //  Save the current line width.

      glPushAttrib (GL_LINE_BIT);


      //  Turn off depth buffering for the moment

      if (depth_test) glDisable (GL_DEPTH_TEST);


      //  Save the original matrix

      glPushMatrix ();


      //  Switch to 2D Orthographic projection

      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      gluOrtho2D (0, map.draw_width, 0, map.draw_height);


      //  Go to model view and load the identity matrix

      glMatrixMode (GL_MODELVIEW);
      glLoadIdentity ();


      for (int32_t i = 0 ; i < line_count ; i++)
        {
          //  Set the color

          glColor4ubv (line[i].color);


          //  Set the line width

          glLineWidth ((GLfloat) line[i].width);


          //  Set the stipple pattern

          glLineStipple (2, line[i].stipple);


          //  Draw the line.  We have to flip the Y coordinates because of the way OpenGL handles Y.

          glBegin (GL_LINES);
          {
            int32_t y = map.draw_height - line[i].y0;
            glVertex2i (line[i].x0, y);
            y = map.draw_height - line[i].y1;
            glVertex2i (line[i].x1, y);
          }
          glEnd ();
        }


      //  Free the memory for the line data.

      if (line_count)
        {
          free (line);
          line = NULL;
          line_count = 0;
        }


      //  Go back to perspective projection

      glMatrixMode (GL_PROJECTION);
      glLoadIdentity ();
      gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


      //  Set the matrix mode to modelview.

      glMatrixMode (GL_MODELVIEW);


      //  Restore the original matrix

      glPopMatrix ();


      //  Turn depth buffering back on

      if (depth_test) glEnable (GL_DEPTH_TEST);


      //  Restore the line width

      glPopAttrib ();


      //  Since we're not double buffering on Windoze, if we got to this point, we need to flush the front buffer.

#ifdef NVWIN3X
      glFlush ();
#else

      //  Manually swap the (just drawn) back buffer to the front.

      swapBuffers ();

#endif


      //  Check for errors (OpenGL is really picky).

      //if (int32_t err = glGetError ()) fprintf (stderr, "Some sort of GL error occurred in drawLine : %d\n", err);
    }
}



//!  Clear all display lists

void 
nvMapGL::clearDisplayLists ()
{
  for (uint32_t i = 0 ; i < list_count ; i++) glDeleteLists (display_list[i], 1);

  if (display_list != NULL) free (display_list);

  display_list = NULL;
  list_count = 0;

  //updateGL ();
}



//!  Draw a sphere in map space.  Radius is in OpenGL units (the whole display is 1.0 by 1.0 by 1.0 so use something small like 0.01)

void 
nvMapGL::drawSphere (double map_x, double map_y, double map_z, double radius, int32_t slices, int32_t stacks, QColor color, uint8_t flush)
{
  float vertex_x, vertex_y, vertex_z;
  GLfloat glcolor[4];

  glcolor[0] = (GLfloat) color.red () / 255.0;
  glcolor[1] = (GLfloat) color.green () / 255.0;
  glcolor[2] = (GLfloat) color.blue () / 255.0;
  glcolor[3] = (GLfloat) color.alpha () / 255.0;


  map_to_screen (map_x, map_y, map_z, &vertex_x, &vertex_y, &vertex_z);


  display_list = (GLuint *) realloc (display_list, (list_count + 1) * sizeof (GLuint));
  if (display_list == NULL)
    {
      perror ("Allocating display_list memory in drawSphere");
      exit (-1);
    }

  display_list[list_count] = glGenLists (1);

  if (display_list[list_count] == 0)
    {
      fprintf (stderr, "Unable to generate display list in drawSphere\n");
      exit (-1);
    }


  glNewList (display_list[list_count], GL_COMPILE);


  glPushMatrix ();

  GLUquadricObj *quadric = gluNewQuadric ();

  gluQuadricNormals (quadric, GLU_SMOOTH);

  gluQuadricDrawStyle (quadric, GLU_FILL);
  glColor4fv (glcolor);
  glTranslatef (vertex_x, vertex_y, vertex_z);
  gluSphere (quadric, radius, slices, stacks);

  gluDeleteQuadric (quadric);

  glPopMatrix ();


  glEndList ();


  list_count++;


  if (flush)
    {
      update_type = 1;
      updateGL ();
    }
}




// *************************************************************************************
//
//  Utility functions
//
// *************************************************************************************


// *************************************************************************************
/*!

  - VERY IMPORTANT NOTE:  In the map world (map_x, map_y, map_z) X is left/right, Y is
    up/down,and Z is elevation  (negative values indicate depth).  In the OpenGL world
    (vertex_x, vertex_y, vertex_z) X is left/right, Y is elevation, and Z is down/up
    (inverted).   Confusing, isn't it?  Positive Z is toward the user, positive X is to
    the right, and positive Y is up.  So, you must remember that map_z will correspond
    (via some conversion factor) to negative vertex_y.  In addition, map_y will
    correspond (again via some conversion factor) to vertex_z.  We'll deal with map_y
    to vertex_z and vice-versa internally but you'll have to flip the sign on map_z if
    you are using depths instead of elevations.  This caveat applies to map_to_screen
    and screen_to_map.  The variables "range_x", "range_y", "range_z", and "bounds" are
    based on map/elevation coordinates.  The variables "vertex_x", "vertex_y", and
    "vertex_z" are based on OpenGL coordinates.  Note that vertex_z is computed from
    map_y, bounds, and range_y.


  - Convert map coordinates to OpenGL coordinates (vertices from 0.0 to 1.0).

  *************************************************************************************/

void 
nvMapGL::map_to_screen (int32_t count, double *map_x, double *map_y, double *map_z, float *vertex_x, float *vertex_y, float *vertex_z)
{
  for (int32_t i = 0 ; i < count ; i++) map_to_screen (map_x[i], map_y[i], map_z[i], &vertex_x[i], &vertex_y[i], &vertex_z[i]);
}


//!  Convert map coordinates to OpenGL coordinates (vertices from 0.0 to 1.0).

void 
nvMapGL::map_to_screen (double map_x, double map_y, double map_z, float *vertex_x, float *vertex_y, float *vertex_z)
{
  *vertex_x = (float) ((((map_x - bounds.min_x) / range_x) - 0.5) * exag_scale);
  *vertex_z = (float) ((((bounds.max_y - map_y) / range_y) - 0.5) * exag_scale);
  *vertex_y = (float) ((((map_z - bounds.min_z) / range_z) - 0.5) * z_scale);
}


//!  Convert map coordinates to OpenGL coordinates (vertices from 0.0 to 1.0).

void 
nvMapGL::map_to_screen (double map_x, double map_y, float map_z, float *vertex_x, float *vertex_y, float *vertex_z)
{
  map_to_screen (map_x, map_y, (double) map_z, vertex_x, vertex_y, vertex_z);
}


//!  Convert OpenGL coordinates to map coordinates.

void 
nvMapGL::screen_to_map (int32_t count, float *vertex_x, float *vertex_y, float *vertex_z, double *map_x, double *map_y, double *map_z)
{
  for (int32_t i = 0 ; i < count ; i++) screen_to_map (vertex_x[i], vertex_y[i], vertex_z[i], &map_x[i], &map_y[i], &map_z[i]);
}


//!  Convert OpenGL coordinates to map coordinates.

void 
nvMapGL::screen_to_map (float vertex_x, float vertex_y, float vertex_z, double *map_x, double *map_y, double *map_z)
{
  *map_x = ((((double) vertex_x / exag_scale) + 0.5) * range_x) + bounds.min_x;
  *map_z = (((double) vertex_y / z_scale + 0.5) * range_z) + bounds.min_z;
  *map_y = bounds.max_y - ((((double) vertex_z / exag_scale) + 0.5) * range_y);
}



//!  Turn display of a layer on or off.

void 
nvMapGL::displayLayer (int32_t layer, uint8_t display)
{
  data_display[layer] = display;
}



//!  Gets rid of all display lists.

void 
nvMapGL::resetMap ()
{
  for (uint32_t i = 0 ; i < list_count ; i++) glDeleteLists (display_list[i], 1);

  if (display_list != NULL) free (display_list);

  display_list = NULL;
  list_count = 0;
  text_object_count = 0;
}



//!  Turn depth testing on or off.  This is used if you want to use transparency in a point cloud.

void
nvMapGL::setDepthTest (uint8_t on)
{
  if (!on)
    {
      glDisable (GL_DEPTH_TEST);
    }
  else
    {
      glEnable (GL_DEPTH_TEST);
    }

  depth_test = on;
}



/*!  Enable all signals emitted by this class.  Signals are disabled by default.  You have
     to enable them to get the class to emit them.  This is handy if you are instantiating
     a map and you don't want it emit resize or redraw signals too soon.  */

void
nvMapGL::enableSignals ()
{
  signalsEnabled = NVTrue;
}



//!  Disable all signals emitted by this class.

void
nvMapGL::disableSignals ()
{
  signalsEnabled = NVFalse;
}



//!  Force a full res update

void
nvMapGL::force_full_res ()
{
  for (int32_t i = 0 ; i < MAX_ABE_PFMS ; i++)
    {
      if (data2_div[i])
        {
          update_type = 1;
          updateGL ();
          break;
        }
    }
}



//!  Update the OpenGL scene

void
nvMapGL::flush ()
{
  update_type = 1;
  updateGL ();
}



//  *************************************************************************************
//
//  Internal functions
//
//  *************************************************************************************

/*!  Wow.  A real scale.  Who'da thunk it.  This thing is a giant PITA to do.  Mostly because you have to determine the 
     orientation of the view (y_rotation and zx_rotation).  There's probably an easier way to do this other than the 
     five million 'if' statements but I really don't feel like figuring it out at the moment.  Just be glad that I
     documented which witch is which.  Don't forget, Z is Y and X is Z except when following 'C' or sounding like 'A' 
     and sometimes 'Y'...  Oh crap, I get so confused.  */

void 
nvMapGL::drawScale ()
{
  glDeleteLists (scale_list, 1);

  scale_list = glGenLists (1);

  if (scale_list == 0)
    {
      fprintf (stderr, "Unable to generate scale display list\n");
      return;
    }


  //  I realize that I don't need the brackets after the glNewList statement and before the glEndList statement 
  //  but it sets off the code and allows XEmacs to indent it nicely.  Same for glBegin and glEnd.  JCD

  glNewList (scale_list, GL_COMPILE);
  {
    glDisable (GL_LIGHTING);

    glPushAttrib (GL_ALL_ATTRIB_BITS);

    glLineWidth (1.0);

    glColor4ubv (scale_color);

    float min_x, min_y, min_z, max_x, max_y, max_z;

    map_to_screen (bounds.min_x, bounds.min_y, bounds.min_z, &min_x, &min_y, &min_z);
    map_to_screen (bounds.max_x, bounds.max_y, bounds.max_z, &max_x, &max_y, &max_z);


    double start_z = 0.0, end_z = 0.0, inc_z = 0.0;
    double range = -(bounds.min_z - bounds.max_z);

    if (range > 1.0)
      {
        inc_z = (double) ((int32_t) (range / 10.0) + 1);

        start_z = (-bounds.max_z) - fmod ((-bounds.max_z - inc_z), inc_z);
        if (start_z < (-bounds.max_z)) start_z += inc_z;
        end_z = (-bounds.min_z) - fmod ((-bounds.min_z), inc_z);
        if (end_z > (-bounds.min_z)) end_z -= inc_z;
        scale_tic_count = NINT ((end_z - start_z) / inc_z) + 1;
      }


    scale_tic[0].z = -bounds.min_z;
    scale_tic[scale_tic_count + 1].z = -bounds.max_z;


    glBegin (GL_LINES);


    //  Trying to determine what edges are in front or in back of the data based on zx_rotation and y_rotation.  We only want to show
    //  what's in back of the data.


    //  ZX Rotation -90.0 to 90.0 or 270.0 to 360.0/0.0 to 90.0 or -270.0 to -360.0/-0.0 to -90.0 (i.e. normal view, not flipped over)

    if ((zx_rotation > -90.0 && zx_rotation < 90.0) || (zx_rotation > 270.0 && zx_rotation < 360.0) ||
        (zx_rotation > 0.0 && zx_rotation < 90.0) || (zx_rotation < -270.0 && zx_rotation > -360.0) ||
        (zx_rotation < 0.0 && zx_rotation > -90.0))
      {
        //  Vertical line at bounds.max_y, bounds.min_x
        //  In front (don't draw) when y_rotation is 90.0 to 180.0 or -180.0 to -270.0

        if ((y_rotation < 90.0 || y_rotation > 180.0) && (y_rotation < -270.0 || y_rotation > -180.0))
          {
            glVertex3f (min_x, min_y, max_z);
            glVertex3f (min_x, max_y, max_z);
          }


        //  On right (draw number labels) when y_rotation is 180.0 to 270.0 or -90.0 to -180.0

        if ((y_rotation > 180.0 && y_rotation < 270.0) || (y_rotation < -90.0 && y_rotation > -180.0))
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.min_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.max_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.max_y;
                scale_tic[i + 1].x = bounds.min_x;
              }
          }


        //  Vertical line at bounds.max_y, bounds.max_x
        //  In front (don't draw) when y_rotation is 180.0 to 270.0 or -90.0 to -180.0

        if ((y_rotation < 180.0 || y_rotation > 270.0) && (y_rotation < -180.0 || y_rotation > -90.0))
          {
            glVertex3f (max_x, max_y, max_z);
            glVertex3f (max_x, min_y, max_z);
          }


        //  On right (draw number labels) when y_rotation is 0.0 to -90.0 or 270.0 to 360.0

        if ((y_rotation < 0.0 && y_rotation > -90.0) || y_rotation > 270.0)
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.max_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.max_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.max_y;
                scale_tic[i + 1].x = bounds.max_x;
              }
          }


        //  Vertical line at bounds.min_y, bounds.max_x
        //  In front (don't draw) when y_rotation is 0.0 to -90.0 or 270.0 to 360.0

        if ((y_rotation < -90.0 || y_rotation > 0.0) && y_rotation < 270.0)
          {
            glVertex3f (max_x, min_y, min_z);
            glVertex3f (max_x, max_y, min_z);
          }


        //  On right (draw number labels) when y_rotation is 0.0 to 90.0 or -270.0 to -360.0

        if ((y_rotation > 0.0 && y_rotation < 90.0) || y_rotation < -270.0)
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.max_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.min_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.min_y;
                scale_tic[i + 1].x = bounds.max_x;
              }
          }


        //  Vertical line at bounds.min_y, bounds.min_x
        //  In front (don't draw) when y_rotation is 0.0 to 90.0 or -270.0 to -360.0

        if ((y_rotation < 0.0 || y_rotation > 90.0) && y_rotation > -270.0)
          {
            glVertex3f (min_x, max_y, min_z);
            glVertex3f (min_x, min_y, min_z);
          }


        //  On right (draw number labels) when y_rotation is 90.0 to 180.0 or -180.0 to -270.0

        if ((y_rotation > 90.0 && y_rotation < 180.0) || (y_rotation < -180.0 && y_rotation > -270.0))
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.min_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.min_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.min_y;
                scale_tic[i + 1].x = bounds.min_x;
              }
          }


        //  Horizontal line from bounds.max_y, bounds.min_x to bounds.min_y, bounds.min_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 0.0 to 180.0 or -180.0 to -360.0 unless zx_rotation places it behind data.

        if (((y_rotation < 0.0 || y_rotation > 180.0) && y_rotation > -180.0) || (zx_rotation > 0.0 && zx_rotation < 90.0))
          {
            glVertex3f (min_x, min_y, max_z);
            glVertex3f (min_x, min_y, min_z);
          }

        if (((y_rotation < 0.0 || y_rotation > 180.0) && y_rotation > -180.0) || (zx_rotation < 0.0 && zx_rotation > -90.0))
          {
            glVertex3f (min_x, max_y, max_z);
            glVertex3f (min_x, max_y, min_z);
          }


        //  In back (draw scale lines) when y_rotation is 180.0 to 360.0 or 0.0 to -180.0

        if (y_rotation > 180.0 || (y_rotation < 0.0 && y_rotation > -180.0))
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.min_x, bounds.max_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.min_x, bounds.min_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }


        //  Horizontal line from bounds.max_y, bounds.max_x to bounds.min_y, bounds.max_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 0.0 to -180.0 or 180.0 to 360.0 unless zx_rotation places it behind data.

        if (((y_rotation < -180.0 || y_rotation > 0.0) && y_rotation < 180.0) || (zx_rotation > 0.0 && zx_rotation < 90.0))
          {
            glVertex3f (max_x, min_y, max_z);
            glVertex3f (max_x, min_y, min_z);
          }

        if (((y_rotation < -180.0 || y_rotation > 0.0) && y_rotation < 180.0) || (zx_rotation < 0.0 && zx_rotation > -90.0))
          {
            glVertex3f (max_x, max_y, max_z);
            glVertex3f (max_x, max_y, min_z);
          }


        //  In back (draw scale lines) when y_rotation is 0.0 to 180.0 or -180.0 to -360.0

        if ((y_rotation > 0.0 && y_rotation < 180.0) || y_rotation < -180.0)
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.max_x, bounds.max_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.max_x, bounds.min_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }



        //  Horizontal line from bounds.min_y, bounds.min_x to bounds.min_y, bounds.max_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 90.0 to -90.0 or -270.0 to -360.0 or 270.0 to 360.0 unless zx_rotation places it behind data.

        if (((y_rotation < -90.0 || y_rotation > 90.0) && y_rotation > -270.0 && y_rotation < 270.0) || (zx_rotation > 0.0 && zx_rotation < 90.0))
          {
            glVertex3f (min_x, min_y, min_z);
            glVertex3f (max_x, min_y, min_z);
          }

        if (((y_rotation < -90.0 || y_rotation > 90.0) && y_rotation > -270.0 && y_rotation < 270.0) || (zx_rotation < 0.0 && zx_rotation > -90.0))
          {
            glVertex3f (min_x, max_y, min_z);
            glVertex3f (max_x, max_y, min_z);
          }


        //  In back (draw scale lines) when y_rotation is 90.0 to 270.0 or -90.0 to -270.0

        if ((y_rotation > 90.0 && y_rotation < 270.0) || (y_rotation < -90.0 && y_rotation > -270.0))
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.min_x, bounds.min_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.max_x, bounds.min_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }


        //  Horizontal line from bounds.max_y, bounds.min_x to bounds.max_y, bounds.max_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 90.0 to 270.0 or -90.0 to -270.0 unless zx_rotation places it behind data.

        if (((y_rotation < 90.0 || y_rotation > 270.0) && (y_rotation < -270.0 || y_rotation > -90.0)) || (zx_rotation > 0.0 && zx_rotation < 90.0))
          {
            glVertex3f (min_x, min_y, max_z);
            glVertex3f (max_x, min_y, max_z);
          }

        if (((y_rotation < 90.0 || y_rotation > 270.0) && (y_rotation < -270.0 || y_rotation > -90.0)) || (zx_rotation < 0.0 && zx_rotation > -90.0))
          {
            glVertex3f (min_x, max_y, max_z);
            glVertex3f (max_x, max_y, max_z);
          }


        //  In back (draw scale lines) when y_rotation is -90.0 to 90.0 or 270 to 360.0/0.0 to 90.0 or -270.0 to -360.0

        if ((y_rotation > -90.0 && y_rotation < 90.0) || y_rotation > 270.0 || y_rotation < -270.0 || (y_rotation > 0.0 && y_rotation < 90.0))
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.min_x, bounds.max_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.max_x, bounds.max_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }
      }


    //  Flipped over.

    else
      {
        //  Vertical line at bounds.max_y, bounds.min_x
        //  In front (don't draw) when y_rotation is 0.0 to -90.0 or 270.0 to 360.0

        if ((y_rotation < -90.0 || y_rotation > 0.0) && y_rotation < 270.0)
          {
            glVertex3f (min_x, min_y, max_z);
            glVertex3f (min_x, max_y, max_z);
          }


        //  On right (draw number labels) when y_rotation is 180.0 to 270.0 or -90.0 to -180.0

        if ((y_rotation > 180.0 && y_rotation < 270.0) || (y_rotation < -90.0 && y_rotation > -180.0))
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.min_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.max_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.max_y;
                scale_tic[i + 1].x = bounds.min_x;
              }
          }


        //  Vertical line at bounds.max_y, bounds.max_x
        //  In front (don't draw) when y_rotation is 0.0 to 90.0 or -270.0 to -360.0

        if ((y_rotation < 0.0 || y_rotation > 90.0) && y_rotation > -270.0)
          {
            glVertex3f (max_x, max_y, max_z);
            glVertex3f (max_x, min_y, max_z);
          }


        //  On right (draw number labels) when y_rotation is 0.0 to -90.0 or 270.0 to 360.0

        if ((y_rotation < 0.0 && y_rotation > -90.0) || y_rotation > 270.0)
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.max_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.max_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.max_y;
                scale_tic[i + 1].x = bounds.max_x;
              }
          }


        //  Vertical line at bounds.min_y, bounds.max_x
        //  In front (don't draw) when y_rotation is 90.0 to 180.0 or -180.0 to -270.0

        if ((y_rotation < 90.0 || y_rotation > 180.0) && (y_rotation < -270.0 || y_rotation > -180.0))
          {
            //  Vertical line

            glVertex3f (max_x, min_y, min_z);
            glVertex3f (max_x, max_y, min_z);
          }


        //  On right (draw number labels) when y_rotation is 0.0 to 90.0 or -270.0 to -360.0

        if ((y_rotation > 0.0 && y_rotation < 90.0) || y_rotation < -270.0)
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.max_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.min_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.min_y;
                scale_tic[i + 1].x = bounds.max_x;
              }
          }


        //  Vertical line at bounds.min_y, bounds.min_x
        //  In front (don't draw) when y_rotation is 180.0 to 270.0 or -90.0 to -180.0

        if ((y_rotation < 180.0 || y_rotation > 270.0) && (y_rotation < -180.0 || y_rotation > -90.0))
          {
            //  Vertical line

            glVertex3f (min_x, max_y, min_z);
            glVertex3f (min_x, min_y, min_z);
          }


        //  On right (draw number labels) when y_rotation is 90.0 to 180.0 or -180.0 to -270.0

        if ((y_rotation > 90.0 && y_rotation < 180.0) || (y_rotation < -180.0 && y_rotation > -270.0))
          {
            scale_tic[0].x = scale_tic[scale_tic_count + 1].x = bounds.min_x;
            scale_tic[0].y = scale_tic[scale_tic_count + 1].y = bounds.min_y;
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                scale_tic[i + 1].z = start_z + (double) i * inc_z;
                scale_tic[i + 1].y = bounds.min_y;
                scale_tic[i + 1].x = bounds.min_x;
              }
          }


        //  Horizontal line from bounds.max_y, bounds.min_x to bounds.min_y, bounds.min_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 0.0 to -180.0 or 180.0 to 360.0 unless zx_rotation places it behind data.

        if (((y_rotation < -180.0 || y_rotation > 0.0) && y_rotation < 180.0) ||
            (zx_rotation < -180.0 && zx_rotation > -360.0) || (zx_rotation > 90.0 && zx_rotation < 180.0))
          {
            glVertex3f (min_x, min_y, max_z);
            glVertex3f (min_x, min_y, min_z);
          }

        if (((y_rotation < -180.0 || y_rotation > 0.0) && y_rotation < 180.0) || 
            (zx_rotation < 0.0 && zx_rotation > -180.0) || (zx_rotation > 180.0 && zx_rotation < 270.0))
          {
            glVertex3f (min_x, max_y, max_z);
            glVertex3f (min_x, max_y, min_z);
          }


        //  In back (draw scale lines) when y_rotation is 0.0 to 180.0 or -180.0 to -360.0

        if ((y_rotation > 0.0 && y_rotation < 180.0) || y_rotation < -180.0)
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.min_x, bounds.max_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.min_x, bounds.min_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }


        //  Horizontal line from bounds.max_y, bounds.max_x to bounds.min_y, bounds.max_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 0.0 to 180.0 or -180.0 to -360.0 unless zx_rotation places it behind data.

        if (((y_rotation < 0.0 || y_rotation > 180.0) && y_rotation > -180.0) ||
            (zx_rotation < -180.0 && zx_rotation > -360.0) || (zx_rotation > 90.0 && zx_rotation < 180.0))
          {
            glVertex3f (max_x, min_y, max_z);
            glVertex3f (max_x, min_y, min_z);
          }

        if (((y_rotation < 0.0 || y_rotation > 180.0) && y_rotation > -180.0) ||
            (zx_rotation < 0.0 && zx_rotation > -180.0) || (zx_rotation > 180.0 && zx_rotation < 270.0))
          {
            glVertex3f (max_x, max_y, max_z);
            glVertex3f (max_x, max_y, min_z);
          }


        //  In back (draw scale lines) when y_rotation is 180.0 to 360.0 or 0.0 to -180.0

        if (y_rotation > 180.0 || (y_rotation < 0.0 && y_rotation > -180.0))
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.max_x, bounds.max_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.max_x, bounds.min_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }


        //  Horizontal line from bounds.min_y, bounds.min_x to bounds.min_y, bounds.max_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 90.0 to 270.0 or -90.0 to -270.0 unless zx_rotation places it behind data.

        if (((y_rotation < 90.0 || y_rotation > 270.0) && (y_rotation < -270.0 || y_rotation > -90.0)) ||
            (zx_rotation < -180.0 && zx_rotation > -360.0) || (zx_rotation > 90.0 && zx_rotation < 180.0))
          {
            glVertex3f (min_x, min_y, min_z);
            glVertex3f (max_x, min_y, min_z);
          }

        if (((y_rotation < 90.0 || y_rotation > 270.0) && (y_rotation < -270.0 || y_rotation > -90.0)) ||
            (zx_rotation < 0.0 && zx_rotation > -180.0) || (zx_rotation > 180.0 && zx_rotation < 270.0))
          {
            glVertex3f (min_x, max_y, min_z);
            glVertex3f (max_x, max_y, min_z);
          }


        //  In back (draw scale lines) when y_rotation is -90.0 to 90.0 or 270 to 360.0/0.0 to 90.0 or -270.0 to -360.0

        if ((y_rotation > -90.0 && y_rotation < 90.0) || y_rotation > 270.0 || y_rotation < -270.0 || (y_rotation > 0.0 && y_rotation < 90.0))
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.min_x, bounds.min_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.max_x, bounds.min_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }


        //  Horizontal line from bounds.max_y, bounds.min_x to bounds.max_y, bounds.max_x (at bounds.min_z and bounds.max_z)
        //  In front (don't draw) when y_rotation is 90.0 to -90.0 or -270.0 to -360.0 or 270.0 to 360.0 unless zx_rotation places it behind data.

        if (((y_rotation < -90.0 || y_rotation > 90.0) && y_rotation > -270.0 && y_rotation < 270.0) ||
            (zx_rotation < -180.0 && zx_rotation > -360.0) || (zx_rotation > 90.0 && zx_rotation < 180.0))
          {
            glVertex3f (min_x, min_y, max_z);
            glVertex3f (max_x, min_y, max_z);
          }

        if (((y_rotation < -90.0 || y_rotation > 90.0) && y_rotation > -270.0 && y_rotation < 270.0) ||
            (zx_rotation < 0.0 && zx_rotation > -180.0) || (zx_rotation > 180.0 && zx_rotation < 270.0))
          {
            glVertex3f (min_x, max_y, max_z);
            glVertex3f (max_x, max_y, max_z);
          }


        //  In back (draw scale lines) when y_rotation is 90.0 to 270.0 or -90.0 to -270.0

        if ((y_rotation > 90.0 && y_rotation < 270.0) || (y_rotation < -90.0 && y_rotation > -270.0))
          {
            for (int32_t i = 0 ; i < scale_tic_count ; i++)
              {
                float x[2], y[2], z[2];
                map_to_screen (bounds.min_x, bounds.max_y, -scale_tic[i + 1].z, &x[0], &y[0], &z[0]);
                map_to_screen (bounds.max_x, bounds.max_y, -scale_tic[i + 1].z, &x[1], &y[1], &z[1]);

                glVertex3f (x[0], y[0], z[0]);
                glVertex3f (x[1], y[1], z[1]);
              }
          }
      }


    glEnd ();

    glPopAttrib ();

    glEnable (GL_LIGHTING);
  }
  glEndList ();
}



//!  Make sure a map point (map_x,map_y) is in the currently displayed map area.

void 
nvMapGL::boundsCheck (double *map_x, double *map_y)
{
  if (*map_x < bounds.min_x) *map_x = bounds.min_x;
  if (*map_x > bounds.max_x) *map_x = bounds.max_x;
  if (*map_y < bounds.min_y) *map_y = bounds.min_y;
  if (*map_y > bounds.max_y) *map_y = bounds.max_y;
}



//!  Initialize OpenGL.

void 
nvMapGL::initializeGL ()
{
  float r, g, b, a;

  r = (float) map.background_color.red () / 255.0;
  g = (float) map.background_color.green () / 255.0;
  b = (float) map.background_color.blue () / 255.0;
  a = (float) (255 - map.background_color.alpha ()) / 255.0;

  glClearColor (r, g, b, a);

  glShadeModel (GL_SMOOTH);


  /*  We will set the ambience for the light in the scene within the setRenderMode Ambience method.
      the reason we have 2 different ambiences is because to achieve shading we can't have a full
      intensity ambient light.  For points, the full intensity is needed for the best effect because
      they really don't have surfaces.  For more complex shapes a small degree of ambience is needed
      so even parts of the object that are not hit with the light will appear.  */

  if (map.light_model != GL_LIGHT_MODEL_AMBIENT) glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);


  /*  The glColorMaterial call is an easy way to control the diffuse and ambient reflections of the shapes.  
      this allows us to use the glColor commands to specify the reflections.  renderMode specifies which route
      we are taking as far as what to visualize each shot with (POINT, SPHERE, etc.).  Based on whether it is a
      point or a complex shape, we will set the ambient light properties accordingly through the setRenderModeAmbience
      method.  */

  //  Unless we're in NVMAPGL_BIN_MODE in which case we're drawing a 3D surface, not discrete points.  JCD

  if (map.mode != NVMAPGL_BIN_MODE)
    {
      glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
      setRenderModeAmbience (map.renderMode);
    }


  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_COLOR_MATERIAL);

  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  glEnable (GL_ALPHA_TEST);
  glAlphaFunc (GL_NOTEQUAL, 0.0);


  //  Enable hidden surface removal

  glEnable (GL_DEPTH_TEST);


  //  Enable arrays

  glEnableClientState (GL_VERTEX_ARRAY);
  glEnableClientState (GL_COLOR_ARRAY);


  //  Enable smoothing of points

  //glEnable (GL_POINT_SMOOTH);


  glReady = NVTrue;


  //  Build the "scale" display list just in case we want to plot it.

  drawScale ();
}



//!  The resize callback.

void 
nvMapGL::resizeGL (int w, int h)
{
  static int32_t start_count = 0;


  //  Set the size and location of the window.

  glViewport (0, 0, (GLint) w, (GLint) h);
  if (!h) return;


  //  Save the size for other stuff.

  map.draw_width = w;
  map.draw_height = h;


  //  Set matrix mode to projection, clear the matrix, and set the projection transformation.

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();

  aspect_ratio = (float) w / (float) h;
  near_plane = 1.0;
  far_plane = 4.0;
  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


  //  Set the matrix mode to modelview and clear the matrix.

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity ();


  //  Move the camera away from the object along the Z axis.

  gluLookAt (eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);


  //  Restore the translation

  glTranslatef (centerX, centerY, centerZ);


  //  Restore the rotation

  glRotatef (y_rotation, 0.0, 1.0, 0.0);
  glRotatef (zx_rotation, cos (y_rotation * NV_DEG_TO_RAD), 0.0, sin (y_rotation * NV_DEG_TO_RAD));


  //  Set the light location.


  //  We want to make sure we are starting from the origin and not moving from prior transformations

  glPushMatrix ();
  glLoadIdentity ();


  //GLfloat light_position[4] = {1.0, 0.33, 1.0, 0.0}; 
  GLfloat light_position[4] = {3.0, 0.33, 1.0, 0.0}; 

  glLightfv (GL_LIGHT0, GL_POSITION, light_position);


  //  Restore the previous matrix 

  glPopMatrix ();


  //  Clear the screen (but only on the second resize - there is some sort of timing issue here).

  if (start_count == 1)
    {
      update_type = 999;
      updateGL ();
      start_count++;
    }
  start_count = 1;


  emit resizeSignal (NULL);
}



/*!  Windows issues a paintGL call whenever focus is returned to this widget which causes all sorts of problems.
     Otherwise, this is just an expose event.  */

void
nvMapGL::paintEvent (QPaintEvent *e __attribute__ ((unused)))
{
  if (e->type () == QEvent::Paint && windoze_paint_event_enabled)
    {
      update_type = 2;
      updateGL ();
    }
  windoze_paint_event_enabled = NVTrue;
}



//!  This is where the actual painting gets done.

void 
nvMapGL::paintGL ()
{
  //  Called from resize () to clear the screen on initialization.

  if (update_type == 999)
    {
      glDrawBuffer (GL_FRONT_AND_BACK);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glFlush ();
      update_type = 0;


      //  Check for errors (OpenGL is really picky).

      //if (int32_t err = glGetError ()) fprintf (stderr, "Some sort of GL error occurred : %d\n", err);
      return;
    }


  uint8_t double_flag = NVFalse;
  uint8_t cleared = NVFalse;


  //  Must use front buffer on Windoze except for the rotate and zoom stuff.

#ifdef NVWIN3X
  glDrawBuffer (GL_FRONT);
#else
  glDrawBuffer (GL_BACK);
#endif


  //  If we are updating a movable object we don't want to redraw everything or clear the screen.

  if (update_type > 0)
    {
      //  Rotate and/or zoom calls (update_type > 256) are double buffered and we don't draw the XOR stuff.

      if (update_type & 0x100)
        {
          double_flag = NVTrue;
#ifdef NVWIN3X
          glDrawBuffer (GL_BACK);
#endif
        }


      //  Clear the screen (disabling dithering for clearing is a speed thing - you shouldn't have to
      //  dither a single color).

      glDisable (GL_DITHER);
      glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable (GL_DITHER);


      //  Draw the "scale" if requested.

      if (map.draw_scale)
        {
          glCallList (scale_list);

          glPushMatrix ();

          float x, y, z;
          QString label;
          QFont qfont ("Charter", 10);


          //  If we're offsetting and/or multiplying the scale numbers set the "offset" color.

          if (map.scale_offset != 0.0 || map.scale_factor != 1.0)
            {
              glColor4ubv (scale_offset_color);
            }
          else
            {
              glColor4ubv (scale_color);
            }

          glRotatef (0.0, 0.0, 1.0, 0.0);

          map_to_screen (scale_tic[0].x, scale_tic[0].y, -scale_tic[0].z, &x, &y, &z);
          label.sprintf (" %.2f", z_orientation * scale_tic[0].z * map.scale_factor + map.scale_offset);
          renderText (x, y, z, label, qfont);

          map_to_screen (scale_tic[scale_tic_count + 1].x, scale_tic[scale_tic_count + 1].y, -scale_tic[scale_tic_count + 1].z, &x, &y, &z);
          label.sprintf (" %.2f", z_orientation * scale_tic[scale_tic_count + 1].z * map.scale_factor + map.scale_offset);
          renderText (x, y, z, label, qfont);

          for (int32_t i = 0 ; i < scale_tic_count ; i++)
            {
              map_to_screen (scale_tic[i + 1].x, scale_tic[i + 1].y, -scale_tic[i + 1].z, &x, &y, &z);
              label.sprintf (" %.2f", z_orientation * scale_tic[i + 1].z * map.scale_factor + map.scale_offset);
              renderText (x, y, z, label, qfont);
            }

          glPopMatrix ();
        }


      //  Display the data layers (with or without GeoTIFF texture)

      for (int32_t pfm = 0 ; pfm < MAX_ABE_PFMS ; pfm++)
        {
          //  If we have two surface resolutions and we're rotating, draw the subsampled one (data2).

          if (data2_div[pfm] && double_flag)
            {
              //  Subsampled resolution

              if (data2_list[pfm]) glCallList (data2_list[pfm]);
            }
          else
            {
              //  Normal resolution

              if (data_list[pfm]) glCallList (data_list[pfm]);
            }
        }


      //  Display the sparse and/or full data points

      if (map.mode == NVMAPGL_SPARSE_MODE)
        {
          if (sparse_count) glCallList (sparse_list);
        }
      else
        {
          if (point_count) glCallList (point_list);
        }


      //  Display the feature points (no display list so that the features scale properly when zooming).

      if (feature_count)
        {
          for (int32_t i = 0 ; i < feature_count ; i++)
            {
              glPushMatrix();

              glTranslatef (feature_data[i].v[0], feature_data[i].v[1], feature_data[i].v[2]);

              GLUquadricObj *quadric = gluNewQuadric ();

              gluQuadricNormals (quadric, GLU_SMOOTH);

              gluQuadricDrawStyle (quadric, GLU_FILL);

              glColor4ubv (feature_data[i].color);

              gluSphere (quadric, feature_data[i].size * exag_scale * map.zoom_factor, feature_data[i].slices, feature_data[i].stacks);

              gluDeleteQuadric (quadric);

              glPopMatrix();
            }
        }


      //  Display the lines (no display list so that the lines scale properly when zooming).

      if (line_count)
        {
          for (int32_t i = 0 ; i < line_count ; i++)
            {
              glColor4ubv (line_data[i].color);


              //  Set the line width

              glLineWidth ((GLfloat) line_data[i].width);


              //  Set the stipple pattern

              glLineStipple (2, line_data[i].stipple);


              glPushMatrix();


              glBegin (GL_LINES);
              {
                for (int32_t j = 1 ; j < line_data[i].count ; j++)
                  {
                    glVertex3f (line_data[i].v[j - 1].x, line_data[i].v[j - 1].y, line_data[i].v[j - 1].z);
                    glVertex3f (line_data[i].v[j].x, line_data[i].v[j].y, line_data[i].v[j].z);
                  }
              }
              glEnd ();

              glPopMatrix();
            }
        }


      //  Display the marker points (no display list so that the markers scale properly when zooming).

      if (marker_count)
        {
          for (int32_t i = 0 ; i < marker_count ; i++)
            {
              glColor4ubv (marker_data[i].color);


              //  Set the line width

              glLineWidth (2.0);//(GLfloat) line[i].width);


              //  Set the stipple pattern

              glLineStipple (2, Qt::SolidLine);//line[i].stipple);


              glPushMatrix();


              glTranslatef (marker_data[i].v[0], marker_data[i].v[1], marker_data[i].v[2]);


              glBegin (GL_LINES);
              {
                float sz = marker_data[i].size * exag_scale * map.zoom_factor;
                glVertex3f (-sz, 0.0, 0.0);
                glVertex3f (sz, 0.0, 0.0);
                glVertex3f (0.0, 0.0, 0.0);
                glVertex3f (0.0, sz, 0.0);
                glVertex3f (0.0, -sz, 0.0);
                glVertex3f (0.0, 0.0, 0.0);
                glVertex3f (0.0, 0.0, sz);
                glVertex3f (0.0, 0.0, -sz);
              }
              glEnd ();

              glPopMatrix();
            }
        }


      //  Display any overlays

      for (uint32_t list = 0 ; list < list_count ; list++) if (display_list[list]) glCallList (display_list[list]);


      //  Display text strings.  I'm doing this here because QGLWidgets renderText method doesn't rotate with the
      //  context as far as I can tell.  I tried putting it into a display list but it didn't work.

      for (uint32_t text = 0 ; text < text_object_count ; text++)
        {
          glPushMatrix ();


          glColor4fv (text_object[text].color);
          glRotatef (text_object[text].angle, 0.0, 1.0, 0.0);

          renderText (text_object[text].x, text_object[text].y, text_object[text].z, text_object[text].string,
                      text_object[text].font);


          glPopMatrix ();
        }


      //  Since we cleared all of the screen we want to re-initiate any existing movable objects without
      //  drawing the XORed, previous object.

      cleared = NVTrue;


      //  If we were rotating/zooming (i.e. double buffering) swap buffers and return.

      if (double_flag)
        {
          swapBuffers ();


          //  Check for errors (OpenGL is really picky).

          //if (int32_t err = glGetError ()) fprintf (stderr, "Some sort of GL error occurred : %d\n", err);

          return;
        }
    }


  //  Display movable and rubberband objects

  int32_t bottom = 0, top = 0, y = 0;

  if (movable_object_count)
    {
      //  Set up XOR graphics

      glEnable (GL_COLOR_LOGIC_OP);
      glLogicOp (GL_XOR);


      for (int32_t k = 0 ; k < movable_object_count ; k++)
        {
          switch (mot[k]->type)
            {
            case NVMAPGL_MOVE_LIST:


              //  Turn off the lights so the markers do not get affected with the shading
			  
              glDisable (GL_LIGHTING);


              //  Set the color

              glColor4ubv (mot[k]->color);


              //  Save the current line width, stipple pattern, etc

              glPushAttrib (GL_LINE_BIT);


              //  Set the line width

              glLineWidth ((GLfloat) mot[k]->line_width);


              //  Turn off depth buffering for the moment

              if (depth_test) glDisable (GL_DEPTH_TEST);


              //  If we had the object before we need to erase it.

              if (!cleared && mot[k]->prev_polygon_count)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Rotate the object

                  glRotatef (mot[k]->prev_rotation_angle, 1.0, 0.0, 0.0);


                  //  Draw the marker

                  glBegin (GL_LINES);
                  {
                    for (int32_t i = 0 ; i < mot[k]->prev_polygon_count ; i++)
                      {
                        y = map.draw_height - mot[k]->prev_polygon_y[i];
                        glVertex2i (mot[k]->prev_polygon_x[i], y);
                      }
                  }
                  glEnd ();


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();
                }


              if (update_type >= 0)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Rotate the object

                  glRotatef (mot[k]->rotation_angle, 1.0, 0.0, 0.0);


                  //  Draw the marker

                  glBegin (GL_LINES);
                  {
                    for (int32_t i = 0 ; i < mot[k]->polygon_count ; i++)
                      {
                        y = map.draw_height - mot[k]->polygon_y[i];
                        glVertex2i (mot[k]->polygon_x[i], y);
                      }
                  }
                  glEnd ();


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();


                  //  Save the previous object information

                  mot[k]->prev_rotation_angle = mot[k]->rotation_angle;
                  mot[k]->prev_polygon_count = mot[k]->polygon_count;

                  for (int32_t i = 0 ; i < mot[k]->polygon_count ; i++)
                    {
                      mot[k]->prev_polygon_x[i] = mot[k]->polygon_x[i];
                      mot[k]->prev_polygon_y[i] = mot[k]->polygon_y[i];
                    }
                }


              //  Turn depth buffering back on

              if (depth_test) glEnable (GL_DEPTH_TEST);


              //  Done with the markers, lights back on.  side note:  for all of the other objects, I figured the color was not correlated
              //  to anything so they will be affected by the shading.
			  
              glEnable (GL_LIGHTING);


              //  Restore the line width

              glPopAttrib ();

              break;



            case NVMAPGL_RUBBER_RECTANGLE:


              //  Set the color

              glColor4ubv (mot[k]->color);


              //  Save the current line width, stipple pattern, etc

              glPushAttrib (GL_LINE_BIT);


              //  Set the line width

              glLineWidth ((GLfloat) mot[k]->line_width);


              //  Set the stipple pattern

              glLineStipple (2, mot[k]->stipple);


              //  Turn off depth buffering for the moment

              if (depth_test) glDisable (GL_DEPTH_TEST);


              //  If we need to erase a previous rectangle...

              if (!cleared && mot[k]->prev_rect.left () >= 0)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Draw the rectangle.  We have to flip the Y coordinates because of the way OpenGL handles Y.

                  bottom = map.draw_height - mot[k]->prev_rect.bottom ();
                  top = map.draw_height - mot[k]->prev_rect.top ();


                  glBegin (GL_LINE_STRIP);
                  {
                    glVertex2i (mot[k]->prev_rect.left (), bottom);
                    glVertex2i (mot[k]->prev_rect.right (), bottom);
                    glVertex2i (mot[k]->prev_rect.right (), top);
                    glVertex2i (mot[k]->prev_rect.left (), top);
                    glVertex2i (mot[k]->prev_rect.left (), bottom);
                  }
                  glEnd ();


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();

                }


              //  Now draw the new rectangle.

              if (update_type>= 0)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Draw the rectangle.  We have to flip the Y coordinates because of the way OpenGL handles Y.

                  bottom = map.draw_height - mot[k]->rect.bottom ();
                  top = map.draw_height - mot[k]->rect.top ();


                  glBegin (GL_LINE_STRIP);
                  {
                    glVertex2i (mot[k]->rect.left (), bottom);
                    glVertex2i (mot[k]->rect.right (), bottom);
                    glVertex2i (mot[k]->rect.right (), top);
                    glVertex2i (mot[k]->rect.left (), top);
                    glVertex2i (mot[k]->rect.left (), bottom);
                  }
                  glEnd ();


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();


                  //  Save the previous object information

                  mot[k]->prev_rect = mot[k]->rect;
                }


              //  Turn depth buffering back on

              if (depth_test) glEnable (GL_DEPTH_TEST);


              //  Restore the line width

              glPopAttrib ();

              break;



            case NVMAPGL_RUBBER_POLYGON:


              //  Set the color

              glColor4ubv (mot[k]->color);


              //  Save the current line width, stipple pattern, etc

              glPushAttrib (GL_LINE_BIT);


              //  Set the line width

              glLineWidth ((GLfloat) mot[k]->line_width);


              //  Set the stipple pattern

              glLineStipple (2, mot[k]->stipple);


              //  Turn off depth buffering for the moment

              if (depth_test) glDisable (GL_DEPTH_TEST);


              //  If we need to erase a previous polygon...

              if (!cleared && mot[k]->prev_polygon_count)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Draw the polygon.  We have to flip the Y coordinates because of the way OpenGL handles Y.

                  if (mot[k]->prev_polygon_count > 2)
                    {
                      glBegin (GL_LINE_STRIP);
                      {
                        for (int32_t i = 0 ; i < mot[k]->prev_polygon_count ; i++)
                          {
                            y = map.draw_height - mot[k]->prev_polygon_y[i];

                            glVertex2i (mot[k]->prev_polygon_x[i], y);
                          }

                        if (mot[k]->close_polygon)
                          {
                            y = map.draw_height - mot[k]->prev_polygon_y[0];

                            glVertex2i (mot[k]->prev_polygon_x[0], y);
                          }
                      }
                      glEnd ();
                    }
                  else
                    {
                      if (mot[k]->prev_polygon_count > 1)
                        {
                          glBegin (GL_LINES);
                          {
                            y = map.draw_height - mot[k]->prev_polygon_y[0];
                            glVertex2i (mot[k]->prev_polygon_x[0], y);
                            y = map.draw_height - mot[k]->prev_polygon_y[1];
                            glVertex2i (mot[k]->prev_polygon_x[1], y);
                          }
                          glEnd ();
                        }
                    }


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();

                }


              //  Now draw the new polygon.

              if (update_type >= 0)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Draw the polygon.  We have to flip the Y coordinates because of the way OpenGL handles Y.

                  if (mot[k]->polygon_count > 2)
                    {
                      glBegin (GL_LINE_STRIP);
                      {
                        for (int32_t i = 0 ; i < mot[k]->polygon_count ; i++)
                          {
                            y = map.draw_height - mot[k]->polygon_y[i];

                            glVertex2i (mot[k]->polygon_x[i], y);
                          }

                        if (mot[k]->close_polygon)
                          {
                            y = map.draw_height - mot[k]->polygon_y[0];

                            glVertex2i (mot[k]->polygon_x[0], y);
                          }
                      }
                      glEnd ();
                    }
                  else
                    {
                      if (mot[k]->polygon_count > 1)
                        {
                          glBegin (GL_LINES);
                          {
                            y = map.draw_height - mot[k]->polygon_y[0];
                            glVertex2i (mot[k]->polygon_x[0], y);
                            y = map.draw_height - mot[k]->polygon_y[1];
                            glVertex2i (mot[k]->polygon_x[1], y);
                          }
                          glEnd ();
                        }
                    }


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();


                  //  Save the previous object information

                  mot[k]->prev_polygon_count = mot[k]->polygon_count;

                  for (int32_t i = 0 ; i < mot[k]->polygon_count ; i++)
                    {
                      mot[k]->prev_polygon_x[i] = mot[k]->polygon_x[i];
                      mot[k]->prev_polygon_y[i] = mot[k]->polygon_y[i];
                    }
                }


              //  Turn depth buffering back on

              if (depth_test) glEnable (GL_DEPTH_TEST);


              //  Restore the line width

              glPopAttrib ();

              break;


            case NVMAPGL_RUBBER_LINE:


              //  Set the color

              glColor4ubv (mot[k]->color);


              //  Save the current line width, stipple pattern, etc

              glPushAttrib (GL_LINE_BIT);


              //  Set the line width

              glLineWidth ((GLfloat) mot[k]->line_width);


              //  Set the stipple pattern

              glLineStipple (2, mot[k]->stipple);


              //  Turn off depth buffering for the moment

              if (depth_test) glDisable (GL_DEPTH_TEST);


              //  If we need to erase a previous line...

              if (!cleared && mot[k]->prev_x0 >= 0)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Draw the line.  We have to flip the Y coordinates because of the way OpenGL handles Y.

                  glBegin (GL_LINES);
                  {
                    y = map.draw_height - mot[k]->prev_y0;
                    glVertex2i (mot[k]->prev_x0, y);
                    y = map.draw_height - mot[k]->prev_y1;
                    glVertex2i (mot[k]->prev_x1, y);
                  }
                  glEnd ();


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();

                }


              //  Now draw the new line.

              if (update_type >= 0)
                {
                  //  Save the original matrix

                  glPushMatrix ();


                  //  Switch to 2D Orthographic projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluOrtho2D (0, map.draw_width, 0, map.draw_height);


                  //  Go to model view and load the identity matrix

                  glMatrixMode (GL_MODELVIEW);
                  glLoadIdentity ();


                  //  Draw the line.  We have to flip the Y coordinates because of the way OpenGL handles Y.

                  glBegin (GL_LINES);
                  {
                    y = map.draw_height - mot[k]->y0;
                    glVertex2i (mot[k]->x0, y);
                    y = map.draw_height - mot[k]->y1;
                    glVertex2i (mot[k]->x1, y);
                  }
                  glEnd ();


                  //  Go back to perspective projection

                  glMatrixMode (GL_PROJECTION);
                  glLoadIdentity ();
                  gluPerspective (fovy * map.zoom_factor * exag_scale, aspect_ratio, near_plane, far_plane);


                  //  Set the matrix mode to modelview.

                  glMatrixMode (GL_MODELVIEW);


                  //  Restore the original matrix

                  glPopMatrix ();


                  //  Save the previous object information

                  mot[k]->prev_x0 = mot[k]->x0;
                  mot[k]->prev_y0 = mot[k]->y0;
                  mot[k]->prev_x1 = mot[k]->x1;
                  mot[k]->prev_y1 = mot[k]->y1;
                }


              //  Turn depth buffering back on

              if (depth_test) glEnable (GL_DEPTH_TEST);


              //  Restore the line width

              glPopAttrib ();

              break;
            }
        }

      glDisable (GL_COLOR_LOGIC_OP);
    }


  //  Since we're not double buffering on Windoze, if we got to this point, we need to flush the front buffer.

#ifdef NVWIN3X
      glFlush ();
#else

  //  Manually swap the (just drawn) back buffer to the front.

  swapBuffers ();

#endif


  //  Since we want complete control or redrawing let's make sure that the update_type doesn't roll through
  //  from some strange redraw command that we don't know about (Windoze, bah humbug!).

  update_type = 0;


  //  Check for errors (OpenGL is really picky).

  //if (int32_t err = glGetError ()) fprintf (stderr, "Some sort of GL error occurred : %d\n", err);
}



//!  Entered the map area.  Emit an "enter" signal.

void
nvMapGL::enterEvent (QEvent *e)
{
  if (signalsEnabled)
    {
      //  Let the parent do something.

      emit enterSignal (e);


      //  Enter and leave events shouldn't generate paint events but they do on Windoze.

#ifdef NVWIN3X
      windoze_paint_event_enabled = NVFalse;
#endif
    }
}



//!  Left the map area.  Emit a "leave" signal.

void
nvMapGL::leaveEvent (QEvent *e)
{
  if (signalsEnabled)
    {
      //  Let the parent do something.

      emit leaveSignal (e);


      //  Enter and leave events shouldn't generate paint events but they do on Windoze.

#ifdef NVWIN3X
      windoze_paint_event_enabled = NVFalse;
#endif
    }
}



/*!  Double clicked a mouse button.  Convert screen coordinates to map coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMapGL::mouseDoubleClickEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      int32_t x = e->x (), y = e->y ();

      double map_x, map_y, map_z;

      get3DCoords (x, y, &map_x, &map_y, &map_z);

      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mouseDoubleClickSignal (e, map_x, map_y, map_z);
    }
}



/*!  Pressed a mouse button.  Convert screen coordinates to map coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMapGL::mousePressEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      int32_t x = e->x (), y = e->y ();

      double map_x, map_y, map_z;

      get3DCoords (x, y, &map_x, &map_y, &map_z);

      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mousePressSignal (e, map_x, map_y, map_z);
    }
  else
    {
      emit preliminaryMousePressSignal (e);
    }
}



/*!  Released a mouse button.  Convert screen coordinates to map coordinates and
     emit a signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMapGL::mouseReleaseEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      int32_t x = e->x (), y = e->y ();

      double map_x, map_y, map_z;

      get3DCoords (x, y, &map_x, &map_y, &map_z);

      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mouseReleaseSignal (e, map_x, map_y, map_z);
    }
}



/*!  Moved mouse.  Convert screen coordinates to map coordinates and emit a 
     signal.  The slot can get the pixel x,y from e->x () and e->y ().  */

void
nvMapGL::mouseMoveEvent (QMouseEvent *e)
{
  if (signalsEnabled)
    {
      int32_t x = e->x (), y = e->y ();

      double map_x, map_y, map_z;

      get3DCoords (x, y, &map_x, &map_y, &map_z);

      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit mouseMoveSignal (e, map_x, map_y, map_z, map);
    }
}



//!  Popped a wheelie!  Emit a wheel signal.

void
nvMapGL::wheelEvent (QWheelEvent *e)
{
  if (signalsEnabled)
    {
      int32_t x = e->x (), y = e->y ();

      get3DCoords (x, y, &cursor_pos);


      double map_x, map_y, map_z;


      screen_to_map (cursor_pos.x, cursor_pos.y, cursor_pos.z, &map_x, &map_y, &map_z);

      boundsCheck (&map_x, &map_y);


      //  Let the parent do something.

      emit wheelSignal (e, cursor_pos.x, cursor_pos.y, cursor_pos.z);
    }
}



//!  Someone may try to close the window using the little X button (I hate those things).

void 
nvMapGL::closeEvent (QCloseEvent *e)
{
  emit closeSignal (e);
}



/*!  This focus event is here because the paintevent would sometimes go batshit if there was a movable
     object and you lost or gained focus.  I have no idea why.  It will emit a signal if you need it for anything.  */

void 
nvMapGL::focusInEvent (QFocusEvent *e)
{
  emit focusInSignal (e);
}



/*!  This focus event is here because the paintevent would sometimes go batshit if there was a movable
     object and you lost or gained focus.  I have no idea why.  It will emit a signal if you need it for anything.  */

void 
nvMapGL::focusOutEvent (QFocusEvent *e)
{
  emit focusOutSignal (e);
}



//!  Get the nearest layer vertex to the ray defined by nwx, nwy, nwz (near) and fwx, fwy, and fwz (far).

uint8_t 
nvMapGL::getNearestLayerVertex (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz,
                                GLuint *data_list, GLfloat ****data_vertices, int32_t *data_rows,
                                int32_t *data_cols, int32_t *pfm, int32_t *row, int32_t *col)
{
  uint8_t ret = NVFalse;


  Vector3 vClickRayP1 = Vector3 (nwx, nwy, nwz);
  Vector3 vClickRayP2 = Vector3 (fwx, fwy, fwz);

  Vector3 vClickSlope;
  VMV3 (vClickSlope, vClickRayP2, vClickRayP1);
  vClickSlope.Normalize ();


  double MinDistance = 99999999999.0;

  for (int32_t p = 0 ; p < MAX_ABE_PFMS ; p++)
    {
      if (data_list[p])
	{
	  for (int32_t i = 0 ; i < data_rows[p] ; i++)
	    {
	      for (int32_t j = 0 ; j < data_cols[p] ; j++)
		{
                  //  I really need to test to see if the point is occluded but I can't get this to work.

                  /*
                  GLint sample = 0;
                  GLuint q;
                  glGenQueries (1, &q);
                  if (!q)
                    {
                      fprintf(stderr, "%s %s %d glGenQueries failed\n",__FILE__,__FUNCTION__,__LINE__);
                      exit (-1);
                    }
                  glBeginQuery (GL_SAMPLES_PASSED, q);
                  {
                    glVertex3dv (data_vertices[p][i][j]);
                  }
                  glEndQuery (GL_SAMPLES_PASSED);
                  glGetQueryObjectiv (q, GL_QUERY_RESULT, &sample);
                  glDeleteQueries (1, &q);
                  

                  if (sample)
                  */
                    {
                      Vector3 pvPoint = Vector3 (data_vertices[p][i][j][0], data_vertices[p][i][j][1],
                                                 data_vertices[p][i][j][2]);

                      Vector3 vSelSlope;
                      VMV3 (vSelSlope, pvPoint, vClickRayP1);
                      vSelSlope.Normalize ();

                      Vector3 res;
                      VMV3 (res, vSelSlope, vClickSlope);
                      double TestDist = res.Length ();


                      if (TestDist < MinDistance)
                        {
                          ret = NVTrue;

                          MinDistance = TestDist;
                          *pfm = p;
                          *row = i;
                          *col = j;
                        }
                    }
		}
	    }
	}
    }

  return (ret);
}



//!  Get the nearest point object to the ray defined by nwx, nwy, nwz (near) and fwx, fwy, and fwz (far).

uint8_t 
nvMapGL::getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, NVMAPGL_POINT_OBJECT *data, int32_t count, int32_t *index)
{
  Vector3 vClickRayP1 = Vector3 (nwx, nwy, nwz);
  Vector3 vClickRayP2 = Vector3 (fwx, fwy, fwz);

  Vector3 vClickSlope;
  VMV3 (vClickSlope, vClickRayP2, vClickRayP1);
  vClickSlope.Normalize ();


  double MinDistance = 99999999999.0;

  if (count)
    {
      for (int32_t i = 0 ; i < count ; i++)
        {
	  Vector3 pvPoint = Vector3 (data[i].v[0], data[i].v[1], data[i].v[2]);

	  Vector3 vSelSlope;
	  VMV3 (vSelSlope, pvPoint, vClickRayP1);
	  vSelSlope.Normalize ();

	  Vector3 res;
	  VMV3 (res, vSelSlope, vClickSlope);
	  double TestDist = res.Length ();

	  if (TestDist < MinDistance)
	    {
	      MinDistance = TestDist;
	      *index = i;
	    }
        }
      return (NVTrue);
    }

  return (NVFalse);
}



//!  Get the nearest data point to the ray defined by nwx, nwy, nwz (near) and fwx, fwy, and fwz (far).

uint8_t 
nvMapGL::getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, GLfloat *data, int32_t count, int32_t *index)
{
  Vector3 vClickRayP1 = Vector3 (nwx, nwy, nwz);
  Vector3 vClickRayP2 = Vector3 (fwx, fwy, fwz);

  Vector3 vClickSlope;
  VMV3 (vClickSlope, vClickRayP2, vClickRayP1);
  vClickSlope.Normalize ();


  double MinDistance = 99999999999.0;

  if (count)
    {
      for (int32_t i = 0 ; i < count ; i++)
        {
	  Vector3 pvPoint = Vector3 (data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);

	  Vector3 vSelSlope;
	  VMV3 (vSelSlope, pvPoint, vClickRayP1);
	  vSelSlope.Normalize ();

	  Vector3 res;
	  VMV3 (res, vSelSlope, vClickSlope);
	  double TestDist = res.Length ();


	  if (TestDist < MinDistance)
	    {
	      MinDistance = TestDist;
	      *index = i;
	    }
        }
      return (NVTrue);
    }

  return (NVFalse);
}



//!  Get the nearest data point to the ray defined by nwx, nwy, nwz (near) and fwx, fwy, and fwz (far).  This ignores transparent data.

uint8_t 
nvMapGL::getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, GLfloat *data, GLubyte *color, int32_t count, int32_t *index)
{
  Vector3 vClickRayP1 = Vector3 (nwx, nwy, nwz);
  Vector3 vClickRayP2 = Vector3 (fwx, fwy, fwz);

  Vector3 vClickSlope;
  VMV3 (vClickSlope, vClickRayP2, vClickRayP1);
  vClickSlope.Normalize ();


  double MinDistance = 99999999999.0;

  if (count)
    {
      *index = -1;

      for (int32_t i = 0 ; i < count ; i++)
        {
          //  We want to ignore points that have alpha values less than 255.  This is used for slicing in the
          //  3D editor.

          if (color[i * 4 + 3] == 255)
            {
              Vector3 pvPoint = Vector3 (data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);

              Vector3 vSelSlope;
              VMV3 (vSelSlope, pvPoint, vClickRayP1);
              vSelSlope.Normalize ();

              Vector3 res;
              VMV3 (res, vSelSlope, vClickSlope);
              double TestDist = res.Length ();


              if (TestDist < MinDistance)
                {
                  MinDistance = TestDist;
                  *index = i;
                }
            }
        }

      if (*index < 0) return (NVFalse);

      return (NVTrue);
    }

  return (NVFalse);
}



//!  Get the nearest marker object to the ray defined by nwx, nwy, nwz (near) and fwx, fwy, and fwz (far).

uint8_t 
nvMapGL::getNearestPoint (float nwx, float nwy, float nwz, float fwx, float fwy, float fwz, NVMAPGL_MARKER_OBJECT *data, int32_t count, int32_t *index)
{
  Vector3 vClickRayP1 = Vector3 (nwx, nwy, nwz);
  Vector3 vClickRayP2 = Vector3 (fwx, fwy, fwz);

  Vector3 vClickSlope;
  VMV3 (vClickSlope, vClickRayP2, vClickRayP1);
  vClickSlope.Normalize ();


  double MinDistance = 99999999999.0;

  if (count)
    {
      for (int32_t i = 0 ; i < count ; i++)
        {
	  Vector3 pvPoint = Vector3 (data[i].v[0], data[i].v[1], data[i].v[2]);

	  Vector3 vSelSlope;
	  VMV3 (vSelSlope, pvPoint, vClickRayP1);
	  vSelSlope.Normalize ();

	  Vector3 res;
	  VMV3 (res, vSelSlope, vClickSlope);
	  double TestDist = res.Length ();


	  if (TestDist < MinDistance)
	    {
	      MinDistance = TestDist;
	      *index = i;
	    }
        }
      return (NVTrue);
    }

  return (NVFalse);
}



//!  Given a screen X and Y position return the nearest 3D coordinates.  This one returns map coordinates and is not "protected".

void 
nvMapGL::get3DCoords (int32_t x, int32_t y, double *map_x, double *map_y, double *map_z)
{
  NV_F32_COORD3 cursor_pos;

  get3DCoords (x, y, &cursor_pos);

  screen_to_map (cursor_pos.x, cursor_pos.y, cursor_pos.z, map_x, map_y, map_z);
}



//!  Given a screen X and Y position return the nearest 3D coordinates.

void 
nvMapGL::get3DCoords (int32_t x, int32_t y, NV_F32_COORD3 *coords)
{
  GLdouble d_nwx, d_nwy, d_nwz, d_fwx, d_fwy, d_fwz;
  float nwx, nwy, nwz, fwx, fwy, fwz;


  coords->x = 0.0;
  coords->y = 0.0;
  coords->z = 0.0;


  glGetIntegerv (GL_VIEWPORT, viewport);
  glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
  glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);


  //  We're trying to find the nearest point (either bin vertex, point, or feature depending
  //  on the mode) and then hand the position back to the caller.  You have to get the X and 
  //  Y position of the cursor at the near clipping plane (z = 0.0) and the far clipping 
  //  plane (z = 1.0) of the frustum and then find the nearest vertex to the ray that goes
  //  between them.  We then want to project the vertex position to a 2D point on the screen.


  //  viewport[3] is height of window in pixels.  OpenGL renders with (0,0) on bottom,
  //  mouse reports with (0,0) on top.

  GLint real_y = viewport[3] - y - 1;


  //  Near plane (z = 0.0).

  gluUnProject ((GLfloat) x, (GLfloat) real_y, 0.0, mvmatrix, projmatrix, viewport, &d_nwx, &d_nwy, &d_nwz);
  nwx = (GLfloat) d_nwx;
  nwy = (GLfloat) d_nwy;
  nwz = (GLfloat) d_nwz;


  //  Far plane (z = 1.0).

  gluUnProject ((GLfloat) x, (GLfloat) real_y, 1.0, mvmatrix, projmatrix, viewport, &d_fwx, &d_fwy, &d_fwz);
  fwx = (GLfloat) d_fwx;
  fwy = (GLfloat) d_fwy;
  fwz = (GLfloat) d_fwz;


  int32_t index = -1;
  switch (map.mode)
    {
    case NVMAPGL_BIN_MODE:

      int32_t pfm, row, col;

      if (getNearestLayerVertex (nwx, nwy, nwz, fwx, fwy, fwz, data_list, data_vertices, data_rows, data_cols, &pfm, &row, &col))
        {
          coords->x = data_vertices[pfm][row][col][0];
          coords->y = data_vertices[pfm][row][col][1];
          coords->z = data_vertices[pfm][row][col][2];
        }
      break;


    case NVMAPGL_POINT_MODE:

      if (getNearestPoint (nwx, nwy, nwz, fwx, fwy, fwz, point_data.data (), point_color_data.data (), point_count, &index))
	{
	  coords->x = point_data[index * 3];
	  coords->y = point_data[index * 3 + 1];
	  coords->z = point_data[index * 3 + 2];
	}
      break;


    case NVMAPGL_SPARSE_MODE:

      if (getNearestPoint (nwx, nwy, nwz, fwx, fwy, fwz, sparse_data.data (), sparse_count, &index))
	{
	  coords->x = sparse_data[index * 3];
	  coords->y = sparse_data[index * 3 + 1];
	  coords->z = sparse_data[index * 3 + 2];
	}
      break;


    case NVMAPGL_FEATURE_MODE:

      if (getNearestPoint (nwx, nwy, nwz, fwx, fwy, fwz, feature_data.data (), feature_count, &index))
	{
	  coords->x = feature_data[index].v[0];
	  coords->y = feature_data[index].v[1];
	  coords->z = feature_data[index].v[2];
	}
      break;
    }
}



//!  Special case for marker points (we use them to mark highlighted points in the editors).

void 
nvMapGL::getMarker3DCoords (int32_t x, int32_t y, double *map_x, double *map_y, double *map_z)
{
  GLdouble nwx, nwy, nwz, fwx, fwy, fwz;
  NV_F32_COORD3 coords;


  coords.x = 0.0;
  coords.y = 0.0;
  coords.z = 0.0;


  glGetIntegerv (GL_VIEWPORT, viewport);
  glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
  glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);


  //  We're trying to find the nearest marker point and then hand the position back to the caller.  You have to get the X and 
  //  Y position of the cursor at the near clipping plane (z = 0.0) and the far clipping plane (z = 1.0) of the frustum and
  //  then find the nearest vertex to the ray that goes between them.  We then want to project the vertex position to a 2D
  //  point on the screen.


  //  viewport[3] is height of window in pixels.  OpenGL renders with (0,0) on bottom mouse reports with (0,0) on top.

  GLint real_y = viewport[3] - y - 1;


  //  Near plane (z = 0.0).

  gluUnProject ((GLfloat) x, (GLfloat) real_y, 0.0, mvmatrix, projmatrix, viewport, &nwx, &nwy, &nwz);


  //  Far plane (z = 1.0).

  gluUnProject ((GLfloat) x, (GLfloat) real_y, 1.0, mvmatrix, projmatrix, viewport, &fwx, &fwy, &fwz);


  int32_t index = -1;

  if (getNearestPoint (nwx, nwy, nwz, fwx, fwy, fwz, marker_data.data (), marker_count, &index))
    {
      coords.x = marker_data[index].v[0];
      coords.y = marker_data[index].v[1];
      coords.z = marker_data[index].v[2];

      screen_to_map (coords.x, coords.y, coords.z, map_x, map_y, map_z);
    }
}



/*!  This function will compute a fake (faux) position in the plane of the anchor point that is in the ray
     through the frustum of the pixel X,Y position.  I'm interpolating the position because I'm not sure 
     if there is a way to invert the ray distance to a position.  Even if there is I wouldn't know how to do
     it ;-)  */

void 
nvMapGL::getFaux3DCoords (double anchor_x, double anchor_y, double anchor_z, int32_t pixel_x, int32_t pixel_y, NV_F64_COORD3 *coords)
{
  GLdouble nwx, nwy, nwz, fwx, fwy, fwz;
  int32_t apx, apy;


  //  First we have to get the 2D coordinates of the point we want to measure from.

  get2DCoords (anchor_x, anchor_y, anchor_z, &apx, &apy);


  //  Convert the map position to screen (OpenGL) position

  GLfloat sx, sy, sz;
  map_to_screen (anchor_x, anchor_y, anchor_z, &sx, &sy, &sz);


  coords->x = 0.0;
  coords->y = 0.0;
  coords->z = 0.0;


  glGetIntegerv (GL_VIEWPORT, viewport);
  glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);
  glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);


  //  We have to get the OpenGL coordinates of the screen position of the anchor point at the near clipping plane (z = 0.0) and the far clipping 
  //  plane (z = 1.0) of the frustum.  We'll use these and the OpenGL position of the anchor point to interpolate the faux point.

  //  viewport[3] is height of window in pixels.  OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top.

  GLint real_y = viewport[3] - apy - 1;


  //  Near plane (z = 0.0).

  gluUnProject ((GLfloat) apx, (GLfloat) real_y, 0.0, mvmatrix, projmatrix, viewport, &nwx, &nwy, &nwz);


  //  Far plane (z = 1.0).

  gluUnProject ((GLfloat) apx, (GLfloat) real_y, 1.0, mvmatrix, projmatrix, viewport, &fwx, &fwy, &fwz);


  //  Now determine the OpenGL coordinates of the pixel position at the near and far planes

  GLdouble faux_nwx, faux_nwy, faux_nwz, faux_fwx, faux_fwy, faux_fwz;
  real_y = viewport[3] - pixel_y - 1;


  //  Near plane (z = 0.0).

  gluUnProject ((GLfloat) pixel_x, (GLfloat) real_y, 0.0, mvmatrix, projmatrix, viewport, &faux_nwx, &faux_nwy, &faux_nwz);


  //  Far plane (z = 1.0).

  gluUnProject ((GLfloat) pixel_x, (GLfloat) real_y, 1.0, mvmatrix, projmatrix, viewport, &faux_fwx, &faux_fwy, &faux_fwz);


  //  Interpolate X, Y, and Z for the faux point using the near and far coordinates from the anchor point and the anchor point coordinates

  GLfloat nx, ny, nz;

  nx = faux_nwx + (faux_fwx - faux_nwx) * ((sx - nwx) / (fwx - nwx));
  ny = faux_nwy + (faux_fwy - faux_nwy) * ((sy - nwy) / (fwy - nwy));
  nz = faux_nwz + (faux_fwz - faux_nwz) * ((sz - nwz) / (fwz - nwz));


  //  Finally, convert the OpenGL coordinates to map coordinates

  screen_to_map (nx, ny, nz, &coords->x, &coords->y, &coords->z);
}



//!  Convert Qt penStyle to an OpenGL stipple pattern.

GLushort nvMapGL::penStyle_to_stipple (Qt::PenStyle penStyle)
{
  switch (penStyle)
    {
    case (Qt::NoPen):
      return (0x0000);
      break;

    case (Qt::SolidLine):
      return (0xFFFF);
      break;

    case (Qt::DashLine):
      return (0x3333);
      break;

    case (Qt::DotLine):
      return (0x5555);
      break;

    case (Qt::DashDotLine):
      return (0x4C99);
      break;

    case (Qt::DashDotDotLine):
      return (0x5353);
      break;

    case (Qt::CustomDashLine):
    case (Qt::MPenStyle):
      return (0);
      break;
    }
  return (0);
}



/*!

   - method: setRenderMode 

   - This method will simply set a flag variable for paintGL to successfully draw the
     more efficient points or the overhead of drawing lit spheres as each data point.
     we also change the lighting properties depending on whether we are drawing the shots
     as points or some other shape.
*/

void nvMapGL::setRenderMode (int32_t mode)
{		
  map.renderMode = mode;
  setRenderModeAmbience (map.renderMode);	

  generateDataPointsDL (point_count);
  generateSparsePointsDL (sparse_count);

  updateGL ();
}



/*!

   -  method: setRenderModeAmbience

   - if we have the light model being that of an ambient light model, we will adjust
     the ambient properties to full intensity when drawing points so maximum brightness
     is achieved.  if we are drawing surface shapes, we will have a slight ambient intensity
     to highlight indirect faces within the scene.
*/

void nvMapGL::setRenderModeAmbience (int32_t mode)
{
  if (map.light_model == GL_LIGHT_MODEL_AMBIENT)
    {
      GLfloat little_ambient[] = {0.2, 0.2, 0.2, 1.0};
      GLfloat lot_ambient[] = {1.0, 1.0, 1.0, 1.0};

      if (mode == NVMAPGL_RENDER_POINT_MODE)
        {
          glLightfv (GL_LIGHT0, GL_AMBIENT, lot_ambient);
          glLightModelfv (map.light_model, lot_ambient);
        }
      else
        {
          glLightfv (GL_LIGHT0, GL_AMBIENT, little_ambient);
          glLightModelfv (map.light_model, little_ambient);
        }
    }
}


/*!

   - method: generateDataPointsDL
   
   - This method will create the display list based on the render mode for the data points.  Most of
     this code is copied from above.  A couple of alterations were made.   

   - BIG ASSUMPTION:  From my debug printouts and perusing over the preferences section, I wanted to make
     sure that a user could not change the alpha value of data shots directly.  The only way that I saw the
     alphas changing was through the slicing.  Because the slicing turns off the depth testing, the complex
     shapes within the slice were kind of "muddled" because of the ordering of the faces of the objects (last
     guy wins basically).  So there was a big dropoff in quality when we went to slicing.  So, the editor sends
     over the color and it affects the transparency values.  Shots within the slice still had a full transparency
     value (255) but because of the depth testing being turned off, the shading was wacky.  So, I looked at the
     tranparency values and once I hit one that was not a 255, the BIG ASSUMPTION was we are slicing and that
     particular shot was outside the slice because according to all my printouts, I got 255's for everything within
     a slice and some other value depending on the transparency slider for the other values.
*/

void nvMapGL::generateDataPointsDL (int32_t count)
{
  //  Generate a display list for the points

  if (count)
    {
      if (point_list) glDeleteLists (point_list, 1);

      point_list = glGenLists (1);

      if (point_list == 0)
        {
          fprintf (stderr, "Unable to generate points display list\n");
          exit (-1);
        }


      //  I realize that I don't need the brackets after the glNewList statement and before the glEndList statement 
      //  but it sets off the code and allows XEmacs to indent it nicely.  Same for glBegin and glEnd.  JCD

      glNewList (point_list, GL_COMPILE);
      {
        if (map.renderMode == NVMAPGL_RENDER_SPHERE_MODE)
          {
            GLUquadricObj *quadric = gluNewQuadric ();

            gluQuadricNormals (quadric, GLU_SMOOTH);
            gluQuadricDrawStyle (quadric, GLU_FILL);

            uint8_t possibleSlice = NVFalse;

            for (int32_t i = 0; i < count; i++)
              {
                glPushMatrix ();
                glTranslatef (point_data[i * 3], point_data[i * 3 + 1], point_data[i * 3 + 2]);

                /* just takes one hit to assume we may be in slice mode */

                if (!possibleSlice)
                  {
                    if (point_color_data[i * 4 + 3] < 255) possibleSlice = NVTrue;
                  }

                /* we will only get here if we think we are in slice mode and if we are
                   we evaluate all shots, if it is a 255 it is within the slice and we
                   do want hidden surface removal */

                else if (point_color_data[i * 4 + 3] == 255)
                  {
                    setDepthTest (NVTrue);
                  }

                /* if it is outside the slice, we can blend it in */

                else
                  {
                    setDepthTest (NVFalse);
                  }

                glColor4ubv (&point_color_data[i * 4]);

                gluSphere (quadric, map.complexObjectWidth / 2.0, map.complexDivisionals, map.complexDivisionals);

                glPopMatrix ();
              }

            if (possibleSlice) setDepthTest (NVFalse);

            gluDeleteQuadric (quadric);
          }


        /* NVMAPGL_RENDER_POINT_MODE */

        else
          {
            //  Technically, your colors were all off because of the lighting being applied to the points.  It's gets a little
            //  dicey with lighting points in OpenGL.  The same color values were being sent to the AV and it wasn't even close
            //  to a match until the lighing was turned on.  Same test was run for the spheres and the colors matched up with
            //  lighting.  Like I said the point primitive gets affected greatly with lighting.  Could be a graphics card thing.
            //  But even then, you're wasting cycles computing lighting calculations with points since there is no shading.
            //  win, win (Gary Morris)

            //  Push our current settings to the matrix, then turn off lighting

            glPushAttrib (GL_LIGHTING_BIT);
            glDisable (GL_LIGHTING);


            glVertexPointer (3, GL_FLOAT, 0, point_data.data ());
            glColorPointer (4, GL_UNSIGNED_BYTE, 0, point_color_data.data ());

            glPointSize ((GLfloat) point_data_size);

            glBegin (GL_POINTS);
            {
              for (int32_t i = 0 ; i < count ; i++) glArrayElement (i);
            }
            glEnd ();

            glPopAttrib ();
          }
      }
      glEndList ();
    }
  point_count = count;
}



/*!

   - method: generateSparsePointsDL
   
   - This method will create the display list based on the render mode for the sparse points.
     See the BIG ASSUMPTION comment in the generateDataPointsDL section.
*/

void nvMapGL::generateSparsePointsDL (int32_t count)
{
  //  Generate a display list for the points

  if (count)
    {
      if (sparse_list) glDeleteLists (sparse_list, 1);

      sparse_list = glGenLists (1);

      if (sparse_list == 0)
        {
          fprintf (stderr, "Unable to generate sparse display list\n");
          exit (-1);
        }

      //  I realize that I don't need the brackets after the glNewList statement and before the glEndList statement 
      //  but it sets off the code and allows XEmacs to indent it nicely.  Same for glBegin and glEnd.  JCD

      glNewList (sparse_list, GL_COMPILE);
      {
        if (map.renderMode == NVMAPGL_RENDER_SPHERE_MODE)
          { 
            uint8_t possibleSlice = NVFalse;

            GLUquadricObj *quadric = gluNewQuadric ();

            gluQuadricNormals (quadric, GLU_SMOOTH);
            gluQuadricDrawStyle (quadric, GLU_FILL);

            for (int32_t i = 0; i < count; i++)
              {
                glPushMatrix ();
                glTranslatef (sparse_data[i * 3], sparse_data[i * 3 + 1], sparse_data[i * 3 + 2]);

                if (!possibleSlice)
                  {
                    if (sparse_color_data[i * 4 + 3] < 255) possibleSlice = NVTrue;
                  }
                else if (sparse_color_data[i * 4 + 3] == 255)
                  {
                    setDepthTest (NVTrue);
                  }
                else
                  {
                    setDepthTest (NVFalse);
                  }

                glColor4ubv (&sparse_color_data[i * 4]);

                gluSphere (quadric, map.complexObjectWidth / 2.0, map.complexDivisionals, map.complexDivisionals);

                glPopMatrix ();
              }

            if (possibleSlice) setDepthTest (NVFalse);

            gluDeleteQuadric (quadric);
          }


        /* NVMAPGL_RENDER_POINT_MODE */

        else
          {
            glVertexPointer (3, GL_FLOAT, 0, sparse_data.data ());
            glColorPointer (4, GL_UNSIGNED_BYTE, 0, sparse_color_data.data ());

            glPointSize ((GLfloat) sparse_data_size);

            glBegin (GL_POINTS);
            {
              for (int32_t i = 0 ; i < count ; i++) glArrayElement (i);
            }

            glEnd ();
          }			
      }

      glEndList ();
    }

  sparse_count = count;
}



void nvMapGL::setComplexObjectWidth (float objWidth)
{
  map.complexObjectWidth = objWidth;	
}



void nvMapGL::setComplexDivisionals (int32_t divisionals)
{
  map.complexDivisionals = divisionals;
}
