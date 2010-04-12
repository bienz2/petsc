#define PETSC_DLL
/*
     Provides utility routines for manulating any type of PETSc object.
*/
#include "petscsys.h"  /*I   "petscsys.h"    I*/

#undef __FUNCT__  
#define __FUNCT__ "PetscObjectGetClassid"
/*@C
   PetscObjectGetClassid - Gets the classid for any PetscObject, 

   Not Collective
   
   Input Parameter:
.  obj - any PETSc object, for example a Vec, Mat or KSP.
         Thus must be cast with a (PetscObject), for example, 
         PetscObjectGetClassid((PetscObject)mat,&classid);

   Output Parameter:
.  classid - the classid

   Level: developer

@*/
PetscErrorCode PETSC_DLLEXPORT PetscObjectGetClassid(PetscObject obj,PetscClassId *classid)
{
  PetscFunctionBegin;
  PetscValidHeader(obj,1);
  *classid = obj->classid;
  PetscFunctionReturn(0);
}

