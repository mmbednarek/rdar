# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


.SUFFIXES: .hpux_make_needs_suffix_list


# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

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
CMAKE_COMMAND = /home/ego/jetbrains/clion/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/ego/jetbrains/clion/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ego/projects/rdar

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ego/projects/rdar/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/rdar.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rdar.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rdar.dir/flags.make

CMakeFiles/rdar.dir/src/main.cpp.o: CMakeFiles/rdar.dir/flags.make
CMakeFiles/rdar.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ego/projects/rdar/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rdar.dir/src/main.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rdar.dir/src/main.cpp.o -c /home/ego/projects/rdar/src/main.cpp

CMakeFiles/rdar.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rdar.dir/src/main.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ego/projects/rdar/src/main.cpp > CMakeFiles/rdar.dir/src/main.cpp.i

CMakeFiles/rdar.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rdar.dir/src/main.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ego/projects/rdar/src/main.cpp -o CMakeFiles/rdar.dir/src/main.cpp.s

CMakeFiles/rdar.dir/src/reader.cpp.o: CMakeFiles/rdar.dir/flags.make
CMakeFiles/rdar.dir/src/reader.cpp.o: ../src/reader.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ego/projects/rdar/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/rdar.dir/src/reader.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rdar.dir/src/reader.cpp.o -c /home/ego/projects/rdar/src/reader.cpp

CMakeFiles/rdar.dir/src/reader.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rdar.dir/src/reader.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ego/projects/rdar/src/reader.cpp > CMakeFiles/rdar.dir/src/reader.cpp.i

CMakeFiles/rdar.dir/src/reader.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rdar.dir/src/reader.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ego/projects/rdar/src/reader.cpp -o CMakeFiles/rdar.dir/src/reader.cpp.s

CMakeFiles/rdar.dir/src/util.cpp.o: CMakeFiles/rdar.dir/flags.make
CMakeFiles/rdar.dir/src/util.cpp.o: ../src/util.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ego/projects/rdar/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/rdar.dir/src/util.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rdar.dir/src/util.cpp.o -c /home/ego/projects/rdar/src/util.cpp

CMakeFiles/rdar.dir/src/util.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rdar.dir/src/util.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ego/projects/rdar/src/util.cpp > CMakeFiles/rdar.dir/src/util.cpp.i

CMakeFiles/rdar.dir/src/util.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rdar.dir/src/util.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ego/projects/rdar/src/util.cpp -o CMakeFiles/rdar.dir/src/util.cpp.s

CMakeFiles/rdar.dir/src/archive.cpp.o: CMakeFiles/rdar.dir/flags.make
CMakeFiles/rdar.dir/src/archive.cpp.o: ../src/archive.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ego/projects/rdar/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/rdar.dir/src/archive.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rdar.dir/src/archive.cpp.o -c /home/ego/projects/rdar/src/archive.cpp

CMakeFiles/rdar.dir/src/archive.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rdar.dir/src/archive.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ego/projects/rdar/src/archive.cpp > CMakeFiles/rdar.dir/src/archive.cpp.i

CMakeFiles/rdar.dir/src/archive.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rdar.dir/src/archive.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ego/projects/rdar/src/archive.cpp -o CMakeFiles/rdar.dir/src/archive.cpp.s

CMakeFiles/rdar.dir/src/file_sink.cpp.o: CMakeFiles/rdar.dir/flags.make
CMakeFiles/rdar.dir/src/file_sink.cpp.o: ../src/file_sink.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ego/projects/rdar/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/rdar.dir/src/file_sink.cpp.o"
	/usr/bin/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rdar.dir/src/file_sink.cpp.o -c /home/ego/projects/rdar/src/file_sink.cpp

CMakeFiles/rdar.dir/src/file_sink.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rdar.dir/src/file_sink.cpp.i"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ego/projects/rdar/src/file_sink.cpp > CMakeFiles/rdar.dir/src/file_sink.cpp.i

CMakeFiles/rdar.dir/src/file_sink.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rdar.dir/src/file_sink.cpp.s"
	/usr/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ego/projects/rdar/src/file_sink.cpp -o CMakeFiles/rdar.dir/src/file_sink.cpp.s

# Object files for target rdar
rdar_OBJECTS = \
"CMakeFiles/rdar.dir/src/main.cpp.o" \
"CMakeFiles/rdar.dir/src/reader.cpp.o" \
"CMakeFiles/rdar.dir/src/util.cpp.o" \
"CMakeFiles/rdar.dir/src/archive.cpp.o" \
"CMakeFiles/rdar.dir/src/file_sink.cpp.o"

# External object files for target rdar
rdar_EXTERNAL_OBJECTS =

rdar: CMakeFiles/rdar.dir/src/main.cpp.o
rdar: CMakeFiles/rdar.dir/src/reader.cpp.o
rdar: CMakeFiles/rdar.dir/src/util.cpp.o
rdar: CMakeFiles/rdar.dir/src/archive.cpp.o
rdar: CMakeFiles/rdar.dir/src/file_sink.cpp.o
rdar: CMakeFiles/rdar.dir/build.make
rdar: _deps/fmt-build/libfmtd.a
rdar: src/libww/liblibww.a
rdar: CMakeFiles/rdar.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ego/projects/rdar/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable rdar"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rdar.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rdar.dir/build: rdar

.PHONY : CMakeFiles/rdar.dir/build

CMakeFiles/rdar.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rdar.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rdar.dir/clean

CMakeFiles/rdar.dir/depend:
	cd /home/ego/projects/rdar/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ego/projects/rdar /home/ego/projects/rdar /home/ego/projects/rdar/cmake-build-debug /home/ego/projects/rdar/cmake-build-debug /home/ego/projects/rdar/cmake-build-debug/CMakeFiles/rdar.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rdar.dir/depend

