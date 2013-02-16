#################################################
#
#  (C) 2010 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_subdirectory( pics )


add_definitions(
  -DQT_PLUGIN
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### makekdewidgets ############################

set( target makekdewidgets )

set( ${target}_SRCS
  makekdewidgets.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

##### kdewidgets ################################

set( target kdewidgets )

set( ${target}_SRCS
  classpreviews.cpp kdewidgets.cpp
)

add_custom_command( OUTPUT kdewidgets.cpp
  COMMAND ./makekdewidgets -o kdewidgets.cpp ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets
  DEPENDS makekdewidgets ${CMAKE_CURRENT_SOURCE_DIR}/kde.widgets
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK tdeabc-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer
)
