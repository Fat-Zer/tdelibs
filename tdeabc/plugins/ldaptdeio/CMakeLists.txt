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
  ${CMAKE_SOURCE_DIR}/tdeio/tdefiles
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    resourceldaptdeio.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kabc )


##### other data ################################

install( FILES ldaptdeio.desktop DESTINATION ${SERVICES_INSTALL_DIR}/tderesources/kabc )


##### kabc_ldaptdeio ##############################

set( target kabc_ldaptdeio )

set( ${target}_SRCS
  resourceldaptdeio.cpp resourceldaptdeioconfig.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdeabc-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kabc_ldaptdeio ##############################

set( target kabc_ldaptdeio )

set( ${target}_SRCS
  resourceldaptdeioplugin.cpp
)

tde_add_kpart( ${target}
  SOURCES ${${target}_SRCS}
  LINK kabc_ldaptdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
