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
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### kinstalltheme #############################

set( target kinstalltheme )

set( ${target}_SRCS
  main.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
