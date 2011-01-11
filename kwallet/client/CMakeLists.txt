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
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/kdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    kwallet.h kwallettypes.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### libkwalletclient ##############################

set( target kwalletclient )

set( ${target}_SRCS
  kwallet.skel kwallet.cc
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.1
  LINK kdecore-shared
  DEPENDENCIES dcopidl
  DESTINATION ${LIB_INSTALL_DIR}
)
