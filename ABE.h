
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



#ifndef _ABE_H_
#define _ABE_H_

#ifdef  __cplusplus
extern "C" {
#endif


#include "pfm_nvtypes.h"
#include "nvdef.h"
#include "pfm.h"


#define         MAX_CONTOUR_LEVELS          200  /*!<  Maximum number of contour levels                                */
#define         MAX_STACK_POINTS            9    /*!<  Maximum stack points (for waveWaterfall from pfmEdit)           */
#define         MAX_ABE_PFMS                16   /*!<  Maximum number of PFM layers for pfmView and pfmEdit            */
#define         MAX_DELETE_FILE_QUEUE       100  /*!<  Maximum number of files allowed on the delete file queue.       */
#define         OTF_GRID_MAX                2147483647
#define         ABE_NULL_TIME               -64000000.0  /*!<  Null time value (no time available for record)  */


  /*  Command codes for ABE programs.  */

#define         STUPID_WINDOZE_ID           6666 /*!<  Used in Windows if there is no user ID available.  The ID
                                                       of the beast ;-)                                                */
#define         CHARTSPIC_SAVED_PICTURE     6969 /*!<  The chartsPic program sets this flag when it has saved a 
                                                       JPG for pfmEdit                                                 */
#define         PFMEDIT_FORCE_SHARE         6970 /*!<  Returned from tofWaterKill when changes have been made          */
#define         WAVEMONITOR_FORCE_REDRAW    6971 /*!<  Force waveMonitor to reread and redraw its display              */
#define         WAIT_FOR_START              6972 /*!<  chartsPic, mosaicView, and pfm3D set this when they have        */
                                                 /*!<  finished initializing                                           */
#define         MOSAICVIEW_FORCE_RELOAD     6973 /*!<  Force mosaicView to reload and redraw its display (changed
                                                       mosaic files)                                                   */
#define         FEATURE_FILE_MODIFIED       6974 /*!<  Let ancillary and related programs know that we have changed
                                                       feature files                                                   */
#define         CHILD_PROCESS_FORCE_EXIT    6975 /*!<  Force all children to exit (this must be checked without
                                                       attempting to lock shared memory)                               */
#define         PFM_LAYERS_CHANGED          6978 /*!<  Set whenever pfmView or pfmEdit change PFM layers, display of
                                                       PFM layers, or order of PFM layers (for multiple PFM display
                                                       in pfmView or pfmEdit)                                          */
#define         PFM3D_FORCE_RELOAD          6979 /*!<  Force pfm3D to reload and redraw its display (changed display   */
                                                 /*!<  in pfmView).                                                    */
#define         ANCILLARY_FORCE_EXIT        6980 /*!<  Force clean exit in pfmEdit ancillary programs                  */
#define         PFMVIEW_FORCE_EDIT          6981 /*!<  Force pfmView to edit the area defined in pfm3D                 */
#define         PFMVIEW_FORCE_EDIT_3D       6982 /*!<  Force pfmView to edit the area defined in pfm3D                 */
#define         PFMEDIT3D_OPENED            6983 /*!<  Set when pfmEdit3D starts.  Used to clear pfm3D memory.         */
#define         PFMEDIT3D_CLOSED            6984 /*!<  Set when pfmEdit3D exits.  Used to reload pfm3D memory.         */
#define         PFMEDIT_KILL                6985 /*!<  Set to gracefully kill pfmEdit(3D).                             */
#define         CZMILREPROCESS_FATAL_ERROR  6986 /*!<  Set when czmilReprocess has encountered a fatal error.          */
#define         CZMILREPROCESS_ERROR        6987 /*!<  Set when czmilReprocess has encountered any error other than a
                                                       fatal type error.                                               */
