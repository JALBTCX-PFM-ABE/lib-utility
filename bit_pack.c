
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



#include <math.h>
#include <stdio.h>

#include "pfm_nvtypes.h"
#include "nvdef.h"


static const uint8_t    mask[8] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 
                        0xfe}, notmask[8] = {0xff, 0x7f, 0x3f, 0x1f, 0x0f, 
                        0x07, 0x03, 0x01}; 


static const int32_t LogTable256[256] =
  {-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};


/*******************************************************************************************/
/*!

  - Function        int_log2 - Computes the base 2 log of an unsigned 32 bit integer.

  - Synopsis        int_log2 (v);
                    - uint32_t v     =      integer value whose base 2 log we will compute

  - Description     This is used to get the number of bits needed to store an unsigned 32 bit
                    integer value.  The alternative is to use log functions, like this: <br>
                    - bits = NINT (log10 ((double) v) / LOG2 + 0.5);
                    Precomputing LOG2 saves a bit of time but the single log10 function is
                    computationally expensive.  The lookup table method takes only about 7
                    operations to find the log of a 32-bit value.  Just call this function and
                    add one to the return value to get the number of bits needed, thusly: <br>
                    - bits = int_log2 (v) + 1;
                    This method was derived from Daniel Lemire's blog.

  - Returns
                    - log base 2 of the input integer

  - Author          Jan C. Depner

*********************************************************************************************/

int32_t int_log2 (uint32_t v)
{
  uint32_t       r, t, tt;


  if ((tt = v >> 16))
    {
      r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
    }
  else
    {
      r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
    }

  return (r);
}


 
/*******************************************************************************************/
/*!

  - Function        short_log2 - Computes the base 2 log of an unsigned 16 bit integer.

  - Synopsis        short_log2 (v);
                    - uint16_t v     =      integer value whose base 2 log we will compute

  - Description     This is used to get the number of bits needed to store an unsigned 16 bit
                    integer value.  The alternative is to use log functions, like this: <br>
                    - bits = NINT (log10 ((double) v) / LOG2 + 0.5);
                    Precomputing LOG2 saves a bit of time but the single log10 function is
                    computationally expensive.  The lookup table method takes only about 7
                    operations to find the log of a 32-bit value.  Just call this function and
                    add one to the return value to get the number of bits needed, thusly: <br>
                    - bits = short_log2 (v) + 1;
                    This method was derived from Daniel Lemire's blog.

  - Returns
                    - log base 2 of the input integer

  - Author          Jan C. Depner

*********************************************************************************************/

int32_t short_log2 (uint16_t v)
{
  uint32_t       r, t, v2;

  v2 = (uint32_t) v;

  r = (t = v2 >> 8) ? 8 + LogTable256[t] : LogTable256[v2];

  return (r);
}



/***************************************************************************/
/*!

  - Function        bit_pack - Packs a long value into consecutive bits in
                    buffer.

  - Synopsis        bit_pack (buffer, start, numbits, value);
                        - uint8_t buffer[]      address of buffer to use
                        - uint32_t start        start bit position in buffer
                        - uint32_t numbits      number of bits to store
                        - int32_t value          value to store

  - Description     Packs the value 'value' into 'numbits' bits in 'buffer'
                    starting at bit position 'start'.  The majority of
                    this code is based on Appendix C of Naval Ocean
                    Research and Development Activity Report #236, 'Data
                    Base Structure to Support the Production of the Digital
                    Bathymetric Data Base', Nov. 1989, James E. Braud,
                    John L. Breckenridge, James E. Current, Jerry L.
                    Landrum.

  - Returns         void

  - Author          Jan C. Depner

***************************************************************************/ 

