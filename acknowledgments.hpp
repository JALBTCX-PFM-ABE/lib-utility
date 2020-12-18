
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
    document the API.  Dashes in these comment blocks are used to create bullet lists.  The
    lack of blank lines after a block of dash preceeded comments means that the next block
    of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/


QString acknowledgmentsText = 
  QObject::tr ("<center><br>This program was built using some, if not all, of the following Open Source or public domain resources:"
               "<br><br></center>"
               "<ul>"
               "<li><a href=http://beecrypt.sourceforge.net>BeeCrypt</a> - Cryptography library</li>"
               "<li><a href=http://www.gdal.org>GDAL</a> - Geospatial Data Abstraction Library</li>"
               "<li><a href=https://www.leidos.com/maritime/gsf>GSF</a> - Generic Sensor Format Library</li>"
               "<li><a href=http://www.hdfgroup.org/HDF5>HDF5</a> - Heirarchical Data Format library</li>"
               "<li><a href=http://www.ijg.org>libjpeg</a> - JPEG library</li>"
               "<li><a href=http://rapidlasso.com>LASlib</a> - LAS I/O library</li>"
               "<li><a href=http://www.opennavsurf.org>OpenNS</a> - Open Navigation Surface API</li>"
               "<li><a href=http://trac.osgeo.org/proj>PROJ.4</a> - Cartographic Projections Library</li>"
               "<li><a href=http://qt-project.org>Qt</a> - A cross-platform application and UI framework</li>"
               "<li><a href=http://shapelib.maptools.org>SHAPELIB</a> - Shapefile C Library</li>"
               "<li><a href=https://sqlite.org>SQLite</a> - SQL database engine API</li>"
               "<li><a href=http://xmlsoft.org>XML2</a> - XML C parser</li>"
               "<li><a href=http://www.zlib.net>ZLIB</a> - Compression Library</li>"
               "<li><a href=http://tango.freedesktop.org/Tango_Icon_Library>Tango Icon Library</a> - Icon set for the Tango Desktop Project</li>"
               "<li><a href=http://www.smashingmagazine.com/2009/05/20/flavour-extended-the-ultimate-icon-set-for-web-designers>Flavour Extended</a> - "
               "Icon set by Oliver Twardowski</li>"
               "<li><a href=http://ddrdark.deviantart.com/art/WIP-32px-mania-iconset-106090029>32pxmania</a> - Icon set by DDrDark</li>"
               "</ul><br>"
               "<ul>"
               "<li>BeeCrypt, GSF, LASlib, Qt, and SHAPELIB are licensed under the <a href=http://www.gnu.org/copyleft/lesser.html>GNU LGPL</a></li>"
               "<li>GDAL is licensed under an X/MIT style open source license</li>"
               "<li>libjpeg, OpenNS, ZLIB, and HDF5 are licensed under their own open source licenses</li>"
               "<li>PROJ.4 and XML2 are licensed under an MIT open source license</li>"
               "<li>SQLite and the Tango Icon Set are released in the public domain</li>"
               "<li>Flavour Extended is free for any private or commercial use without any restrictions</li>"
               "<li>32pxmania is free for personal and commercial use with the proviso that the icon set can not be sold as a standalone icon set</li>"
               "</ul><br><br>"
               "Many thanks to the authors of these and all of their supporting projects.  For more information on "
               "each project please visit their websites using the links above.<br>"
               "<center>Jan C. Depner<br><br></center>",
               "This file is included in almost all ABE GUI programs (e.g. pfmView, pfmEdit3D, areaCheck) so it may have already been translated.  If there is "
               "a .ts file in one of the ABE GUI programs for the language that you are translating to, a translation for this text may already be done.  Please "
               "run linguist on one of those files to check.");
