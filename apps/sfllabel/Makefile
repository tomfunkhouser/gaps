#
# Application name and list of source files.
#

NAME=sfllabel
CCSRCS=$(NAME).cpp io.cpp glfw_ui.cpp glut_ui.cpp



#
# GLFW stuff for MAC
#

OS=$(shell uname -s)
ifeq ("$(OS)","Darwin")
  USER_CFLAGS=-DUSE_GLFW -DGL_SILENCE_DEPRECATION 
  USER_LIBS=-LGLFW -lglfw3 -framework OpenGL -framework Cocoa -framework IOKit
endif



#
# Dependency libraries
#

PKG_LIBS=-lR3Utils -lR3Surfels -lR3Graphics -lR3Shapes -lR2Shapes -lRNBasics -ljpeg -lpng



#
# R3 application makefile
#

include ../../makefiles/Makefile.apps


