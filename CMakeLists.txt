cmake_minimum_required(VERSION 2.8.11)

project(Mown)

#Generate version (format: v0.1-beta.2)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_SUFFIX_NAME "") #alpha, beta, patch. Empty for releases.
set(VERSION_SUFFIX_NUMBER 1) #only if this is not a release

if(VERSION_SUFFIX_NAME STREQUAL "")
  set(LONG_VERSION "v${VERSION_MAJOR}.${VERSION_MINOR}")
else()
  set(LONG_VERSION "v${VERSION_MAJOR}.${VERSION_MINOR}-${VERSION_SUFFIX_NAME}.${VERSION_SUFFIX_NUMBER}")
endif()

execute_process(
  COMMAND git --git-dir=${PROJECT_SOURCE_DIR}/.git rev-parse --verify HEAD
  RESULT_VARIABLE git_returncode
  OUTPUT_VARIABLE BUILD_HASH
  ERROR_VARIABLE git_error
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT ${git_returncode} STREQUAL 0)
  message(FATAL_ERROR "git error ${git_error} ${git_returncode}")
endif(NOT ${git_returncode} STREQUAL 0)
string(SUBSTRING ${BUILD_HASH} 0 8 BUILD_HASH)

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


file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE ALL_HEADER_FILES src/*.h)

# Tell CMake to create the mown executable
add_executable(Mown WIN32
  ${ALL_SOURCE_FILES}
  ${ALL_HEADER_FILES})

target_link_libraries(Mown
  ${Boost_LIBRARIES}
  Qt5::Widgets
  Qt5::WebEngineWidgets
  ${YAMLCPP_LIBNAME})

include(WinDeployQt)
WinDeployQt(TARGET Mown COMPILER_RUNTIME INCLUDE_MODULES ${QTLIBS})

INSTALL(TARGETS Mown RUNTIME DESTINATION .)
set(CPACK_PACKAGE_EXECUTABLES Mown "Mown")

if (MSVC)
  #Windows only: deploy windeployqt files
  #TODO: try to run WinDeployQt when packaging, so we do not have to copy all these manually
  INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/bearer" DESTINATION ".")
  INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/iconengines" DESTINATION ".")
  INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/imageformats" DESTINATION ".")
  INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/platforms" DESTINATION ".")
  INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/resources" DESTINATION ".")
  INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Release/translations" DESTINATION ".")
  INSTALL(FILES
    ${QT_DEPLOY_DLLS}
    ${CMAKE_CURRENT_BINARY_DIR}/Release/D3Dcompiler_47.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/libEGL.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/libGLESV2.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/opengl32sw.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5Core.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5Gui.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5Network.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5Qml.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5Quick.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5Svg.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5WebChannel.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5WebEngineCore.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5WebEngineWidgets.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/Qt5Widgets.dll
    ${CMAKE_CURRENT_BINARY_DIR}/Release/QtWebEngineProcess.exe
    DESTINATION ".")
endif()

set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(CPACK_PACKAGE_DESCRIPTION "My Own Website Now!")
set(CPACK_PACKAGE_VENDOR "Camille Masson")
set(CPACK_PACKAGE_CONTACT "")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_FILE_NAME "Mown-${LONG_VERSION}-${CMAKE_SYSTEM_NAME}${PLATFORM}-${BUILD_HASH}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/installer/agreement.txt")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Mown")

#NSIS specifics
set(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_DISPLAY_NAME "Mown")

INCLUDE(CPack)
