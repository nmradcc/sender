# sender
NMRA Conformance Test Software

This branch contains a refactor of the B 5.11.5 (V3-archive branch).

The build tools are upgraded to build under Visual Studio Code with CMake.

The VSC C/C++ Extension Pack and CMake Tools extensions are required.

You can use the Visual Studio Build Tools 2022 Release - x86 as the build kit but others may work also.

The send configuration files (*.INI) have been reworked to accept *.cfg files of the same format.

It will also accept legacy .INI files but is discuraged as using .INI files for parameters is a no-no in todays world.

