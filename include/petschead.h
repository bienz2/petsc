/* $Id: petschead.h,v 1.66 1998/07/21 16:25:52 bsmith Exp balay $ */

/*
    Defines the basic header of all PETSc objects.
*/

#if !defined(_PHEAD_H)
#define _PHEAD_H
#include "petsc.h"  

extern int PetscCommDup_Private(MPI_Comm,MPI_Comm*,int*);
extern int PetscCommFree_Private(MPI_Comm*);

extern int PetscRegisterCookie(int *);

/*
   All major PETSc data structures have a common core; this is defined 
   below by PETSCHEADER. 

   PetscHeaderCreate() should be used whenever creating a PETSc structure.
*/

/*
   PetscOps: structure of core operations that all PETSc objects support.
   
      getcomm()         - Gets the object's communicator.
      view()            - Is the routine for viewing the entire PETSc object; for
                          example, MatView() is the general matrix viewing routine.
      reference()       - Increases the reference count for a PETSc object; when
                          a reference count reaches zero it is destroyed.
      destroy()         - Is the routine for destroying the entire PETSc object; 
                          for example, MatDestroy() is the general matrix 
                          destruction routine.
      compose()         - Associates a PETSc object with another PETSc object.
      query()           - Returns a different PETSc object that has been associated
                          with the first object.
      composefunction() - Attaches an additional registered function.
      queryfunction()   - Requests a registered function that has been registered.
      composelanguage() - associates the object's representation in a different language
      querylanguage()   - obtain the object's representation in a different language
*/

typedef struct {
   int (*getcomm)(PetscObject,MPI_Comm *);
   int (*view)(PetscObject,Viewer);
   int (*reference)(PetscObject);
   int (*destroy)(PetscObject);
   int (*compose)(PetscObject,const char[],PetscObject);
   int (*query)(PetscObject,const char[],PetscObject *);
   int (*composefunction)(PetscObject,const char[],const char[],void *);
   int (*queryfunction)(PetscObject,const char[], void **);
   int (*composelanguage)(PetscObject,PetscLanguage,void *);
   int (*querylanguage)(PetscObject,PetscLanguage,void **);
   int (*publish)(PetscObject);
} PetscOps;

#define PETSCHEADER(ObjectOps)                         \
  int         cookie;                                  \
  PetscOps    *bops;                                   \
  ObjectOps   *ops;                                    \
  MPI_Comm    comm;                                    \
  int         type;                                    \
  PLogDouble  flops,time,mem;                          \
  int         id;                                      \
  int         refct;                                   \
  int         tag;                                     \
  FList      qlist;                                   \
  OList       olist;                                   \
  char        *type_name;                              \
  PetscObject parent;                                  \
  char*       name;                                    \
  char        *prefix;                                 \
  void        *cpp;                                    \
  int         amem;                                    \
  void**      fortran_func_pointers;       

  /*  ... */                               

#define  PETSCFREEDHEADER -1

extern int PetscHeaderCreate_Private(PetscObject,int,int,MPI_Comm,int (*)(PetscObject),int (*)(PetscObject,Viewer));
extern int PetscHeaderDestroy_Private(PetscObject);

/*
    PetscHeaderCreate - Creates a PETSc object

    Input Parameters:
+   tp - the data structure type of the object
.   pops - the data structure type of the objects operations (for example VecOps)
.   cook - the cookie associated with this object
.   t - type (no longer should be used)
.   com - the MPI Communicator
.   des - the destroy routine for this object
-   vie - the view routine for this object

    Output Parameter:
.   h - the newly created object
*/ 
#define PetscHeaderCreate(h,tp,pops,cook,t,com,des,vie)                                     \
  { int _ierr;                                                                              \
    h       = PetscNew(struct tp);CHKPTRQ((h));                                             \
    PetscMemzero(h,sizeof(struct tp));                                                      \
    (h)->bops = PetscNew(PetscOps);CHKPTRQ(((h)->bops));                                    \
    PetscMemzero((h)->bops,sizeof(sizeof(PetscOps)));                                       \
    (h)->ops  = PetscNew(pops);CHKPTRQ(((h)->ops));                                         \
    _ierr = PetscHeaderCreate_Private((PetscObject)h,cook,t,com,                            \
                                       (int (*)(PetscObject))des,                           \
                                       (int (*)(PetscObject,Viewer))vie); CHKERRQ(_ierr);   \
  }

