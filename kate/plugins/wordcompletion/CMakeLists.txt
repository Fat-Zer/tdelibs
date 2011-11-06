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

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/interfaces
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install( FILES ktexteditor_docwordcompletion.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES docwordcompletionui.rc DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_docwordcompletion )


##### ktexteditor_docwordcompletion #############

set( target ktexteditor_docwordcompletion )

set( ${target}_SRCS
  docwordcompletion.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK ktexteditor-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
