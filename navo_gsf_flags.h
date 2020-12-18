
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

#ifndef _NAVO_GSF_FLAGS_H_
#define _NAVO_GSF_FLAGs_H_

#ifdef  __cplusplus
extern "C" {
#endif


  /*  These are NAVO's versions of the old Hydrographic Multibeam Processing System (HMPS)
      beam flags for GSF data.  They were originally defined by NRL for HMPS and were later
      modified by NAVO and SAIC to support SAIC's ISS60 and SABER systems as well as NAVOs
      Bathy/Hydro Post-Processing (BHPP) system.  These definitions match the bit patterns
      in the HMPS flag set in order to maintain a consistent set of flags between SAIC and
      NAVO when testing/setting beam flags.  */


  /*  GSF carries eight bits for each beam flag.  There is one beam flag per beam (obviously).  */


  /*  Definitions for GSF beam flag categories (see check_flag.c for uses).  */

#define NV_GSF_INFORMATIONAL             0x01  /*!<  Informational category  */
#define NV_GSF_IGNORE                    0x02  /*!<  Ignore category  */
#define NV_GSF_SELECT                    0x03  /*!<  Select category  */


  /*  Definitions for NV_GSF_INFORMATIONAL category  */

#define NV_GSF_INFO_NOT_1X_IHO           0x10  /*!<  This beam does not meet IHO standard  */


  /*  Definitions for NV_GSF_IGNORE category  */

#define NV_GSF_IGNORE_NULL_BEAM          0x01  /*!<  This beam is to be ignored, no detection was made by the sonar  */
#define NV_GSF_IGNORE_MANUALLY_EDITED    0x05  /*!<  This beam is to be ignored, it has been manually edited  */
#define NV_GSF_IGNORE_FILTER_EDITED      0x09  /*!<  This beam is to be ignored, it has been filter edited  */
#define NV_GSF_IGNORE_NOT_2X_IHO         0x21  /*!<  This beam is to be ignored, since it exceeds two times IHO  */
#define NV_GSF_IGNORE_FOOTPRINT_TOO_BIG  0x41  /*!<  This beam is to be ignored, since it exceeds the maximum footprint  */
#define NV_GSF_IGNORE_TPE_EXCEEDS_IHO    0x81  /*!<  This beam is to be ignored, since the horizontal error or vertical error tolerance is exceeded  */
#define NV_GSF_IGNORE_BIT_MASK           0xec  /*!<  Mask of combination of reason bits  */


  /*  Definitions for the NV_GSF_SELECT category  */

#define NV_GSF_SELECTED                  0x02  /*!<  Selected sounding, no reason specified  */
#define NV_GSF_SELECTED_LEAST            0x06  /*!<  Selected sounding, it is a least depth  */
#define NV_GSF_SELECTED_MAXIMUM          0x0a  /*!<  Selected sounding, it is a maximum depth  */
#define NV_GSF_SELECTED_FEATURE          0x22  /*!<  Selected sounding, it has been identified as a multibeam/lidar feature  */
#define NV_GSF_SELECTED_SPARE_1          0x42  /*!<  Selected sounding, spare ....  */
#define NV_GSF_SELECTED_DESIGNATED       0x82  /*!<  Selected sounding, designated Isounding  */


  /*  Definition of GSF flag used to indicate that the GSF ping datum shifted instead of tide corrected.  */

#define NV_GSF_GPS_VERTICAL_CONTROL      GSF_PING_USER_FLAG_13


#ifdef  __cplusplus
}
#endif

#endif
