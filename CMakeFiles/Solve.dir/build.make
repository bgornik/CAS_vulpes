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
include CMakeFiles/Solve.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Solve.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Solve.dir/flags.make

CMakeFiles/Solve.dir/solve.cpp.o: CMakeFiles/Solve.dir/flags.make
CMakeFiles/Solve.dir/solve.cpp.o: solve.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/bojan/cfiles/solve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Solve.dir/solve.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Solve.dir/solve.cpp.o -c /home/bojan/cfiles/solve/solve.cpp

CMakeFiles/Solve.dir/solve.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Solve.dir/solve.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/bojan/cfiles/solve/solve.cpp > CMakeFiles/Solve.dir/solve.cpp.i

CMakeFiles/Solve.dir/solve.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Solve.dir/solve.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/bojan/cfiles/solve/solve.cpp -o CMakeFiles/Solve.dir/solve.cpp.s

# Object files for target Solve
Solve_OBJECTS = \
"CMakeFiles/Solve.dir/solve.cpp.o"

# External object files for target Solve
Solve_EXTERNAL_OBJECTS =

Solve: CMakeFiles/Solve.dir/solve.cpp.o
Solve: CMakeFiles/Solve.dir/build.make
Solve: source/libexpression.a
Solve: source/libfunctions.a
Solve: source/libbignum.a
Solve: CMakeFiles/Solve.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/bojan/cfiles/solve/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Solve"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Solve.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Solve.dir/build: Solve

.PHONY : CMakeFiles/Solve.dir/build

CMakeFiles/Solve.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Solve.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Solve.dir/clean

CMakeFiles/Solve.dir/depend:
	cd /home/bojan/cfiles/solve && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/bojan/cfiles/solve /home/bojan/cfiles/solve /home/bojan/cfiles/solve /home/bojan/cfiles/solve /home/bojan/cfiles/solve/CMakeFiles/Solve.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Solve.dir/depend

