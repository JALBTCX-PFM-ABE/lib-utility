
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



#include "unregisterABE.hpp"

/*  unregisterABE class.  */

/***************************************************************************/
/*!

   - Module :        unregisterABE

   - Programmer :    Jan C. Depner

   - Date :          04/09/08

   - Purpose :       Remove an ABE application from the ABE register shared
                     memory segment and then detach from the shared memory.

   - Arguments :
                     - abeRegister        -  pointer to QSharedMemory
                     - abe_register       -  pointer to ABE_REGISTER structure
                     - group              -  group that this app belongs to
                     - key                -  PID key of application

\***************************************************************************/

unregisterABE::unregisterABE (QSharedMemory *abeRegister, ABE_REGISTER *abe_register, int32_t group, int32_t key)
{
  //  Lock this while we're working with it.

  abeRegister->lock ();


  for (int32_t j = 0 ; j < MAX_ABE_MEMBERS ; j++)
    {
      //  We found the member, clear it and decrement the client count.

      if (abe_register->member[group][j].key == key)
	{
	  abe_register->member[group][j].key = 0;
	  strcpy (abe_register->member[group][j].name, "N/A");
	  strcpy (abe_register->member[group][j].file, "N/A");
	  abe_register->member_count[group]--;


	  //  If we have removed the last member, free up the group.

	  if (!abe_register->member_count[group]) abe_register->group[group] = NVFalse;

	  break;
	}
    }


  //  Unlock when we're done.

  abeRegister->unlock ();


  //  Detach it.

  abeRegister->detach ();
}



unregisterABE::~unregisterABE ()
{
}
