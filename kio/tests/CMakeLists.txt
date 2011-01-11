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
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kio/kio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### ktradertest ###############################

set( target ktradertest )

set( ${target}_SRCS
  ktradertest.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
