cmake_minimum_required(VERSION 2.8.11)

project(mown)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/config/cmake/")
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc and generate .ui file headers automatically when needed.
set(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)  

if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PLATFORM 64)
    MESSAGE(STATUS "Detected 64-bit platform")
else()
    set(PLATFORM 32)
    MESSAGE(STATUS "Detected 32-bit platform")
endif()

# Find the used Qt libraries
find_package(Qt5Widgets)
find_package(Qt5WebEngineWidgets)

# Find yaml-cpp
set(YAMLCPP_USE_STATIC_LIBS ON CACHE BOOL "If true, will try to find static YamlCpp first instead of dynamic.")
find_package(YamlCpp)
if(NOT YAMLCPP_FOUND)
	set(YAMLCPP_USE_STATIC_LIBS OFF CACHE BOOL "If true, will try to find static YamlCpp first instead of dynamic.")
	find_package(YamlCpp)
endif()

if(YAMLCPP_FOUND)
	include_directories(${YAMLCPP_INCLUDE_DIR})
	link_directories(${YAMLCPP_LIBRARY_DIR})
	message(STATUS "Found the following YamlCpp library:")
	message("  ${YAMLCPP_LIBRARY}\n")
else()
	message(FATAL_ERROR "You need yamlcpp to build this, build yamlcpp in the 'dependencies/' directory, or install it as a system package")
endif()

# Find boost
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

if(DEFINED ENV{BOOST_ROOT})
  set(BOOST_ROOT $ENV{BOOST_ROOT})
  if (MSVC14)
    set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib${PLATFORM}-msvc-14.0)
  endif()
endif()

find_package(Boost REQUIRED COMPONENTS date_time filesystem system)


include_directories(${Boost_INCLUDE_DIRS})
include_directories(src)

# Tell CMake to create the mown executable
add_executable(mown WIN32
  src/article.cpp
  src/articletag.cpp
  src/clickablestatusbar.cpp
  src/contentfactory.cpp
  src/main.cpp
  src/mainwindow.cpp
  src/mown.cpp
  src/projectsettings.cpp)

target_link_libraries(mown ${Boost_LIBRARIES})

# Use the modules from Qt 5.
target_link_libraries(mown Qt5::Widgets)
target_link_libraries(mown Qt5::WebEngineWidgets)

target_link_libraries(mown ${YAMLCPP_LIBNAME})

include(WinDeployQt)
WinDeployQt(TARGET mown COMPILER_RUNTIME INCLUDE_MODULES ${QTLIBS})

#INSTALL(TARGETS mown DESTINATION ".")
INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release" DESTINATION ".")

INCLUDE(CPack)
