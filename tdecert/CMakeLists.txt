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
  ${CMAKE_BINARY_DIR}/kio/kssl
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
  ${CMAKE_SOURCE_DIR}/kio/kfile
  ${CMAKE_SOURCE_DIR}/kio/kssl
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES tdecertpart.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES tdecertpart.rc DESTINATION ${DATA_INSTALL_DIR}/tdecertpart )


##### libtdecertpart ##############################

set( target libtdecertpart )

set( ${target}_SRCS
  tdecertpart.cc
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kio-shared tdeparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
