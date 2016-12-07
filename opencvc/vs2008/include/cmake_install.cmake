# Install script for directory: F:/open/lib/opencvc/sources/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "F:/open/lib/opencvc/vs2008/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv" TYPE FILE FILES
    "F:/open/lib/opencvc/sources/include/opencv/cv.h"
    "F:/open/lib/opencvc/sources/include/opencv/cv.hpp"
    "F:/open/lib/opencvc/sources/include/opencv/cvaux.h"
    "F:/open/lib/opencvc/sources/include/opencv/cvaux.hpp"
    "F:/open/lib/opencvc/sources/include/opencv/cvwimage.h"
    "F:/open/lib/opencvc/sources/include/opencv/cxcore.h"
    "F:/open/lib/opencvc/sources/include/opencv/cxcore.hpp"
    "F:/open/lib/opencvc/sources/include/opencv/cxeigen.hpp"
    "F:/open/lib/opencvc/sources/include/opencv/cxmisc.h"
    "F:/open/lib/opencvc/sources/include/opencv/highgui.h"
    "F:/open/lib/opencvc/sources/include/opencv/ml.h"
    )
endif()

if(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "dev")
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/opencv2" TYPE FILE FILES "F:/open/lib/opencvc/sources/include/opencv2/opencv.hpp")
endif()

