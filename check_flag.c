
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

#include "check_flag.h"


/***************************************************************************/
/*!

  - Function        check_flag - Checks the specified NV_GSF flag in a GSF beam
                    flag field.

  - Synopsis        check_flag (field, flag);
                        - uint8_t field         beam_flag field
                        - uint8_t flag          NV_GSF flag to test for

  - Returns
                    - NVFalse if the flag is not set in the field
		    - NVTrue if the flag is set in the field

  - Author          Jan C. Depner

***************************************************************************/ 
 
uint8_t check_flag (uint8_t field, uint8_t flag)
{
  uint8_t        ret_value;


  ret_value = NVFalse;

  if (flag == NV_GSF_IGNORE_NULL_BEAM)
    {
      if (!(field ^ flag)) ret_value = NVTrue;
    }
  else if (flag == NV_GSF_INFO_NOT_1X_IHO)
    {
      if (field & flag) ret_value = NVTrue;
    }
  else
    {
      if ((field & flag) == flag) ret_value = NVTrue;
    }

  return (ret_value);
}



/***************************************************************************/
/*!

  - Function        set_flag - Sets the specified NV_GSF flag in a GSF beam
                    flag field.

  - Synopsis        set_flag (&field, flag);
                        - uint8_t field         address of beam_flag field
                        - uint8_t flag          NV_GSF flag to set

  - Returns         void

  - Author          Jan C. Depner

***************************************************************************/ 

void set_flag (uint8_t *field, uint8_t flag)
{
  int32_t                category;


  category = (flag & 0x03) + 1;

  switch (category)
    {
    case (NV_GSF_INFORMATIONAL):
      *field |= flag;

      /*  Clear the selected bit or it will get confused with NV_GSF_SELECTED_AVERAGE.  */

      /*  Blew this off because we're not using selected average anyway.   JCD  07/14/00  */

      /**field |= ~NV_GSF_SELECTED;*/
      break;

    case (NV_GSF_IGNORE):

      /*  If this sounding has been selected, clear the selection flags, and then set the ignore flags.  */

      if (*field & NV_GSF_SELECTED) *field = 0;

      *field |= flag;
      break;

    case (NV_GSF_SELECT):

      /*  The beam must not be set ignore, or have the informational bit set to be selected.  */

      if (((*field & 0x03) != 1) && (*field != NV_GSF_INFO_NOT_1X_IHO))
        {
          *field |= flag;
        }
      break;
    }
}



/***************************************************************************/
/*!

  - Function        clear_flag - Clears the specified NV_GSF flag in a GSF beam
                    flag field.

  - Synopsis        set_flag (&field, flag);
                        - uint8_t field         address of beam_flag field
                        - uint8_t flag          NV_GSF flag to clear

  - Returns         void

  - Author          Jan C. Depner

***************************************************************************/ 

void clear_flag (uint8_t *field, uint8_t flag)
{
  int32_t        category;
  uint8_t        cMask, reasons, clear_bit;


  category = (flag & 0x03) + 1;


  /*  Switch on the category for this flag.  */

  switch (category)
    {
    case (NV_GSF_INFORMATIONAL):
      *field &= ~flag;
      break;

    case (NV_GSF_IGNORE):

      /*  Save the category mask, and check to see if more than one of the ignore bits is set, 
          if so don't clear the category mask.  */

      cMask = *field & 0x01;
      reasons = *field & NV_GSF_IGNORE_BIT_MASK;
      clear_bit = flag & ~(0x01);
      reasons &= ~clear_bit;
      if (reasons)
        {
          *field &= ~flag;
          *field |= cMask;
        }
      else
        {
          *field &= ~flag;
        }
      break;

    case (NV_GSF_SELECT):

      /*  Save the category mask, and check to see if more than one of the select bits is set, 
          if so don't clear the category mask.  */

      cMask = *field & 0x02;
      reasons = *field & 0xfc;
      clear_bit = flag & ~(0x02);
      reasons &= ~clear_bit;
      if (reasons)
        {
          *field &= ~flag;
          *field |= cMask;
        }
      else
        {
          *field &= ~flag;
        }
      break;
    }
}
