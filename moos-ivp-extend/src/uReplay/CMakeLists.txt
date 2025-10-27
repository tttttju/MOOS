#--------------------------------------------------------
# The CMakeLists.txt for:                        uReplay
#--------------------------------------------------------

# Set System Specific Libraries
if (${WIN32})
  SET(SYSTEM_LIBS
    wsock32)
else ()
  SET(SYSTEM_LIBS
    m
    pthread)
endif ()

FILE(GLOB SRC *.cpp)

ADD_EXECUTABLE(uReplay ${SRC})

TARGET_LINK_LIBRARIES(uReplay
  ${MOOS_LIBRARIES}
  ulogview
  logic
  mbutil
  geometry
  ivpbuild
  ivpcore
  ${SYSTEM_LIBS})
