
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



#ifndef _NVUTILITY_H_
#define _NVUTILITY_H_

#ifdef  __cplusplus
extern "C" {
#endif


  /*! \mainpage NVUTILITY library

       <br><br>\section disclaimer Disclaimer

       This is a work of the US Government. In accordance with 17 USC 105, copyright
       protection is not available for any work of the US Government.

       Neither the United States Government nor any employees of the United States
       Government, makes any warranty, express or implied, without even the implied
       warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes
       any liability or responsibility for the accuracy, completeness, or usefulness
       of any information, apparatus, product, or process disclosed, or represents
       that its use would not infringe privately-owned rights. Reference herein to
       any specific commercial products, process, or service by trade name, trademark,
       manufacturer, or otherwise, does not necessarily constitute or imply its
       endorsement, recommendation, or favoring by the United States Government. The
       views and opinions of authors expressed herein do not necessarily state or
       reflect those of the United States Government, and shall not be used for
       advertising or product endorsement purposes.


       <br><br>\section intro Introduction

       The nvutility library is a loose collection of functions that are used in many of the Pure File Magic (PFM)
       Area-Based Editor (ABE) and associated programs.  Many of these functions have been in use at the Naval
       Oceanographic Office for decades.


       <br><br>\section sec1 Searching the Doxygen Documentation

       The easiest way to find documentation for a particular C function is by looking for it in the
       <a href="globals.html"><b>Files>Globals>All</b></a> tab.  So, for instance, if you are looking for
       documentation on the <b><i>clip</i></b> function you would go to
       <a href="globals.html"><b>Files>Globals>All</b></a> and click on the <b><i>c</i></b> tab then look for
       <b><i>clip</i></b>.  When you find <b><i>clip</i></b>, just click on the .h file associated with it.  Note
       that C++ methods are not documented in exactly the same way as C functions.  The easiest way to find C++
       methods is to go to the <a href="functions.html"><b>Data Structures>Data Fields>All</b></a> tab.  If, for
       example, you are looking for <b><i>setExaggeration</i></b> you would then click on the <b><i>s</i></b> tab,
       find <b><i>setExaggeration</i></b>, and then click on the associated structure (in this case nvMapGL).
       The entire group of public methods for a particular class are documented via their class documentation
       not via their include files.  You can see documentation for them in the main
       <a href="annotated.html"><b>Data Structures</b></a> tab or by selecting the class in the Data Structures
       section of the .hpp file.  For example, nvMap and nvMapGL can be accessed via the overall
       <a href="annotated.html"><b>Data Structures</b></a> tab or from the class defined in the Data Structures
       section of the nvmap.hpp or nvMapGL.hpp files.<br><br>



       <br><br>\section sec2 The pfm_nvtypes.h file

       Since almost everything in the Area-Based Editor (ABE) depends on the Pure File Magic (PFM) library, this
       library depends on certain data types that are defined in the pfm_nvtypes.h file.  The pfm_nvtypes.h file
       is not documented here.  If you have this library you must also have the PFM library and its documentation.
       You can check that documentation for information on the pfm_nvtypes.h file.  To put it simply, the
       pfm_nvtypes.h file contains, for the most part, a number of standard structures that we use throughout
       the ABE.  These structures are fairly easy to understand.  They have names like NV_F64_COORD3, NV_F64_XYMBR,
       or NV_F64_MBC.  These are a structure containing three, 64 bit floating point coordinates (e.g. x, y, and z),
       a structure containing a 64 bit floating point XY minimum bounding rectangle (MBR, .e.g. min_x, max_x, min_y,
       max_y), and a structure containing a 64 bit floating point geographic minimum bounding cube (MBC, e.g.
       slat, nlat, elon, wlon, min_z, and max_z) respectively.  Other structures will use the F32, I32, I64, or
       other data type indicators.  These are 32 bit floating point, 32 bit signed integer, and 64 bit signed
       integer respectively.  In any case, it should be fairly easy to guess what the structures are for without
       having to refer to the actual documentation of the pfm_nvtypes.h file.

  */

#include "pfm_nvtypes.h"
#include "nvdef.h"

#include "ABE.h"
#include "area.h"
#include "basename.h"
#include "big_endian.h"
#include "bit_pack.h"
#include "carter.h"
#include "check_flag.h"
#include "check_target_schema.h"
#include "chrtr.h"
#include "convolve.h"
#include "cnstnts.h"
#include "cvtime.h"
#include "direct.h"
#include "dted.h"
#include "fget_coord.h"
#include "find_startup.h"
#include "find_startup_name.h"
#include "fixpos.h"
#include "geo_distance.h"
#include "get_area_mbr.h"
#include "get_egm08.h"
#include "get_geoid03.h"
#include "get_geoid09.h"
#include "get_geoid12a.h"
#include "get_geoid12b.h"
#include "get_string.h"
#include "inside.h"
#include "inside_polygon.h"
#include "invgp.h"
#include "line_intersection.h"
#include "linterp.h"
#include "martin.h"
#include "msv.h"
#include "nav4word.h"
#include "newgp.h"
#include "ngets.h"
#include "normtime.h"
#include "nvutility_version.h"
#include "points.h"
#include "polygon_collision.h"
#include "polygon_intersection.h"
#include "read_coast.h"
#include "read_shape_mask.h"
#include "read_srtm1_topo.h"
#include "read_srtm2_topo.h"
#include "read_srtm3_topo.h"
#include "read_srtm30_topo.h"
#include "read_srtm_mask.h"
#include "read_srtm_topo.h"
#include "read_swbd_mask.h"
#include "savgol.h"
#include "select.h"
#include "sharedFile.h"
#include "sspfilt.h"
#include "swap_bytes.h"
#include "vec.h"
#include "windows_getuid.h"


#ifdef  __cplusplus
}
#endif

#endif
