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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

list( REMOVE_ITEM CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined" )

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    kscreensaver.h kscreensaver_vroot.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### kscreensaver ##############################

set( target kscreensaver )

set( ${target}_SRCS
  main.cpp kscreensaver.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 4.2.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
