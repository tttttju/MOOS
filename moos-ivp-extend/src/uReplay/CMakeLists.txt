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

set(ALOGVIEW_DIR ${MOOSIVP_SOURCE_TREE_BASE}/ivp/src/app_alogview)

set(SRC
  ${ALOGVIEW_DIR}/EncounterViewer.cpp
  ${ALOGVIEW_DIR}/GUI_Encounters.cpp
  ${ALOGVIEW_DIR}/NavPlotViewer.cpp
  ${ALOGVIEW_DIR}/LogPlotViewer.cpp
  ${ALOGVIEW_DIR}/REPLAY_GUI.cpp
  ${ALOGVIEW_DIR}/GUI_IPF.cpp
  ${ALOGVIEW_DIR}/GUI_HelmScope.cpp
  ${ALOGVIEW_DIR}/GUI_VarScope.cpp
  ${ALOGVIEW_DIR}/GUI_AppLogScope.cpp
  ${ALOGVIEW_DIR}/GUI_TaskDiary.cpp
  ${ALOGVIEW_DIR}/IvPFuncViewerX.cpp
  ${ALOGVIEW_DIR}/LogViewLauncher.cpp
  uReplayMain.cpp)

include_directories(${ALOGVIEW_DIR})

ADD_EXECUTABLE(uReplay ${SRC})

TARGET_LINK_LIBRARIES(uReplay
  ${MOOS_LIBRARIES}
  marineview
  logutils
  helmivp
  geometry
  contacts
  ipfview
  encounters
  ivpbuild
  apputil
  mbutil
  ${MOOSGeodesy_LIBRARIES}
  ${SYSTEM_LIBS}
  fltk
  fltk_gl
  dl
  tiff)
