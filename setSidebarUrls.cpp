
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



#include "setSidebarUrls.hpp"


/*!  This method clears the sidebarUrls list and adds just the current working directory and the last used
     directory for QFileDialogs.  This makes things quite a bit less confusing.  I used to just add to the 
     list but that made too many directories appear in the sidebar.  We still sort the (two entry) list
     and remove duplicates (because the current may be the same as the previous).  */

void setSidebarUrls (QFileDialog *fd, QString dir)
{
  /*  Clear out the "filedialog" settings for Qt just in case we have a missing file and/or directory that
      will cause the QFileDialog to hang up.  */

  QSettings settings (QSettings::UserScope, "Trolltech");
  settings.beginGroup ("Qt");
  settings.remove ("filedialog"); 


  QList<QUrl> urls;
  urls = fd->sidebarUrls ();


  //  Clear the sidebar URLs

  urls.clear ();


  //  Add the previous and the current working directory

  urls += QUrl::fromLocalFile (QDir (dir).absolutePath ());
  urls += QUrl::fromLocalFile (QDir (".").absolutePath ());


  //  QSet will get rid of duplicates.

  QSet<QUrl> set = urls.toSet ();
  urls = set.toList ();


  //  Sort them.

  qSort (urls);


  //  Get rid of nonexistant URLs.

  uint8_t *kill_list = new uint8_t[urls.size ()];

  for (int32_t i = 0 ; i < urls.size () ; i++)
    {
      kill_list[i] = NVFalse;

      QString path = urls.at (i).path ();

      if (!path.isEmpty () && urls.at (i).toString ().contains ("file:"))
        {

	  //  I can't, for the life of me, figure out how to get the file in the right format
	  //  on Windows.  toNativeSeparators doesn't seem to work.  This kludge does ;-)

#ifdef NVWIN3X
	  path.remove (0, 1);
	  path.replace ("/", "\\");
#endif

          QFileInfo qf = QFileInfo (path);

          if (!qf.exists () || !qf.isDir ()) kill_list[i] = NVTrue;
        }
    }


  int32_t loop = urls.size ();
  for (int32_t i = loop ; i >= 0 ; i--)
    {
      if (kill_list[i]) urls.removeAt (i);
    }

  delete[] kill_list;


  //  Set the sidebarUrls.

  fd->setSidebarUrls (urls);


  //  Set the directory to the last used directory.

  fd->setDirectory (QDir (dir).absolutePath ());
}
