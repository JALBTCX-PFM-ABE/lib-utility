
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


#ifndef NVUTILITY_VERSION

#define     NVUTILITY_VERSION     "PFM Software - nvutility library V2.2.43 - 08/07/19"

#endif

/*!< <pre>

    Version 1.00
    04/01/08
    Jan C. Depner

    This is the first version of the nvutility library.  April Fools!  Actually, this is just
    the first time we've had a version file.  The first version of nvutility was somewhere back
    in 1998.


    Version 1.01
    04/07/08
    Jan C. Depner

    Added bit_test and bit_set to chrtr.c.  Added get_string.c.  Added nvutility.h which contains
    includes of all of the .h and .hpp files.  Removed getopt_win.h, getopt1.c, and getopt.c (not used).
    Had to change the POINT structure name to CONTOUR_POINT in globals.hpp because POINT is
    already defined in MinGW.


    Version 1.02
    04/08/08
    Jan C. Depner

    Removed geod.c, geod.h, and ellipsoid.h.  These were a major problem in Windows and in all but one
    case they were in the application directories anyway.


    Version 1.03
    04/09/08
    Jan C. Depner

    Added registerABE.cpp, registerABE.hpp, unregisterABE.cpp, unregisterABE.hpp, changeFileRegisterABE.cpp, and
    changeFileRegisterABE.hpp.


    Version 1.04
    04/24/08
    Jan C. Depner

    Fixed a small probelm in check_target_schema.c


    Version 1.05
    04/25/08
    Jan C. Depner

    Leap year bug in inv_cvtime.


    Version 1.06
    05/20/08
    Jan C. Depner

    Modified get_area_mbr to adjust west longitudes to positive if the area crosses the dateline.  Also,
    made read_srtm_mask and read_coast deal with the 0-360 world correctly.


    Version 1.07
    06/09/08
    Jan C. Depner

    Fix one-off error in nvmap.cpp when reading the landmask.  Also, modified the comments in read_srtm_mask.c.


    Version 1.08
    06/10/08
    Jan C. Depner

    Added command capability to ABE_register.hpp.


    Version 1.09
    07/02/08
    Jan C. Depner

    Added NV_I32_COORD2 and NV_F64_COORD2 versions of drawPolygon to nvmap.cpp.


    Version 1.10
    07/14/08
    Jan C. Depner

    nvmap::resetBounds wasn't resetting initial_bounds in addtion to bounds[0].


    Version 1.11
    07/21/08
    Jan C. Depner

    get_area_mbr wasn't reading afs files properly.


    Version 1.12
    09/11/08
    Jan C. Depner

    movable_object_count was getting hosed in nvmap.cpp.


    Version 2.0.0
    09/17/08
    Jan C. Depner

    Had to make it a static library on Windoze due to my inability to make a shared version of 
    the target library.  Google for __declspec and MINGW to see what's happening.


    Version 2.0.1
    10/02/08
    Jan C. Depner

    Added drawTextClear, getZoomLevel, and clipLine functions to nvmap.cpp.  Fixed MANY dateline issues.


    Version 2.0.2
    10/08/08
    Jan C. Depner

    More dateline insanity in nvmap.cpp.  The comment looks like this:

      //  Arbitrary cutoff - if we're crossing the dateline and one point is negative and the other is >=0
      //  and either point is more than 90 degrees outside of the boundaries we're going to say that this line
      //  is completely outside of the area.  If you want to do really large areas that cross the datelin you
      //  need to use 0-360 world always.


    Version 2.0.3
    11/24/08
    Jan C. Depner

    Added squat.cpp, squat.hpp, and vec.h.  These are used for 3D OpenGL programs
    like pfm3D and pfmEdit3D.


    Version 2.0.4
    12/02/08
    Jan C. Depner

    Fixed bug in the rubberband line code of nvmap.cpp and nvpic.cpp.


    Version 2.0.5
    12/03/08
    Jan C. Depner

    Fixed check_target_schema bug.


    Version 2.0.6
    01/08/09
    Jan C. Depner

    More dateline stupidity.  This time in zoomOutPercent.


    Version 2.0.7
    02/11/09
    Jan C. Depner

    Added a couple of new flags to ABE.h.  PFMVIEW_FORCE_EDIT, PFMVIEW_FORCE_EDIT_3D, PFMEDIT3D_OPENED, and
    PFMEDIT3D_CLOSED.  See the comments in ABE.h.


    Version 2.0.8
    02/24/09
    Jan C. Depner

    Bug fix for get_geoid03.c


    Version 2.0.9
    04/08/09
    Jan C. Depner

    Modified nvmap.cpp to have an NV_BOOL "flush" argument for all drawing routines.


    Version 2.0.10
    04/27/09
    Jan C. Depner

    Added inside_polygon and inside_polygon2.


    Version 2.0.11
    05/21/09
    Jan C. Depner

    Added nvMapGl.cpp and .hpp after removing them from the pfm3D, pfmEdit3D, and bagViewer directories.


    Version 2.0.12
    05/22/09
    Jan C. Depner

    Removed the redrawMapWithCheck call from the resize event.  This should be handled by the 
    application, not the library.


    Version 2.0.13
    06/02/09
    Jan C. Depner

    Added setMovingText to nvmap.cpp.


    Version 2.0.14
    06/18/09
    Jan C. Depner

    Replaced double_bit_pack and double_bit_unpack in bit_pack.c with improved code from Will Burros (IVS).


    Version 2.0.15
    Jan C. Depner
    07/29/09

    Changed %lf in printf statements to %f.  The MinGW folks broke this even though it works on every
    compiler known to man (including theirs up to the last version).  No compiler warnings or errors
    are issued.  Many thanks to the MinGW folks for fixing what wasn't broken.


    Version 2.0.16
    Jan C. Depner
    08/17/09

    Added a change to nvMapGL.cpp to only return fully opaque points when performing a 
    nearest point check.  This is used for slicing in pfmEdit3D.


    Version 2.0.17
    Jan C. Depner
    08/20/09

    Added drawLine to nvMapGL.cpp.  This allows you to draw a 2D line over the 3D data.


    Version 2.0.18
    Jan C. Depner
    10/28/09

    Added dummyBuffer to nvmap.cpp.  Added check_srtm_mask to read_srtm_mask.c.


    Version 2.0.19
    Jan C. Depner
    11/19/09

    Added polygon_is_rectangle and polygon_is_rectangle2 to get_area_mbr.c.


    Version 2.0.20
    Jan C. Depner
    12/02/09

    Added get_egm08.c.


    Version 2.0.21
    Jan C. Depner
    12/09/09

    Added WA_DELETE_ON_CLOSE to nvMapGL.cpp.


    Version 2.0.22
    Jan C. Depner
    12/11/09

    Added polygon_collision functions to inside_polygon.c and inside_polygon.h.


    Version 2.0.23
    Jan C. Depner
    01/07/10

    Fixed bug in read_srtm_mask.cpp.


    Version 2.0.24
    Jan C. Depner
    01/11/10

    Fixed equator bug in read_srtm_mask.cpp.


    Version 2.0.25
    Jan C. Depner
    01/19/10

    Fixed yet another equator bug in read_srtm_mask.cpp.


    Version 2.0.26
    Jan C. Depner
    02/02/10

    Increased the number of points in an AREA structure to 1000.  AFAIK this is only used in apply_tides.


    Version 2.0.27
    Jan C. Depner
    02/09/10

    Added sunshade.cpp and survey.cpp.


    Version 2.0.28
    Jan C. Depner
    03/16/10

    Added polygon_intersection.c and polygon_intersection.h.


    Version 2.0.29
    Jan C. Depner
    04/09/10

    Added NVMAP_UP_LEFT, NVMAP_UP_RIGHT, NVMAP_DOWN_LEFT, and NVMAP_DOWN_RIGHT to the possible move directions in
    nvmap.cpp, nvmap.hpp, nvMapGL.cpp, and nvMapGL.hpp.


    Version 2.0.30
    Jan C. Depner
    04/14/10

    Added scaleBox.cpp and scaleBox.hpp.  This is a Qt widget for drawing colored, numbered scale boxes.


    Version 2.0.31
    Jan C. Depner
    04/29/10

    Changed fixpos to use numeric constants instead of character strings to set type and format.  Same with rproj and read_coast.
    Changed many "char *" arguments to "const char *" arguments for gcc 4.4 compatibility (when calling with a constant).


    Version 2.0.32
    Jan C. Depner
    06/01/10

    Fixed contour default initializations in contour.cpp.


    Version 2.0.33
    Jan C. Depner
    06/07/10

    Added geo_distance.c (stolen from PFM library and made generic).


    Version 2.0.34
    Jan C. Depner
    07/07/10

    Modified scaleBox to include a flag for drawing a lock in the scaleBox (on the right).


    Version 2.0.35
    Jan C. Depner
    07/19/10

    Added inside_mbr and inside_xymbr to inside.c.


    Version 2.0.36
    Jan C. Depner
    07/22/10

    Added martin.c and convolve.c to support CHRTR2 gridEdit program.


    Version 2.1.0
    Gary Morris (USM)
    09/20/10

    Updates to merge in latest CZMIL changes from Gary Morris (USM)


    Version 2.1.1
    Jan C. Depner
    09/24/10

    Added clip_lat_lon to line_intersection.c.


    Version 2.1.2
    Jan C. Depner
    10/22/10

    Fixed memory leak in nvpic.cpp - I hope.


    Version 2.1.3
    Jan C. Depner
    11/05/10

    Finally fixed auto Z scaling in nvMapGL.  Removed warning to stderr in read_coast.c when it can't
    find coast50k.ccl since it is restricted and might not be available.


    Version 2.1.4
    Jan C. Depner
    11/30/10

    Finally added a real scale to the 3D stuff in nvMapGL.cpp.


    Version 2.1.5
    Jan C. Depner
    12/14/10

    Moved the scale to the background in nvMapGL.cpp.  Added clearMarkerPoints and setMarkerPoints to nvMapGL.cpp so 
    that we can finally mark points nicely.


    Version 2.1.6
    Jan C. Depner
    12/22/10

    Added unlock icon to scaleBox.cpp.  We now have three modes -1 = unlocked, 0 = no lock, 1 = locked.


    Version 2.1.7
    Jan C. Depner
    01/05/11

    DOH!  I missed a couple of %lf fields in printf statements (see version 2.0.15 of 07/29/09).


    Version 2.1.8
    Jan C. Depner
    01/07/11

    Added getFaux3DCoords to nvMapGL in order to get the distance, azimuth, and delta Z of the cursor when measuring in 3D
    point editors.


    Version 2.1.9
    Jan C. Depner
    01/15/11

    Fixed bug in resize code of nvMapGL.cpp.  I gotta learn to typecast ints to floats when I do math ;-)


    Version 2.1.10
    Jan C. Depner
    01/27/11

    Fixed bug in find_startup.c and find_startup_name.c.  The last directory in the PATH wasn't being searched.


    Version 2.1.11
    Jan C. Depner
    01/28/11

    Fixed a light ambience problem introduced by Gary's setRenderModeAmbience stuff.  We don't want to do that when we're
    drawing a 3D surface, as opposed to dicrete points.


    Version 2.1.12
    Gary Morris (USM)
    02/18/11

    Turned off lighting when in point mode.  It makes the colors match better when you go to a non-OpenGL program.  
    Hopefully it's also a bit faster.


    Version 2.1.13
    Jan Depner
    02/21/11

    Removed references to lat and lon from nvMapGL.cpp and nvMapGL.hpp since we're not always dealing with lat and lon.
    Sometimes it's northing and easting or it may be just X and Y.


    Version 2.1.14
    Jan Depner
    03/30/11

    Added setAutoScale to nvMapGL.cpp and nvMapGL.hpp so we can turn auto-scaling on and off at will.


    Version 2.1.15
    Jan C. Depner
    04/07/11

    The min and max depth HSV values, colors, and locks for pfmView, pfmEdit(3D), and pfmEditShell have been added to ABE.h (ABE
    shared memory).


    Version 2.1.16
    Jan C. Depner
    04/21/11

    Modified get_egm_08 to return -1 for an error when trying to set the grid type.


    Version 2.1.17
    Jan C. Depner
    05/09/11

    Added getMarker3DCoords to nvMapGL.cpp so we could find the nearest "highlighted" (i.e. marker) point in
    pfmEdit3D.  The lat and lon returned by mouseMoveEvent is the nearest of all displayed points when in 
    NVMAPGL_POINT_MODE.  Due to that we needed to be able to find the nearest marker point in the slot
    connected to the nvMapGL mouseMoveSignal (slotMouseMove in pfmEdit3D).


    Version 2.1.18
    Jan C. Depner
    05/20/11

    Added pmt and apd ac zero offset thresholds to the hofWaveFilter parameters in the ABE.h file.  Note that this
    is a change to shared memory and may cause temporary problems until ipcclean is run or the syatem is rebooted.


    Version 2.1.19
    Jan C. Depner
    07/14/11

    Fixed bug in nvmap.cpp when using NO_PROJECTION.  It was still checking dateline.  DOH!


    Version 2.1.20
    Jan C. Depner
    07/22/11

    Added setSidebarUrls.cpp and .hpp.


    Version 2.1.21
    Jan C. Depner
    10/28/11

    Added getSystemInfo.cpp and .hpp.


    Version 2.1.22
    Jan C. Depner
    11/04/11

    Fixed bug in nvMapGL.cpp when coloring by something other than depth/elevation.


    Version 2.1.23
    Jan C. Depner
    01/04/12

    - The setSidebarUrls function now clears the sidebar and adds only the last used and
    current working directory.  Much less confusing ;-)
    - Minor OpenGL modifications in nvMapGL.cpp (for speed).  Disabled dithering prior to
    clearing and commented out glGetError calls (I'll turn these back on during development).


    Version 2.1.24
    Jan C. Depner
    01/25/12

    Had to put a hack into setSideBarUrls.cpp to handle the filename formatting on Windows.
    It kept coming back with things like /C:/datasets/saipan instead of /c/datasets/saipan
    or C:\datasets\saipan.  toNativeSeparators didn't fix it so I kludged it ;-)


    Version 2.1.25
    Jan C. Depner
    04/24/12

    Added setTextFont to nvmap.cpp and nvmap.hpp to allow setting of the font for text strings.
    This is so we can figure out how big a string is before we draw it.


    Version 2.1.26
    06/26/12
    Jan C. Depner

    Replaced #define's with typedef's for type definitions in the pfm_nvtypes.h file.


    Version 2.1.27
    09/18/13
    Jan C. Depner (PFM Software)

    Changed nvmap.cpp to better handle the new coast50K.ccl.  That is, only use it when less than 5 degrees
    and boundaries changed to 60N to 56S.


    Version 2.1.28
    10/14/13
    Jan C. Depner (PFM Software)

    Fix for silly error in read_srtm_mask.c.  Also added read_srtm_mask_min_res.


    Version 2.1.29
    10/28/13
    Jan C. Depner (PFM Software)

    Added read_swbd_mask.c and read_swbd_mask.h for the new SWBD land mask files.


    Version 2.1.30
    10/30/13
    Jan C. Depner (PFM Software)

    Removed #ifdef NVWIN3X statements from nvMapGL.cpp.  It appears that the stupidity in
    Windows was corrected when they went to Windows 7.  Since we (and Microsoft as of April 2014)
    no longer support Windows XP I've essentially commented out the changes to support XP.


    Version 2.1.31
    12/12/13
    Jan C. Depner (PFM Software)

    Added swbd_is_water function to the read_swbd_mask.c file.


    Version 2.1.32
    12/13/13
    Jan C. Depner (PFM Software)

    Changed the name of the SWBD version of coast50k.ccl to coast_swbd.ccl to avoid any confusion
    with the old coast50k.ccl.  Also, switched away from using single one-degree cell land/water
    files in SWBD to using a single, 64800 byte land_water map file.


    Version 2.1.33
    12/17/13
    Jan C. Depner (PFM Software)

    No longer using the single land_water file mentioned above since the "beat each point against
    every polygon in a shape file" method of determining land and water was unbelievably slow.
    Now using a 3 second land mask made using the "beat each point" method.  Soon (probably
    about ten days from now) I'll be using a one-second mask made from SWBD (yes, it takes about
    10 days to run it on a 16 processor system using all the processors).  Changed the swbd_is_water
    function to swbd_is_land and just grab the point from the mask.


    Version 2.2.00
    12/30/13
    Jan C. Depner (PFM Software)

    Moved all of the WVS_DIR data to a directory called ABE_data that is pointed to by the ABE_DATA
    environment variable.  Moved the SRTM and SWBD data there as well.  The data was also reorganized
    to be easier to keep track of.  There are wvs_wdb, land_mask, geoid_data, srtm_data, and SWBD
    directories in the ABE_data directory.  To paraphrase J. R. R. Tolkein:
        One environment variable to rule them all
        One environment variable to find them
        One environment variable to bring them all
        And in the darkness bind them


    Version 2.2.01
    01/03/14
    Jan C. Depner (PFM Software)

    - Changed swbd_is_land in read_swbd_mask.c to use up to four one-degree cells so that we don't thrash
      back and forth between cells when handling data along a one-degree boundary.
    - Replaced the #ifdef NVWIN3X statements in nvMapGL.cpp.  It appears that the stupidity in
      Windows was NOT corrected when they went to Windows 7.


    Version 2.2.02
    01/05/14
    Jan C. Depner (PFM Software)

    - Fixed get_geoid03.c to get the g... files from geoid_data instead of wvs_wdb.


    Version 2.2.03
    01/07/14
    Jan C. Depner (PFM Software)

    Added get_geoid12a.c and get_geoid12.h.


    Version 2.2.04
    01/21/14
    Jan C. Depner (PFM Software)

    Finally got a CPF file that crossed over a degree boundary so I got a chance to fix
    read_swbd_mask.c since I hadn't been able to test it before (and, of course, it
    didn't work right.


    Version 2.2.05
    02/03/14
    Jan C. Depner (PFM Software)

    Yet another fix for read_swbd_mask.c.  Hopefully I've got it right this time.


    Version 2.2.06
    02/05/14
    Jan C. Depner (PFM Software)

    Updated read_swbd_mask.c to use a least recently used (LRU) algorithm to determine which of
    four cells should be reloaded.


    Version 2.2.07
    02/05/14
    Jan C. Depner (PFM Software)

    Added check_flag.c and check_flag.h to check/set/clear GSF beam flags.
    Replaced HMPS beam flags with NV_GSF beam flags in navo_gsf_flags.h.


    Version 2.2.08
    03/14/14
    Jan C. Depner (PFM Software)

    - Added Kevin Byrd's nav4word_lib.c and nav4word_lib.h (renamed to nav4word.c and nav4word.h).
    - Removed all TAB characters fromnav4word.c and .h (non-ANSI).
    - Removed all use of "continue" by reversing the logic and indenting in brackets ("continue" is nothing
      more or less than a "go to" statement and should be avoided at almost any cost).
    - Added __attribute__ ((unused)) in front of the count parameter in NavRecSplitter (nav4word.c).


    Version 2.2.09
    03/21/14
    Jan C. Depner (PFM Software)

    Removed set_egm08_grid_type from get_egm08.c.  We just use the 1X1 EGM08 now.


    Version 2.2.10
    05/07/14
    Jan C. Depner (PFM Software)

    - Added return value checks for fread to get rid of GCC warnings.  These should never happen.


    Version 2.2.11
    06/13/14
    Jan C. Depner (PFM Software)

    - Changed QPixmap->fill calls that will be deprecated in Qt5 to a more normal usage.  Two places in
      nvmap.cpp and one in nvpic.cpp.  They were only used to initialize the pixmp anyway so I just filled
      with the background color.


    Version 2.2.12
    07/20/14
    Jan C. Depner (PFM Software)

    - Removed nvtypes.h (we now use pfm_nvtypes.h since they were the same).
    - Switched to using data types from stdint.h and inttypes.h instead of the old NV_INT64 kind of things.
      This was actually first done in the PFM library but is picked up via pfm_nvtypes.h.
    - Moved the read_srtmX_topo.c and .h files and the dted.c and dted.h files here.


    Version 2.2.13
    07/29/14
    Jan C. Depner (PFM Software)

    - Fixed some possible resource leaks and undefined fields that were discovered by cppcheck.


    Version 2.2.14
    11/19/14
    Jan C. Depner (PFM Software)

    - Removed cpf pre-pulse filter values from ABE.h.  HydroFusion is now taking care of pre-pulse issues.


    Version 2.2.15
    11/28/14
    Jan C. Depner (PFM Software)

    - Added read_shape_mask.c and read_shape_mask.h to read mask files created by the shape_mask program.


    Version 2.2.16
    12/02/14
    Jan C. Depner (PFM Software)

    - Added get_geoid09.c and get_geoid09.h (added NGS GEOID09 files to ABE_data/geoid_data).


    Version 2.2.20
    12/24/14
    Jan C. Depner (PFM Software)

    - Replaced the large reallocated arrays (point_data, point_color_data, sparse_data, etc) in nvMapGL.cpp and
      nvMapGL.hpp with std::vectors.  I found out yesterday that realloc on Windows is unbelievably slow.  The
      initial draw on Windows is now about two orders of magnitude faster.  Yup, you read that right, 20 times
      faster.  Merry freakin' Christmas ;-)


    Version 2.2.21
    12/28/14
    Jan C. Depner (PFM Software)

    - Made get2DCoords in nvMapGL.cpp not continuously call the glGet* functions.  This really shows up in
      Windows when determining 3D points within a 2D polygon.


    Version 2.2.22
    12/29/14
    Jan C. Depner (PFM Software)

    - New version of inside_polygon.c and inside_polygon.h based on E. Haines.


    Version 2.2.23
    04/02/15
    Jim Hammack (Next Wave Systems)
    Jan C. Depner (PFM Software)

    - Fixed bug in nvMapGL.cpp.  data2_list array was not being initialized to 0.


    Version 2.2.24
    07/14/15
    Jan C. Depner (PFM Software)

    - Added setContents with QString argument (as opposed to float) to scaleBox.cpp.
    - Added setContents with int32_t argument (as opposed to float) to scaleBox.cpp.
    - Added setScaleOffset, setScaleOffsetColor to nvMapGL.cpp to change "box scale" number colors when offsetting
      Z values in pfmEdit3D.


    Version 2.2.25
    07/22/15
    Jan C. Depner (PFM Software)

    - Added attributes.hpp and attributes.cpp since they're used in multiple programs.


    Version 2.2.26
    08/05/15
    Jan C. Depner (PFM Software)

    - Added clickLabel.cpp and clickLabel.hpp since they're used in multiple programs.


    Version 2.2.27
    08/12/15
    Jan C. Depner (PFM Software)

    - Removed attributes.hpp and attributes.cpp even though they're used in multiple programs.  Library cross dependencies... DOH!


    Version 2.2.28
    08/14/15
    Jan C. Depner (PFM Software)

    - Added a new function, setScaleOrientation, to nvMapGL.cpp to allow the "box scale" to be labelled with either depths or
      elevations.


    Version 2.2.29
    10/01/15
    Jan C. Depner (PFM Software)

    - Added line_index to the POINT_CLOUD structure in ABE.h.  This allows pfmEdit3D to give the best possible separation of 
      line colors.  Note that we are not expecting more that 255 lines in any on edit session.


    Version 2.2.30
    04/05/16
    Jan C. Depner (PFM Software)

    - Added get_geoid12b.c and get_geoid12b.h to support geoid12b files added to the ABE_data/geoid_data folder.


    Version 2.2.31
    04/24/16
    Jan C. Depner (PFM Software)

    - Added lidar_class_change to the POINT_CLOUD structure in ABE.h.


    Version 2.2.32
    08/03/16
    Jan C. Depner (PFM Software)

    - Fixed the way paintGL handled line_data from setLines in nvMapGL.cpp.


    Version 2.2.33
    08/15/16
    Jan C. Depner (PFM Software)

    - Added setGridThickness to nvmap.cpp and nvmap.hpp.
    - Fixed bug drawing grid overlay in nvmap.cpp.


    Version 2.2.34
    08/26/16
    Jan C. Depner (PFM Software)

    - Changed the definition in navo_gsf_flags.h from NV_GSF_SELECTED_CONTACT to NV_GSF_SELECTED_FEATURE.
      The flag and its use haven't changed, just the name so that it better describes what we (and SABER)
      use it for.


    Version 2.2.35
    09/27/16
    Jan C. Depner (PFM Software)

    - Added qPosfix.cpp and qPosfix.hpp.


    Version 2.2.40
    06/19/17
    Brett Goldenbloome (Leidos), Jan Depner (PFM Software)

    - After using the bit_pack and bit_unpack functions for 18 years with no apparent problems, Brett Goldenbloome
      at Leidos, Newport RI discovered that, in extremely rare circumstances, the functions would access one byte
      past the end of the buffer.  I have added an "if" statement (shudder) to correct the problem.


    Version 2.2.41
    09/23/17
    Jan Depner (PFM Software)

    - Added acknowledgments.hpp.  This isn't used here in the utility library but it is used in almost all ABE GUI
      programs.  Putting it here allows me to just update it in one place when needed.  In the individual mk files
      for eachprogram I add it to the HEADERS list so that it can get translated for each GUI program.


    Version 2.2.42
    12/18/18
    Jan Depner (PFM Software)

    - Added some "__attribute__ ((fallthrough));" statements in switches to suppress compiler warnings when
      the fallthrough is deliberate.


    Version 2.2.43
    08/07/19
    Jan Depner (PFM Software)

    - Added SHAPE file support to get_area_mbr.c

</pre>*/
