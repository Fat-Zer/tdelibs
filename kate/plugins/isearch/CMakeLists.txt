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

install( FILES ktexteditor_isearch.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES ktexteditor_isearchui.rc DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_isearch )


##### ktexteditor_isearch #######################

set( target ktexteditor_isearch )

set( ${target}_SRCS
  ISearchPlugin.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK ktexteditor-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