void bit_pack (uint8_t buffer[], uint32_t start, uint32_t numbits, int32_t value)
{
  int32_t                 start_byte, end_byte, start_bit, end_bit, i;


  i = start + numbits;


  /*  Right shift the start and end by 3 bits, this is the same as        */
  /*  dividing by 8 but is faster.  This is computing the start and end   */
  /*  bytes for the field.                                                */

  start_byte = start >> 3;
  end_byte = i >> 3;


  /*  AND the start and end bit positions with 7, this is the same as     */
  /*  doing a mod with 8 but is faster.  Here we are computing the start  */
  /*  and end bits within the start and end bytes for the field.          */

  start_bit = start & 7;
  end_bit = i & 7;


  /*  Compute the number of bytes covered.                                */

  i = end_byte - start_byte - 1;


  /*  If the value is to be stored in one byte, store it.                 */

  if (start_byte == end_byte)
    {
      /*  Rather tricky.  We are masking out anything prior to the start  */
      /*  bit and after the end bit in order to not corrupt data that has */
      /*  already been stored there.                                      */

      buffer[start_byte] &= mask[start_bit] | notmask[end_bit];


      /*  Now we mask out anything in the value that is prior to the      */
      /*  start bit and after the end bit.  This is, of course, after we  */
      /*  have shifted the value left past the end bit.                   */

      buffer[start_byte] |= (value << (8 - end_bit)) & (notmask[start_bit] & mask[end_bit]);
    }


  /*  If the value covers more than 1 byte, store it.                     */

  else
    {
      /*  Here we mask out data prior to the start bit of the first byte. */

      buffer[start_byte] &= mask[start_bit];


      /*  Get the upper bits of the value and mask out anything prior to  */
      /*  the start bit.  As an example of what's happening here, if we   */
      /*  wanted to store a 14 bit field and the start bit for the first  */
      /*  byte is 3, we would be storing the upper 5 bits of the value in */
      /*  the first byte.                                                 */

      buffer[start_byte++] |= (value >> (numbits - (8 - start_bit))) & notmask[start_bit];


      /*  Loop while decrementing the byte counter.                       */

      while (i--)
        {
          /*  Clear the entire byte.                                      */

          buffer[start_byte] &= 0;


          /*  Get the next 8 bits from the value.                         */

          buffer[start_byte++] |= (value >> ((i << 3) + end_bit)) & 255;
       	}


      /*  If the end bit is an exact multiple of 8 we are done.  If we    */
      /*  try to mess around with the next byte it will cause a memory    */
      /*  overflow.                                                       */

      if (end_bit)
        {
          /*  For the last byte we mask out anything after the end bit.   */

          buffer[start_byte] &= notmask[end_bit];


          /*  Get the last part of the value and stuff it in the end      */
          /*  byte.  The left shift effectively erases anything above     */
          /*  8 - end_bit bits in the value so that it will fit in the    */
          /*  last byte.                                                  */

          buffer[start_byte] |= (value << (8 - end_bit));
        }
    }
} 
 
 
 
/***************************************************************************/
/*!

  - Function        bit_unpack - Unpacks a long value from consecutive bits
                    in buffer.

  - Synopsis        bit_unpack (buffer, start, numbits);
                        - uint8_t buffer[]      address of buffer to use
                        - uint32_t start        start bit position in buffer
                        - uint32_t numbits      number of bits to retrieve

  - Description     Unpacks the value from 'numbits' bits in 'buffer'
                   starting at bit position 'start'.  The value is assumed
                   to be unsigned.  The majority of this code is based on
                   Appendix C of Naval Ocean Research and Development
                   Activity Report #236, 'Data Base Structure to Support
                   the Production of the Digital Bathymetric Data Base',
                   Nov. 1989, James E. Braud, John L. Breckenridge, James
                   E. Current, Jerry L. Landrum.

  - Returns         uint32_t                value retrieved from buffer

  - Caveats         Note that the value that is output from this function
                    is an unsigned 32 bit integer.  Even though you may have
                    passed in a signed 32 bit value to bit_pack it will not
                    be sign extended on the way out.  If you just happenned
                    to store it in 32 bits you can just typecast it to a
                    signed 32 bit number and, lo and behold, you have a
                    nice, signed number.  Otherwise, you have to do the
                    sign extension yourself.

  - Author          Jan C. Depner

****************************************************************************/ 

