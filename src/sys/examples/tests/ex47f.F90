! Example for PetscOptionsInsertFileYAML: Fortran Example

program main

#include <petsc/finclude/petscsys.h>
      use petscsys
      
      implicit none
      PetscErrorCode      :: ierr
      character(len=256)  :: filename
      PetscBool           ::  flg

      call PetscInitialize(PETSC_NULL_CHARACTER,ierr)
      if (ierr /= 0) then
        write(6,*)'Unable to initialize PETSc'
        stop
      endif
      
      call PetscOptionsGetString(PETSC_NULL_OPTIONS,PETSC_NULL_CHARACTER,"-f",filename,flg,ierr)
      if (flg) then
        call PetscOptionsInsertFileYAML(PETSC_COMM_WORLD,filename,PETSC_TRUE,ierr)
      end if

      call  PetscOptionsView(PETSC_NULL_OPTIONS,PETSC_VIEWER_STDOUT_WORLD,ierr)
      call  PetscFinalize(ierr)
      
!    build:
!      requires: yaml

!   test:
!      suffix: 1
!      requires: yaml      args: -f petsc.yml
!      filter:  grep -v saws_port_auto_select |grep -v malloc_dump | grep -v display
!      localrunfiles: petsc.yml

!   test:
!      suffix: 2
!      requires: yaml
!      filter:  grep -v saws_port_auto_select
!      args: -options_file_yaml petsc.yml |grep -v malloc_dump | grep -v display
!      localrunfiles: petsc.yml
      
end program main 
