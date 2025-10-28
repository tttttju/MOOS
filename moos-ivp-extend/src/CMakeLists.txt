##############################################################################
# FILE:        moos-ivp-extend/src/CMakeLists.txt
# DATE:        2010/09/07
#              2020/05/09 minor mods
# DESCRIPTION: CMakeLists.txt file for the moos-ivp-extend source directory
##############################################################################


#============================================================================
# Add the libraries in the current directory to the include path
#============================================================================
FILE(GLOB LOCAL_LIBRARY_DIRS ./lib_*)
INCLUDE_DIRECTORIES(${LOCAL_LIBRARY_DIRS})
FILE(GLOB LOCAL_PROGRAM_DIRS ./p*)
INCLUDE_DIRECTORIES(${LOCAL_PROGRAM_DIRS})

INCLUDE_DIRECTORIES(lib_common_utils/include)


#============================================================================
# List the subdirectories to build...
#============================================================================

ADD_SUBDIRECTORY(uReplay)


##############################################################################
#                           END of CMakeLists.txt
##############################################################################
