cmake_minimum_required (VERSION 2.6)
project (shark)
# The version number.
set (shark_VERSION_MAJOR 0)
set (shark_VERSION_MINOR 1)

include(CheckIncludeFile)
include(CheckLibraryExists)


# -fopenmp -fpermissive
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCPU_ONLY -std=c++11 -fopenmp -mfpu=neon-vfpv4 -funsafe-math-optimizations -ftree-vectorize -fomit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp")

#enable for debug builds.
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)


# look for PointGrey/FLIR FlyCapture camera support libs
# on ubuntu it puts the flycap in /usr/include
set(PG_SDK_ROOT "/usr/local/include/flycapture")
check_include_file("${PG_SDK_ROOT}/C/FlyCapture2_C.h" HAVE_PG_SDK_HEADER)
find_library(LIB_FLYCAP flycapture)
find_library(LIB_FLYCAP_C flycapture-c)
check_library_exists(flycapture-c fc2CreateContext ${LIB_FLYCAP_C} HAVE_PG_SDK)

#check raspicam
find_library(LIB_RASPICAM raspicam)
check_library_exists(raspicam _ZNK8raspicam8_private12Private_Impl5getIdEv ${LIB_RASPICAM} HAVE_RASPICAM)

#check wiringPi
find_library(LIB_WIRINGPI wiringPi)
check_library_exists(wiringPi piBoardRev ${LIB_WIRINGPI} HAVE_WIRINGPI)

# add the binary tree to the search path for include files
# so that we will find SharkConfig.h
include_directories("${PROJECT_BINARY_DIR}" "src" "contrib" ${PG_SDK_ROOT})

#our executable
add_executable(shark src/main.cpp src/json.cpp src/config.cpp src/pointgrey.cpp contrib/joystick/joystick.cc contrib/jsmn/jsmn.c contrib/v4l_helper/capture_raw_frames.c)

#link libraries
TARGET_LINK_LIBRARIES(shark zmq czmq pthread)

#conditional libraries

if(HAVE_WIRINGPI)
  add_subdirectory(contrib/mcqueen/car)
  TARGET_LINK_LIBRARIES(shark car ${LIB_WIRINGPI})
  set (ENABLE_CAR 1)
else()
  set (ENABLE_CAR 0)
endif()

if(HAVE_RASPICAM)
  TARGET_LINK_LIBRARIES(shark ${LIB_RASPICAM})
  set (ENABLE_RASPICAM 1)
else()
  set (ENABLE_RASPICAM 0)
endif()

if(HAVE_PG_SDK_HEADER AND HAVE_PG_SDK)
  TARGET_LINK_LIBRARIES(shark ${LIB_FLYCAP_C} ${LIB_FLYCAP})
  set (ENABLE_PG_SDK 1)
else()
  set (ENABLE_PG_SDK 0)
endif()

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/SharkConfig.h.in"
  "${PROJECT_BINARY_DIR}/SharkConfig.h"
  )

# add the install targets
install (TARGETS shark DESTINATION bin)