#define         NO_ACTION_REQUIRED          9999 /*!<  No action required, duh                                         */



  /*! 
     This structure contains all of the information needed to process the multiwaveform data being shared by pfmEdit(3D),
     waveformMonitor, waveWaterfall, and CZMILwaveMonitor.  It is also used by fileEdit3D, chartsSwap, chartsPic, and others
     so be careful if you change things here.  The arrays (with MAX_STACK_POINTS items) contain data that is associated
     with the nearest point to the cursor in pfmEdit(3D) (or fileEdit3D).  The [0] point is the closest to the cursor of
     the MAX_STACK_POINTS points.
  */


  typedef struct 
  {
    int32_t       multiNum;                            /*!<  currently not used                                        */
    NV_F64_COORD3 multiPoint[MAX_STACK_POINTS];        /*!<  3D coordinate                                             */
    int32_t       multiFile[MAX_STACK_POINTS];         /*!<  file descriptor                                           */
    uint32_t      multiRecord[MAX_STACK_POINTS];       /*!<  record descriptor                                         */
    int32_t       multiSubrecord[MAX_STACK_POINTS];    /*!<  subrecord descriptor                                      */
    int32_t       multiPfm[MAX_STACK_POINTS];          /*!<  PFM descriptor                                            */
    int32_t       multiLine[MAX_STACK_POINTS];         /*!<  line descriptor                                           */
    int32_t       multiFlightlineIndex[MAX_STACK_POINTS];/*!<  index number of the flightline so a proper color can be
                                                               used for waveforms in color by flightline mode          */
    int32_t       multiPresent[MAX_STACK_POINTS];      /*!<  is waveform present?                                      */
    int32_t       multiLocation[MAX_STACK_POINTS];     /*!<  location within swath                                     */
    int16_t       multiType[MAX_STACK_POINTS];         /*!<  data type of point                                        */
    NV_C_RGBA     multiColors[MAX_STACK_POINTS];       /*!<  color struct for multi-waveforms                          */
    uint32_t      multiNearRecord;                     /*!<  currently not used                                        */
    int32_t       multiMode;                           /*!<  multiple waveform mode (0 - nearest neighbor, 1 - single) */
    uint8_t       multiSwitch;                         /*!<  have we changed multi-modes?                              */
    uint8_t       waveMonitorRunning;                  /*!<  Set if waveDisplay program is running                     */
  } MULTIWAVE_SHARE;



  typedef struct
  {
    double        search_radius;                       /*!<  Search radius in meters                                   */
    int32_t       search_width;                        /*!<  Number of points to search before and after selected 
                                                             point on nearby waveforms                                 */
    int32_t       rise_threshold;                      /*!<  Number of rise points needed as corroborating data on 
                                                             nearby waveforms                                          */
    int32_t       pmt_ac_zero_offset_required;         /*!<  If selected point is less than this amount above the
                                                             PMT AC zero offset the point will be marked invalid       */
    int32_t       apd_ac_zero_offset_required;         /*!<  If selected point is less than this amount above the
                                                             APD AC zero offset the point will be marked invalid       */
  } FILTER_SHARE;


  /*  This is the On-The-Fly (OTF) grid record structure used in pfmView and pfm3D.  */

  typedef struct
  {
    uint32_t      cnt;                       /*!<  Number of VALID soundings in the bin (set to greater than
                                                   OTF_GRID_MAX to force a recompute while loading).                   */
    float         min;                       /*!<  Minimum Z.                                                          */
    float         max;                       /*!<  Maximum Z.                                                          */
    float         avg;                       /*!<  Average Z (used as sum while loading).                              */
    float         std;                       /*!<  Standard deviation (used as sum of squares while loading).          */
  } OTF_GRID_RECORD;


  /* The POINT_CLOUD structure defines the values stored in the point cloud in pfmEdit or pfmEdit3D.  The actual     */
  /* data is stored in a separate shared memory structure that is named parent process ID (ppid in the ABE_SHARE     */
  /* structure) combined with "_abe_pfmEdit" (e.g.35467_abe_pfmEdit).                                                */

  typedef struct
  {
    double        x;                         /*!<  X position                                                            */
    double        y;                         /*!<  Y position                                                            */
    float         z;                         /*!<  Z value                                                               */
    float         herr;                      /*!<  Horizontal error                                                      */
    float         verr;                      /*!<  Vertical error                                                        */
    uint32_t      val;                       /*!<  Validity                                                              */
    uint32_t      oval;                      /*!<  Original validity (this is used in the editor to monitor changes)     */
    int16_t       pfm;                       /*!<  PFM number                                                            */
    int16_t       file;                      /*!<  File number                                                           */
    int32_t       line;                      /*!<  Line number (uses 32 bits because we're kludging it with the PFM 
                                                   number to make it unique)                                             */
    uint8_t       line_index;                /*!<  Index of line number as loaded in pfmEdit3D - the first line loaded
                                                   will be 0, the second will be 1, etc.  This is used for unique line
                                                   coloring in pfmEdit3D.                                                */
    uint32_t      rec;                       /*!<  Record (e.g. ping) number                                             */
    int32_t       sub;                       /*!<  Subrecord (e.g. beam) number                                          */
    int16_t       type;                      /*!<  Data type                                                             */
    int64_t       addr;                      /*!<  Depth record block address                                            */
    uint8_t       pos;                       /*!<  Depth record address position                                         */
    int32_t       xcoord;                    /*!<  X coordinate of the bin cell                                          */
    int32_t       ycoord;                    /*!<  Y coordinate of the bin cell                                          */
    uint8_t       exflag;                    /*!<  Externally flagged point                                              */
    uint8_t       mask;                      /*!<  Whether we are hiding this point or not                               */
    uint8_t       fmask;                     /*!<  Whether this point is filter masked or not                            */
    float         attr[NUM_ATTR];            /*!<  Optional attributes                                                   */
    uint8_t       lidar_class_change;        /*!<  Boolean flag to indicate that the LiDAR classification attribute has
                                                   been modified.                                                        */
  } POINT_CLOUD;




  /***********************************************************************************************************************/
  /*  This structure (ABE_SHARE) is shared between ABE programs for inter-process communication (IPC).                   */
  /***********************************************************************************************************************/

  typedef struct
  {
    uint8_t       settings_changed;           /*!<  Set if pfmEdit has changed any of the shared settings                */
    uint8_t       zoom_requested;             /*!<  Set if pfmEdit has requested a mosaicView zoom (so pfmView can 
                                                    inform any linked apps of the zoom request).                         */
    int32_t       position_form;              /*!<  Position format number (position formats are described in MISC)      */
    int32_t       smoothing_factor;           /*!<  Contour smoothing factor (0-10)                                      */
    float         z_factor;                   /*!<  Z scaling factor                                                     */
    float         z_offset;                   /*!<  Z offset value                                                       */
    char          snippet_file_name[2048];    /*!<  Snippet file name that chartsPic will use to store a feature image
                                                    from HOF or TOF files (jpg from the .img file, see 
                                                    CHARTSPIC_SAVED_PICTURE above).  Also used to return to pfmEdit3D the
                                                    error string from a fatal error encountered in czmilReprocess.       */
    float         heading;                    /*!<  Heading from chartsPic for use in displaying the snippet.            */
    int32_t       active_window_id;           /*!<  Process ID of the active window (set by the mouseMove function)      */
    NV_F64_COORD3 cursor_position;            /*!<  Position of the cursor (and possibly depth) in the application       */
    char          mosaic_hotkey[10];          /*!<  Hotkey to start the mosaic viewer program (can be modified in 
                                                    pfmEdit)                                                             */
    char          mosaic_prog[100];           /*!<  Mosaic viewer program name                                           */
    char          mosaic_actkey[10];          /*!<  Action keys for the mosaic viewer program                            */
    int32_t       polygon_count;              /*!<  Number of points in polygon                                          */
    double        polygon_x[2000];            /*!<  Polygon X positions                                                  */
    double        polygon_y[2000];            /*!<  Polygon Y positions                                                  */
    char          feature_search_string[128]; /*!<  Feature search string from pfmView                                   */
    int32_t       feature_search_type;        /*!<  0 = highlight result of search, 1 = only display result of search    */
    uint8_t       feature_search_invert;      /*!<  if set, search for features NOT containing search string             */


    /*  The following block of variables pertains to the color scale boxes on the left of pfmView, pfmEdit, and
        pfmEdit3D.  This is only applicable to color-by-depth mode in any of the three programs.  */

    uint16_t      min_hsv_color;              /*!<  HSV color wheel index for color to be used for minimum values
                                                    (default 0)  */
    uint16_t      max_hsv_color;              /*!<  HSV color wheel index for color to be used for maximum values
                                                    (default 315)  */
    float         min_hsv_value;              /*!<  Locked minimum data value for minimum HSV color  */
    float         max_hsv_value;              /*!<  Locked maximum data value for maximum HSV color  */
    uint8_t       min_hsv_locked;             /*!<  Set if min HSV is locked to a value  */
    uint8_t       max_hsv_locked;             /*!<  Set if max HSV is locked to a value  */


    /*  This part was lifted almost verbatim from pfm.h.  I've stopped using pfm.h because it is almost impossible
	to change the shared part without impacting everybody else who is using it.  */

    NV_F64_XYMBR  edit_area;                  /*!<  Total rectangular area to be edited                                  */
    int32_t       ppid;                       /*!<  Parent process ID (pfmView)                                          */
    float         cint;                       /*!<  Current contour interval (0 for user defined)                        */
    float         contour_levels[MAX_CONTOUR_LEVELS]; /*!<  User defined contour levels                                  */
    int32_t       num_levels;                 /*!<  Number of user defined contour levels                                */
    int32_t       layer_type;                 /*!<  Type of bin data/contour to display (AVERAGE_FILTERED_DEPTH,
                                                    MIN_FILTERED_DEPTH, MAX_DEPTH, etc. from pfm.h                       */
    NV_F64_XYMBR  displayed_area;             /*!<  Displayed area in the ndx editor                                     */
    NV_F64_XYMBR  viewer_displayed_area;      /*!<  Displayed area in the bin viewer                                     */
    PFM_OPEN_ARGS open_args[MAX_ABE_PFMS];    /*!<  Opening arguments for open_pfm_file                                  */
    uint8_t       display_pfm[MAX_ABE_PFMS];  /*!<  Whether the PFM is visible (if not, don't use it).                   */
    int32_t       pfm_count;                  /*!<  Number of currently open PFM files (layers).                         */
    int32_t       point_cloud_count;          /*!<  Total number of points in use in the point cloud.                    */


    /*  The following section is used for On-The-Fly (OTF) gridding in pfmView and pfm3D.  */

    int32_t       otf_width;                  /*!<  Width of otf grid (if this is set to 0 we're not using OTF).         */
    int32_t       otf_height;                 /*!<  Height of otf grid.                                                  */
    float         otf_min_z;                  /*!<  Minimum Z value of the otf grid.                                     */
    float         otf_max_z;                  /*!<  Maximum Z value of the otf grid.                                     */
    float         otf_null_value;             /*!<  Null value for otf grid.                                             */
    double        otf_x_bin_size;             /*!<  X otf bin size in degrees.                                           */
    double        otf_y_bin_size;             /*!<  Y otf bin size in degrees.                                           */


    /*  This is the delete file queue that will be populated by pfmEdit or pfmEdit3D (right click menu in DELETE_POINT
        mode) and acted on in pfmView.  The 2D array contains the PFM number in [0] and the file number in [1].        */

    int32_t       delete_file_queue[MAX_DELETE_FILE_QUEUE][2];
    int32_t       delete_file_queue_count;


    MULTIWAVE_SHARE  mwShare;                 /*!<  CHL: structure for multi-waveform data with waveMonitor              */


    FILTER_SHARE  filterShare;                /*!<  Values to be used in external filter programs                        */


    /*!  Flag to tell pfmView, pfmEdit, and pfmEdit3D that they are not allowed to edit the PFMs they are viewing.       */

    uint8_t       read_only;


    /*!  Only one of these lest the overhead kill us.                                                                   */

    char          nearest_filename[512];


    /*!  Nearest point to the cursor in pfmEdit.  This is the trigger for waveMonitor and waveWaterfall.                */

    int32_t       nearest_point;



    uint32_t      key;                        /*!<  Key or command code.                                                 */


    uint32_t      killed;                     /*!<  Kill switch number for a process that is killed in the process.      
                                                    This is used to check on the death of processes that are started
                                                    from buttons (that need their state set).  These type of programs
                                                    are monitor type programs like chartsMonitor or chartsPic.  These
                                                    programs have their "state" set to non-zero in pfmEdit and/or
                                                    pfmEdit3D.  That is, they are respawnable.                            */


    /*!  Modified record information from ancillary programs.  This info will be read by pfmEdit and pfmEdit3D to
         effect the changes to the PFM.                                                                                  */

    char          modified_file[512];
    char          modified_line[512];
    int32_t       modified_file_num;
    int32_t       modified_line_num;
    int16_t       modified_type;
    uint32_t      modified_record;
    int32_t       modified_subrecord;
    int32_t       modified_point;
    double        modified_value;
    uint32_t      modified_validity;
    uint32_t      modcode;                    /*!<  Set to something other than zero to indicate a change in the shared
                                                    data                                                                 */
  } ABE_SHARE;


#ifdef  __cplusplus
}
#endif

#endif
