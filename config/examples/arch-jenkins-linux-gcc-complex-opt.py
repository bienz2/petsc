#!/usr/bin/env python
if __name__ == '__main__':
  import sys
  import os
  sys.path.insert(0, os.path.abspath('config'))
  import configure
  configure_options = [
    '--with-cc=/home/petsc/soft/mpich-3.3b1/bin/mpicc',
    '--with-fc=/home/petsc/soft/mpich-3.3b1/bin/mpif90',
    '--with-cxx=/home/petsc/soft/mpich-3.3b1/bin/mpicxx',
    '--with-clanguage=c',
    '--with-shared-libraries=yes',
    '--with-debugging=no',
    '--with-scalar-type=complex',
    '--with-64-bit-indices=no',
    '--with-precision=double',
    '--with-sowing=1',
    '--with-sowing-dir=/home/petsc/soft/sowing-v1.1.25-p1',
    'DATAFILESPATH=/home/petsc/datafiles',
    ]
  configure.petsc_configure(configure_options)
