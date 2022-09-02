GAPS Users -

This directory contains all code for the GAPS software library.
There are several subdirectories:

    pkgs - source and include files for all packages (software libraries).
    apps - source files for several application and example programs. 
    makefiles - unix-style make file definitions
    vc - visual studio solution files
    lib - archive library (.lib) files (created during compilation).
    bin - executable files (created during compilation).

Installing: GAPS depends only on c++ (gcc) and opengl development
libraries (opengl, glu, and glew), and they need to be installed prior
to compiling GAPS.  These libraries are installed by default on many
systems.  However, you may need to install the opengl libraries
explicitly.  For example, on a Debian Linux system, you can install
the glew development library with "sudo apt-get install libglew-dev".

Compiling: If you are using linux or cygwin and have gcc and OpenGL
development libraries installed, or if you are using MAC OS X with the
xcode development environment, you should be able to compile all the
code by typing "make clean; make" in this directory.  If you are using
Windows Visual Studio 10 or later, then you should be able to open the
solution file vc.sln in the vc subdirectory and then "Rebuild
Solution."  For other development platforms, you should edit the
shared compilation settings in the makefiles/Makefiles.std to meet
your needs.

Running: After GAPS has been compiled, the executables for all the
apps will be in gaps/bin/<arch>/ (where <arch> is a string indicating
the architecture of the compile-time system -- probably x86_64).  It
will be convenient to include that directory in your PATH if you plan
on running gaps apps multiple times (e.g., by adding
PATH="~/gaps/bin/x86_64:${PATH}" to your .bash_profile).

Development: To write a program that uses the GAPS pkgs, then you
should include "-I XXX/gaps/pkgs" in your compile flags (CFLAGS) and
"-L XXX/gaps/lib" in your link flags (LDFLAGS), where XXX is the
directory where you installed the gaps software.

License: The software is distributed under the MIT license (see
LICENSE.txt) and thus can be used for any purpose without warranty,
any liability, or any suport of any kind.

Have fun!

- Tom Funkhouser



