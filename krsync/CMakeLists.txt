#################################################
#
#  (C) 2011 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing.net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kdefx
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    krsync.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libkrsync )


##### krandr ####################################

set( target krsync )

set( ${target}_SRCS
  krsync.cpp rsyncconfigdialog.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 0.0.1
  LINK kdeui-shared kio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
