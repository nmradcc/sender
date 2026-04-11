# sender
NMRA Conformance Test Software

This branch contains a refactor of the B 5.11.5 (V3-archive branch) reworked and retooled to support operation on modern PC (64bit Windows) with attached STM32 Nucleo controller for timing and IO.

The build tools are upgraded to build under Visual Studio Code with CMake.

The VSC C/C++ Extension Pack and CMake Tools extensions are required.

You can use the Visual Studio Build Tools 2022 Release - x86 as the build kit but others may work also.

There are two projects:
1. The host PC software
2. The STM32 controller (MCU) software

The send configuration files (*.INI) have been reworked to accept *.cfg files of the same format.

It will also accept legacy .INI files but is discuraged as using .INI files for parameter passing is a no-no in todays world.

Much of the documentation may still show .INI

Most of old Visual Source Safe (scc stuff) has been remove. See the V3-archive branch for previous history.



