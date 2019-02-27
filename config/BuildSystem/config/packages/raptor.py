import config.package

class Configure(config.package.CMakePackage):
  def __init__(self, framework):
    config.package.CMakePackage.__init__(self, framework)
    self.gitcommit         = 'v0.4'
    self.download          = ['https://github.com/raptor-library/raptor.git','https://github.com/raptor-library/raptor/releases/download/'+self.gitcommit+'/raptor.tar.gz']
    self.downloaddirnames  = ['raptor']
    self.functions         = ['sor']
    self.includes          = ['raptor.hpp']
    self.liblist           = [['libraptor.a']]
    self.cxx               = 1
    self.requirescxx11     = 1
    self.downloadonWindows = 0
    self.precisions        = ['double']
    self.complex           = 0
    self.hastests          = 1
    self.hastestsdatafiles = 1
    return

  def setupDependencies(self, framework):
    config.package.CMakePackage.setupDependencies(self, framework)
    self.cxxlibs    = framework.require('config.packages.cxxlibs',self)
    self.blasLapack = framework.require('config.packages.BlasLapack',self)
    self.mpi        = framework.require('config.packages.MPI',self)
    self.mathlib    = framework.require('config.packages.mathlib',self)
    self.deps       = [self.mpi,self.blasLapack,self.cxxlibs,self.mathlib]    
    return

  def formCMakeConfigureArgs(self):
    if not self.cmake.found:
      raise RuntimeError('CMake > 2.5 is needed to build RAPtor\nSuggest adding --download-cmake to ./configure arguments')

    args = config.package.CMakePackage.formCMakeConfigureArgs(self)
    if self.checkSharedLibrariesEnabled():
      args.append('-DSHARED=1')
    if self.compilerFlags.debugging:
      args.append('-DDEBUG=1')
    return args