uint32_t bit_unpack (uint8_t buffer[], uint32_t start, uint32_t numbits)
{
  int32_t                 start_byte, end_byte, start_bit, end_bit, i;
  uint32_t                value;


  i = start + numbits;

  /*  Right shift the start and end by 3 bits, this is the same as        */
  /*  dividing by 8 but is faster.  This is computing the start and end   */
  /*  bytes for the field.                                                */

  start_byte = start >> 3;
  end_byte = i >> 3;


  /*  AND the start and end bit positions with 7, this is the same as     */
  /*  doing a mod with 8 but is faster.  Here we are computing the start  */
  /*  and end bits within the start and end bytes for the field.          */

  start_bit = start & 7;
  end_bit = i & 7;


  /*  Compute the number of bytes covered.                                */

  i = end_byte - start_byte - 1;


  /*  If the value is stored in one byte, retrieve it.                    */

  if (start_byte == end_byte)
    {
      /*  Mask out anything prior to the start bit and after the end bit. */

      value = (uint32_t) buffer[start_byte] & (notmask[start_bit] & mask[end_bit]);


      /*  Now we shift the value to the right.                            */

      value >>= (8 - end_bit);
    }


  /*  If the value covers more than 1 byte, retrieve it.                  */

  else
    {
      /*  Here we mask out data prior to the start bit of the first byte  */
      /*  and shift to the left the necessary amount.                     */

      value = (uint32_t) (buffer[start_byte++] & notmask[start_bit]) << (numbits - (8 - start_bit));


      /*  Loop while decrementing the byte counter.                       */

      while (i--)
        {
          /*  Get the next 8 bits from the buffer.                        */

          value += (uint32_t) buffer[start_byte++] << ((i << 3) + end_bit);
       	}


      /*  If the end bit is an exact multiple of 8 we are done.  If we    */
      /*  try to mess around with the next byte it will cause a memory    */
      /*  overflow.                                                       */

      if (end_bit)
        {
          /*  For the last byte we mask out anything after the end bit    */
          /*  and then shift to the right (8 - end_bit) bits.             */

          value += (uint32_t) (buffer[start_byte] & mask[end_bit]) >> (8 - end_bit);
        }
    }

  return (value);
}



/***************************************************************************/
/*!

  - Function        double_bit_pack - Packs a long long integer value into
                    consecutive bits in buffer.

  - Synopsis        double_bit_pack (buffer, start, numbits, value);
                        - uint8_t buffer[]      address of buffer to use
                        - uint32_t start        start bit position in buffer
                        - uint32_t numbits      number of bits to store
                        - int64_t value          value to store

  - Description     Packs the value 'value' into 'numbits' bits in 'buffer'
                   starting at bit position 'start'.

  - Returns         void

  - Author          Jan C. Depner

****************************************************************************/ 

void double_bit_pack (uint8_t buffer[], uint32_t start, uint32_t numbits, int64_t value)
{
  int32_t             high_order, low_order;


  high_order = (int32_t) (((uint64_t) value) >> 32);
  low_order  = (int32_t) (value & UINT32_MAX);

  bit_pack (buffer, start, numbits - 32, high_order);
  bit_pack (buffer, start + (numbits - 32), 32, low_order);
}



/***************************************************************************/
/*!

  - Function        double_bit_unpack - Unpacks a long long integer value
                    from consecutive bits in buffer.

  - Synopsis        double_bit_unpack (buffer, start, numbits);
                        - uint8_t buffer[]      address of buffer to use
                        - uint32_t start        start bit position in buffer
                        - uint32_t numbits      number of bits to store

  - Description     Unpacks a value from 'numbits' bits in 'buffer'
                   starting at bit position 'start'.  It is then unscaled
                   by 'scale'.

  - Returns         uint64_t                Value unpacked from buffer

  - Author          Jan C. Depner

****************************************************************************/ 

uint64_t double_bit_unpack (uint8_t buffer[], uint32_t start, uint32_t numbits)
{
  uint64_t            result;
  uint32_t            high_order, low_order;


  high_order = bit_unpack (buffer, start, numbits - 32);
  low_order  = bit_unpack (buffer, start + (numbits - 32), 32);

  result = ((uint64_t) high_order) << 32;
  result |= (uint64_t) low_order;

  return (result);
}