#define PetscHeaderDestroy(h)                                             \
  { int _ierr;                                                            \
    _ierr = PetscHeaderDestroy_Private((PetscObject) (h)); CHKERRQ(_ierr);\
  }                 

/* ---------------------------------------------------------------------------------------*/

/* 
    Macros to test if a PETSc object is valid and if pointers are
valid

*/
#define PetscValidHeaderSpecific(h,ck)                              \
  {if (!h) {SETERRQ(PETSC_ERR_ARG_CORRUPT,0,"Null Object");}        \
  if ((unsigned long)h & (unsigned long)3) {                        \
    SETERRQ(PETSC_ERR_ARG_CORRUPT,0,"Invalid Pointer to Object");   \
  }                                                                 \
  if (((PetscObject)(h))->cookie != ck) {                           \
    if (((PetscObject)(h))->cookie == PETSCFREEDHEADER) {           \
      SETERRQ(PETSC_ERR_ARG_CORRUPT,0,"Object already free");       \
    } else {                                                        \
      SETERRQ(PETSC_ERR_ARG_WRONG,0,"Wrong Object");                \
    }                                                               \
  }} 

#define PetscValidHeader(h)                                         \
  {if (!h) {SETERRQ(PETSC_ERR_ARG_CORRUPT,0,"Null Object");}        \
  if ((unsigned long)h & (unsigned long)3) {                        \
    SETERRQ(PETSC_ERR_ARG_CORRUPT,0,"Invalid Pointer to Object");   \
  } else if (((PetscObject)(h))->cookie == PETSCFREEDHEADER) {      \
      SETERRQ(PETSC_ERR_ARG_CORRUPT,0,"Object already free");       \
  } else if (((PetscObject)(h))->cookie < PETSC_COOKIE ||           \
      ((PetscObject)(h))->cookie > LARGEST_PETSC_COOKIE) {          \
      SETERRQ(PETSC_ERR_ARG_CORRUPT,0,"Invalid Object");            \
  }}

#define PetscValidIntPointer(h)                                     \
  {if (!h) {SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Null Pointer");}        \
  if ((unsigned long)h & (unsigned long)3){                         \
    SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Invalid Pointer to Int");       \
  }}

#define PetscValidPointer(h)                                        \
  {if (!h) {SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Null Pointer");}        \
  if ((unsigned long)h & (unsigned long)3){                         \
    SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Invalid Pointer");              \
  }}

#if !defined(HAVE_DOUBLE_ALIGN)
#define PetscValidScalarPointer(h)                                  \
  {if (!h) {SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Null Pointer");}        \
  if ((unsigned long)h & (unsigned long)3) {                        \
    SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Invalid Pointer to Scalar");    \
  }}
#else
#define PetscValidScalarPointer(h)                                  \
  {if (!h) {SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Null Pointer");}        \
  if ((unsigned long)h & (unsigned long)7) {                        \
    SETERRQ(PETSC_ERR_ARG_BADPTR,0,"Invalid Pointer to Scalar");    \
  }}
#endif

/*
    For example, in the dot product between two vectors,
  both vectors must be either Seq or MPI, not one of each 
*/
#define PetscCheckSameType(a,b) \
  if ((a)->type != (b)->type) SETERRQ(PETSC_ERR_ARG_NOTSAMETYPE,0,"Objects not of same type");

/*
   All PETSc objects begin with the fields defined in PETSCHEADER.
   The PetscObject is a way of examining these fields regardless of 
   the specific object. In C++ this could be a base abstract class
   from which all objects are derived.
*/
struct _p_PetscObject {
  PETSCHEADER(int)
};


#endif


