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
  -DQT_PLUGIN
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdefx
  ${CMAKE_SOURCE_DIR}/kdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kthemestyle ###############################

set( target kthemestyle )

set( ${target}_SRCS
  kthemebase.cpp kthemestyle.cpp kstyledirs.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdefx-shared kdecore-shared dl
  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles
)
