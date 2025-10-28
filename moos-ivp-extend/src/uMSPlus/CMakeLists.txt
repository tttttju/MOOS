#--------------------------------------------------------
# The CMakeLists.txt for:                        uMSPlus
#--------------------------------------------------------

cmake_minimum_required(VERSION 2.8)

# Resources for platform specific builds
set(RESOURCES)
if(WIN32)
  list(APPEND RESOURCES icon1.ico uMSPlus.rc)
elseif(APPLE)
  set(MACOSX_BUNDLE_ICON_FILE moose.icns)
  set_source_files_properties(Mac/moose.icns
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  list(APPEND RESOURCES Mac/moose.icns)
endif()

# Build a list of files to compile
set(UMSPLUS_SRCS
  ScopeGrid.cpp
  ScopeTabPane.cpp
  DBImage.cpp
  uMSPlus.cpp
  ScopeGrid.h
  ScopeTabPane.h
  DBImage.h
  ${RESOURCES})

find_package(MOOS REQUIRED)
find_package(FLTK REQUIRED)
find_package(OpenGL REQUIRED)

# Build third-party FLTK widgets bundled with the original uMS
add_subdirectory(Thirdparty)

include_directories(
  ${MOOS_INCLUDE_DIRS}
  ${MOOS_DEPEND_INCLUDE_DIRS}
  ${fltkvw_INCLUDE_DIRS}
  ${fltkvw_DEPEND_INCLUDE_DIRS})

add_executable(uMSPlus WIN32 MACOSX_BUNDLE ${UMSPLUS_SRCS})

target_link_libraries(uMSPlus
  ${MOOS_LIBRARIES}
  ${MOOS_DEPEND_LIBRARIES}
  ${fltkvw_LIBRARIES}
  ${fltkvw_DEPEND_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${FLTK_LIBRARIES})
