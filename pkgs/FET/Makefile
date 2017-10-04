#
# Package name 
#

NAME=FET



#
# List of source files
#

CCSRCS=$(NAME).cpp \
  FETReconstruction.cpp \
  FETSequence.cpp \
  FETMatch.cpp \
  FETShape.cpp \
  FETCorrespondence.cpp \
  FETFeature.cpp \
  FETDescriptor.cpp 



#
# Include flags for optimization
# 

#USER_CFLAGS=-I/usr/local/suitesparse -I/usr/local/include/eigen3 -DRN_USE_CERES -DRN_USE_MINPACK -DRN_USE_SPLM -DRN_USE_CSPARSE
#USER_CFLAGS=-I/usr/include/suitesparse -I/usr/include/eigen3 -DRN_USE_CERES -DRN_USE_MINPACK -DRN_USE_SPLM -DRN_USE_CSPARSE
#USER_CFLAGS=-DRN_USE_CSPARSE -DRN_USE_SPLM
USER_CFLAGS=-DRN_USE_CSPARSE



#
# Dependencies
#

PKG_DEPENDENCIES=R3Shapes R2Shapes RNMath RNBasics CSparse



#
# PKG makefile
#

include ../../makefiles/Makefile.pkgs


