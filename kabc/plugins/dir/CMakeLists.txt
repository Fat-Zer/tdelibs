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
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kabc
  ${CMAKE_SOURCE_DIR}/kabc

  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
  ${CMAKE_SOURCE_DIR}/kio/kfile
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    resourcedir.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kabc )


##### other data ################################

install( FILES dir.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kresources/kabc )


##### kabc_dir (library) ########################

set( target kabc_dir )

set( ${target}_SRCS
  resourcedir.cpp resourcedirconfig.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK kabc-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kabc_dir (module) #########################

set( target kabc_dir )

set( ${target}_SRCS
  resourcedirplugin.cpp
)

tde_add_kpart( ${target}
  SOURCES ${${target}_SRCS}
  LINK kabc_dir-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
