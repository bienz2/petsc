#!/usr/bin/env python

configure_options = [
  'CC=pgcc',
  'CXX=pgc++',
  'FC=pgf90',
  '--with-hwloc=0', # ubuntu -lhwloc requires -lnuma - which conflicts with -lnuma from pgf90
  '--download-mpich=1',
  '--download-fblaslapack=1',
  '--with-cxx-dialect=C++11',
  '--download-codipack=1',
  '--download-adblaslapack=1',
  ]

if __name__ == '__main__':
  import sys,os
  sys.path.insert(0,os.path.abspath('config'))
  import configure
  configure.petsc_configure(configure_options)
