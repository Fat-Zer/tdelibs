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

add_definitions(
  ${TQT_CFLAGS_OTHER}
  -D_KDEPRINT_COMPILE
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kdeprint
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES ext.print DESTINATION ${DATA_INSTALL_DIR}/kdeprint/plugins )


##### kdeprint_ext ##############################

set( target kdeprint_ext )

set( ${target}_SRCS
  kmextfactory.cpp kmextmanager.cpp kextprinterimpl.cpp
  kmextuimanager.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeprint-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
