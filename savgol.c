
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


#include "savgol.h"


/*  Note: index 0 not used here.  */

typedef float MAT[SAVGOL_MMAX+2][SAVGOL_MMAX+2];


static int32_t IMin (int32_t ia, int32_t ib)
{
  if (ia <= ib) return ia;

  return ib;
}


/**************************************************************
* Given an N x N matrix A, this routine replaces it by the LU *
* decomposition of a rowwise permutation of itself. A and N   *
* are input. INDX is an output vector which records the row   *
* permutation effected by the partial pivoting; D is output   *
* as -1 or 1, depending on whether the number of row inter-   *
* changes was even or odd, respectively. This routine is used *
* in combination with LUBKSB to solve linear equations or to  *
* invert a matrix. Return code is 1, if matrix is singular.   *
**************************************************************/

static void ludcmp (MAT a, int32_t n, int32_t np __attribute__ ((unused)), int32_t *indx, int32_t *d, int32_t *code)
{
#define NMX  100

  float amax, dum, sum, tiny;
  float vv[NMX];
  int32_t    i, imax = 0, j, k;

  tiny = (float) 1e-12;

  *d = 1;
  *code = 0;

  for (i = 1 ; i <= n ; i++)
    {
      amax=0.0;
      for (j = 1 ; j <= n ; j++)
        {
          if (fabs (a[i][j]) > amax) amax = (float) fabs(a[i][j]);
        }

      if (amax < tiny)
        {
          *code = 1;
          return;
        }

      vv[i] = 1.0 / amax;
    }

  for (j=1 ; j <= n ; j++)
    {
     for (i = 1 ; i < j ; i++)
       {
         sum = a[i][j];
         for (k = 1 ; k < i ; k++) sum -= a[i][k] * a[k][j];
         a[i][j] = sum;
       }
     amax = 0.0;
     for (i = j ; i <= n ; i++)
       {
         sum = a[i][j];
         for (k = 1 ; k < j ; k++) sum -= a[i][k] * a[k][j];
         a[i][j] = sum;
         dum = vv[i] * (float) fabs (sum);
         if (dum >= amax)
           {
             imax = i;
             amax = dum;
           }
       }  

     if (j != imax)
       {
         for (k = 1 ; k <= n ; k++)
           {
             dum = a[imax][k];
             a[imax][k] = a[j][k];
             a[j][k] = dum;
           }
         *d = -(*d);
         vv[imax] = vv[j];
       }

     indx[j] = imax;
     if ((float) fabs (a[j][j]) < tiny)  a[j][j] = tiny;

     if (j != n)
       {
         dum = 1.0 / a[j][j];
         for (i = j + 1 ; i <= n ; i++) a[i][j] *= dum;
       } 
    }
}


/*****************************************************************
* Solves the set of N linear equations A . X = B.  Here A is    *
* input, not as the matrix A but rather as its LU decomposition, *
* determined by the routine LUDCMP. INDX is input as the permuta-*
* tion vector returned by LUDCMP. B is input as the right-hand   *
* side vector B, and returns with the solution vector X. A, N and*
* INDX are not modified by this routine and can be used for suc- *
* cessive calls with different right-hand sides. This routine is *
* also efficient for plain matrix inversion.                     *
*****************************************************************/

static void lubksb(MAT a, int32_t n, int32_t np __attribute__ ((unused)), int32_t *indx, float *b)
{
  float sum;
  int32_t i, ii, j, ll;

  ii = 0;

  for (i = 1 ; i <= n ; i++)
    {
      ll = indx[i];
      sum = b[ll];
      b[ll] = b[i];
      if (ii != 0)
        {
          for (j = ii ; j < i ; j++) sum -= a[i][j] * b[j];
        }
      else if (sum != 0.0)
        {
          ii = i;
        }
      b[i] = sum;
    }

  for (i = n ; i > 0 ; i--)
    {
      sum = b[i];
      if (i < n)
        {
          for (j = i + 1 ; j <= n; j++) sum -= a[i][j] * b[j];
        }
      b[i] = sum / a[i][i];
    }
}


/***************************************************************************/
/*!

   - Module :        savgol

   - Programmer :    Jan Depner, based on code by J-P Moreau

   - Date :          08/17/12

   - Purpose :       Returns in c(np), in wrap-around order (see reference)
                     consistent with the argument respns in routine convlv, a
                     set of Savitzky-Golay filter coefficients. nl is the number
                     of leftward (past) data points used, while nr is the number
                     of rightward (future) data points, making the total number
                     of data points used nl + nr + 1. ld is the order of the
                     derivative desired (e.g., ld = 0 for smoothed function).
                     m is the order of the smoothing polynomial, also equal to
                     the highest conserved moment; usual values are m = 2 or m = 4. 

   - Reference:      Numerical Recipes By W.H. Press, B. P. Flannery, S.A. Teukolsky
                     and W.T. Vetterling, Cambridge University Press, 1986 - 1992

****************************************************************************/

int32_t savgol (float *c, int32_t np, int32_t nl, int32_t nr, int32_t ld, int32_t m)
{
  int32_t d, icode, imj, ipj, i, j, k, kk, mm;
  int32_t indx[SAVGOL_MMAX+2];
  float fac, sum;
  MAT a;
  float b[SAVGOL_MMAX+2];

  if (np < nl + nr + 1 || nl < 0 || nr < 0 || ld > m || m > SAVGOL_MMAX || nl + nr < m)
    {
      fprintf (stderr, "\n Bad args in savgol.\n");
      return (-1);
    }

  for (i = 1 ; i <= SAVGOL_MMAX + 1 ; i++)
    {
      for (j = 1 ; j <= SAVGOL_MMAX + 1 ; j++) a[i][j] = 0.0;
      b[i] = 0.0;
      indx[i] = 0;
    }

  /*  Set up the normal equations of the desired leastsquares fit.  */

  for (ipj = 0 ; ipj <= 2 * m ; ipj++)
    {
      sum = 0.0;
      if (ipj == 0) sum = 1.0;

      for (k = 1 ; k <= nr ; k++) sum += (float) pow (k, ipj);

      for (k = 1 ; k <= nl ; k++) sum += (float) pow (-k, ipj);

      mm = IMin (ipj, 2 * m - ipj);
      imj = -mm;
      do
        {
          a[1 + (ipj + imj) / 2][1 + (ipj - imj) / 2] = sum;
          imj += 2;
        } while (imj <= mm); 
    }

  ludcmp (a, m + 1, SAVGOL_MMAX + 1, indx, &d, &icode);    /*  Solve them: LU decomposition  */
 
  for (j = 1 ; j <= m + 1 ; j++) b[j] = 0.0;

  b[ld + 1] = 1.0;    /*  Right-hand side vector is unit vector, depending on which derivative we want.  */

  lubksb (a, m + 1, SAVGOL_MMAX + 1, indx, b);      /*  Backsubstitute, giving one row of the inverse matrix.  */
  

  /*  Zero the output array (it may be bigger than the number of coefficients.  */

  for (kk = 1 ; kk <= np ; kk++) c[kk] = 0.0;


  /*  Each Savitzky-Golay coefficient is the dot product of powers of an integer with the inverse matrix row.  */

  for (k = -nl ; k <= nr ; k++)
    {
      sum = b[1];                       
      fac = 1.0;
      for (mm = 1 ; mm <= m ; mm++)
        {
          fac *= k;
          sum += b[mm + 1] * fac;
        } 

      /*  Store in wrap-around order  */

      kk = ((np - k) % np) + 1;
      c[kk] = sum;
    }

  return (0);
}
