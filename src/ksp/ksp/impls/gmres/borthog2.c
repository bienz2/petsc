/*$Id: borthog2.c,v 1.20 2001/08/07 03:03:51 balay Exp $*/
/*
    Routines used for the orthogonalization of the Hessenberg matrix.

    Note that for the complex numbers version, the VecDot() and
    VecMDot() arguments within the code MUST remain in the order
    given for correct computation of inner products.
*/
#include "src/ksp/ksp/impls/gmres/gmresp.h"

EXTERN_C_BEGIN
/*
  This version uses classical UNMODIFIED Gram-Schmidt.  It has options for using
  iterative refinement to improve stability.

 */
#undef __FUNCT__  
#define __FUNCT__ "KSPGMRESClassicalGramSchmidtOrthogonalization"
int KSPGMRESClassicalGramSchmidtOrthogonalization(KSP  ksp,int it)
{
  KSP_GMRES   *gmres = (KSP_GMRES *)(ksp->data);
  int         j,ierr;
  PetscScalar *hh,*hes,shh[500],*lhh;
  PetscReal   hnrm, wnrm;
  PetscTruth  refine = (PetscTruth)(gmres->cgstype == KSP_GMRES_CGS_REFINE_ALWAYS);

  PetscFunctionBegin;
  ierr = PetscLogEventBegin(KSP_GMRESOrthogonalization,ksp,0,0,0);CHKERRQ(ierr);
  /* Don't allocate small arrays */
  if (it < 501) lhh = shh;
  else {
    ierr = PetscMalloc((it+1) * sizeof(PetscScalar),&lhh);CHKERRQ(ierr);
  }
  
  /* update Hessenberg matrix and do unmodified Gram-Schmidt */
  hh  = HH(0,it);
  hes = HES(0,it);

  /* Clear hh and hes since we will accumulate values into them */
  for (j=0; j<=it; j++) {
    hh[j]  = 0.0;
    hes[j] = 0.0;
  }

  /* 
     This is really a matrix-vector product, with the matrix stored
     as pointer to rows 
  */
  ierr = VecMDot(it+1,VEC_VV(it+1),&(VEC_VV(0)),lhh);CHKERRQ(ierr); /* <v,vnew> */
  for (j=0; j<=it; j++) {
    lhh[j] = - lhh[j];
  }

  /*
         This is really a matrix vector product: 
         [h[0],h[1],...]*[ v[0]; v[1]; ...] subtracted from v[it+1].
  */
  ierr = VecMAXPY(it+1,lhh,VEC_VV(it+1),&VEC_VV(0));CHKERRQ(ierr);
  for (j=0; j<=it; j++) {
    hh[j]  -= lhh[j];     /* hh += <v,vnew> */
    hes[j] += lhh[j];     /* hes += - <v,vnew> */
  }

  /*
   *  the second step classical Gram-Schmidt is only necessary
   *  when a simple test criteria is not passed
   */
  if (gmres->cgstype == KSP_GMRES_CGS_REFINE_IFNEEDED) {
    hnrm = 0.0;
    for (j=0; j<=it; j++) {
      hnrm  +=  PetscRealPart(lhh[j] * PetscConj(lhh[j]));
    }
    hnrm = sqrt(hnrm);
    ierr = VecNorm(VEC_VV(it+1),NORM_2, &wnrm);CHKERRQ(ierr);
    if (wnrm < 1.0286 * hnrm) {
      refine = PETSC_TRUE;
      PetscLogInfo(ksp,"KSPGMRESClassicalGramSchmidtOrthogonalization:Performing iterative refinement wnorm %g hnorm %g\n",wnrm,hnrm);
    }
  }

  if (refine) {
    ierr = VecMDot(it+1,VEC_VV(it+1),&(VEC_VV(0)),lhh);CHKERRQ(ierr); /* <v,vnew> */
    for (j=0; j<=it; j++) lhh[j] = - lhh[j];
    ierr = VecMAXPY(it+1,lhh,VEC_VV(it+1),&VEC_VV(0));CHKERRQ(ierr);
    for (j=0; j<=it; j++) {
      hh[j]  -= lhh[j];     /* hh += <v,vnew> */
      hes[j] += lhh[j];     /* hes += - <v,vnew> */
    }
  }

  if (it >= 501) {ierr = PetscFree(lhh);CHKERRQ(ierr);}
  ierr = PetscLogEventEnd(KSP_GMRESOrthogonalization,ksp,0,0,0);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}
EXTERN_C_END








