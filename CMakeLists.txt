# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("SandDemo")

if(WIN32)
	add_definitions(-D_WIN32)
elseif(LINUX)
	add_definitions(-D__linux__)
endif


# Include sub-projects.
add_subdirectory ("SandEngine")
add_subdirectory ("App")
