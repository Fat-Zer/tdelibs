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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdefile
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    resourcenet.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kabc )


##### other data ################################

install( FILES net.desktop DESTINATION ${SERVICES_INSTALL_DIR}/tderesources/kabc )


##### kabc_net (library) ########################

set( target kabc_net )

set( ${target}_SRCS
  resourcenet.cpp resourcenetconfig.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdeabc-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kabc_net (module) #########################

set( target kabc_net )

set( ${target}_SRCS
  resourcenetplugin.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kabc_net-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
