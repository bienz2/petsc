/* $Id: aij.h,v 1.15 1995/10/17 21:41:57 bsmith Exp bsmith $ */

#include "matimpl.h"
#include <math.h>

#if !defined(__AIJ_H)
#define __AIJ_H

/*  
  MATSEQAIJ format - Compressed row storage (also called Yale sparse matrix
  format), compatible with Fortran.  The i[] and j[] arrays start at 1,
  not or 0, depending on the value of shift.  For example, in Fortran 
  j[i[k]+p+shift] is the pth column in row k.
*/

typedef struct {
  int    sorted;           /* if true, rows are sorted by increasing columns */
  int    roworiented;      /* if true, row-oriented storage */
  int    nonew;            /* if true, don't allow new elements to be added */
  int    singlemalloc;     /* if true a, i, and j have been obtained with
                               one big malloc */
  int    assembled;        /* if true, matrix is fully assembled */
  int    m, n;             /* rows, columns */
  int    nz, maxnz;        /* nonzeros, allocated nonzeros */
  int    *diag;            /* pointers to diagonal elements */
  int    *i;               /* pointer to beginning of each row */
  int    *imax;            /* maximum space allocated for each row */
  int    *ilen;            /* actual length of each row */
  int    *j;               /* column values: j + i[k] - 1 is start of row k */
  Scalar *a;               /* nonzero elements */
  IS     row, col;         /* index sets, used for reorderings */
  Scalar *solve_work;      /* work space used in MatSolve */
  void   *spptr;           /* pointer for special library like SuperLU */
  int    indexshift;       /* zero or -one for C or Fortran indexing */
} Mat_SeqAIJ;

extern int MatILUFactorSymbolic_SeqAIJ(Mat,IS,IS,double,int,Mat *);
extern int MatConvert_SeqAIJ(Mat,MatType,Mat *);
extern int MatCopyPrivate_SeqAIJ(Mat, Mat*,int);
extern int MatMarkDiag_SeqAIJ(Mat);


#endif
