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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${XRENDER_LIBRARY_DIRS}
)


##### tdetradertest ###############################

set( target tdetradertest )

set( ${target}_SRCS
  tdetradertest.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK tdeio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
