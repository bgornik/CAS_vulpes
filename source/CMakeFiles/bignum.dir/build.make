# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/bojan/cfiles/solve

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/bojan/cfiles/solve

# Include any dependencies generated for this target.
include source/CMakeFiles/bignum.dir/depend.make

# Include the progress variables for this target.
include source/CMakeFiles/bignum.dir/progress.make

# Include the compile flags for this target's objects.
include source/CMakeFiles/bignum.dir/flags.make

source/CMakeFiles/bignum.dir/bignum.cpp.o: source/CMakeFiles/bignum.dir/flags.make
source/CMakeFiles/bignum.dir/bignum.cpp.o: source/bignum.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bojan/cfiles/solve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object source/CMakeFiles/bignum.dir/bignum.cpp.o"
	cd /home/bojan/cfiles/solve/source && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/bignum.dir/bignum.cpp.o -c /home/bojan/cfiles/solve/source/bignum.cpp

source/CMakeFiles/bignum.dir/bignum.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/bignum.dir/bignum.cpp.i"
	cd /home/bojan/cfiles/solve/source && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/bojan/cfiles/solve/source/bignum.cpp > CMakeFiles/bignum.dir/bignum.cpp.i

source/CMakeFiles/bignum.dir/bignum.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/bignum.dir/bignum.cpp.s"
	cd /home/bojan/cfiles/solve/source && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/bojan/cfiles/solve/source/bignum.cpp -o CMakeFiles/bignum.dir/bignum.cpp.s

# Object files for target bignum
bignum_OBJECTS = \
"CMakeFiles/bignum.dir/bignum.cpp.o"

# External object files for target bignum
bignum_EXTERNAL_OBJECTS =

source/libbignum.a: source/CMakeFiles/bignum.dir/bignum.cpp.o
source/libbignum.a: source/CMakeFiles/bignum.dir/build.make
source/libbignum.a: source/CMakeFiles/bignum.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bojan/cfiles/solve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libbignum.a"
	cd /home/bojan/cfiles/solve/source && $(CMAKE_COMMAND) -P CMakeFiles/bignum.dir/cmake_clean_target.cmake
	cd /home/bojan/cfiles/solve/source && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bignum.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
source/CMakeFiles/bignum.dir/build: source/libbignum.a

.PHONY : source/CMakeFiles/bignum.dir/build

source/CMakeFiles/bignum.dir/clean:
	cd /home/bojan/cfiles/solve/source && $(CMAKE_COMMAND) -P CMakeFiles/bignum.dir/cmake_clean.cmake
.PHONY : source/CMakeFiles/bignum.dir/clean

source/CMakeFiles/bignum.dir/depend:
	cd /home/bojan/cfiles/solve && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bojan/cfiles/solve /home/bojan/cfiles/solve/source /home/bojan/cfiles/solve /home/bojan/cfiles/solve/source /home/bojan/cfiles/solve/source/CMakeFiles/bignum.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : source/CMakeFiles/bignum.dir/depend

