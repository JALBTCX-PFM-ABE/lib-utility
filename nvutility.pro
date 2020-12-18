contains(QT_CONFIG, opengl): QT += opengl
QT += 
INCLUDEPATH += /c/PFM/compile/include
DEFINES += XML_LIBRARY WIN32 NVWIN3X _LARGEFILE64_SOURCE PFM_STATIC
QMAKE_CXXFLAGS_EXCEPTIONS_ON = -fexceptions
VERSION = 2.2.43 
CONFIG += staticlib exceptions
CONFIG += console
QMAKE_CXXFLAGS += -fno-strict-aliasing
######################################################################
# Automatically generated by qmake (2.01a) Fri Dec 11 16:03:36 2020
######################################################################

TEMPLATE = lib
TARGET = nvutility
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += ABE.h \
           ABE_register.hpp \
           acknowledgments.hpp \
           area.h \
           basename.h \
           big_endian.h \
           bit_pack.h \
           carter.h \
           carter_tables.h \
           changeFileRegisterABE.hpp \
           check_flag.h \
           check_target_schema.h \
           chrtr.h \
           clickLabel.hpp \
           cnstnts.h \
           convolve.h \
           cosang.hpp \
           cvtime.h \
           direct.h \
           dted.h \
           fget_coord.h \
           find_startup.h \
           find_startup_name.h \
           fixpos.h \
           geo_distance.h \
           get_area_mbr.h \
           get_egm08.h \
           get_geoid03.h \
           get_geoid09.h \
           get_geoid12a.h \
           get_geoid12b.h \
           get_string.h \
           getSystemInfo.hpp \
           globals.hpp \
           inside.h \
           inside_polygon.h \
           interp.hpp \
           interpolate.hpp \
           invgp.h \
           line_intersection.h \
           linterp.h \
           martin.h \
           msv.h \
           nav4word.h \
           navo_gsf_flags.h \
           newgp.h \
           ngets.h \
           normtime.h \
           nvdef.h \
           nvmap.hpp \
           nvMapGL.hpp \
           nvpic.hpp \
           nvutility.h \
           nvutility.hpp \
           nvutility_version.h \
           points.h \
           polygon_collision.h \
           polygon_intersection.h \
           qPosfix.hpp \
           read_coast.h \
           read_shape_mask.h \
           read_srtm1_topo.h \
           read_srtm2_topo.h \
           read_srtm30_topo.h \
           read_srtm3_topo.h \
           read_srtm_mask.h \
           read_srtm_topo.h \
           read_swbd_mask.h \
           registerABE.hpp \
           rproj.hpp \
           savgol.h \
           scaleBox.hpp \
           select.h \
           setSidebarUrls.hpp \
           sharedFile.h \
           smooth_contour.hpp \
           squat.hpp \
           sspfilt.h \
           sunshade.hpp \
           survey.hpp \
           swap_bytes.h \
           unregisterABE.hpp \
           vec.h \
           windows_getuid.h
SOURCES += area_list.c \
           b_spline.cpp \
           basename.c \
           big_endian.c \
           bit_pack.c \
           bitmap.cpp \
           carter.c \
           changeFileRegisterABE.cpp \
           check_flag.c \
           check_target_schema.c \
           chrtr.c \
           clickLabel.cpp \
           contour.cpp \
           convolve.c \
           cosang.cpp \
           cvtime.c \
           direct.c \
           dted.c \
           fget_coord.c \
           find_startup.c \
           find_startup_name.c \
           fixpos.c \
           follow.cpp \
           geo_distance.c \
           get_area_mbr.c \
           get_coords.c \
           get_egm08.c \
           get_geoid03.c \
           get_geoid09.c \
           get_geoid12a.c \
           get_geoid12b.c \
           get_points.c \
           get_string.c \
           get_survey.c \
           getexit.cpp \
           getSystemInfo.cpp \
           inside.c \
           inside_polygon.c \
           interpolate.cpp \
           invgp.c \
           line_intersection.c \
           linterp.c \
           load_verts.c \
           martin.c \
           msv.c \
           nav4word.c \
           newgp.c \
           ngets.c \
           norm_posix.c \
           normtime.c \
           nvmap.cpp \
           nvMapGL.cpp \
           nvpic.cpp \
           polygon_collision.c \
           polygon_intersection.c \
           print_time.c \
           qPosfix.cpp \
           read_coast.c \
           read_shape_mask.c \
           read_srtm1_topo.c \
           read_srtm2_topo.c \
           read_srtm30_topo.c \
           read_srtm3_topo.c \
           read_srtm_mask.c \
           read_srtm_topo.c \
           read_swbd_mask.c \
           registerABE.cpp \
           rproj.cpp \
           savgol.c \
           scaleBox.cpp \
           setSidebarUrls.cpp \
           smooth_contour.cpp \
           spline.cpp \
           spline_cof.cpp \
           squat.cpp \
           sspfilt.c \
           strtcon.cpp \
           sunshade.cpp \
           survey.cpp \
           swap_bytes.c \
           unregisterABE.cpp \
           wdbplt.cpp \
           windows_getuid.c