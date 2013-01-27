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

add_subdirectory( libisofs )


include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( FILES iso.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES isoservice.desktop DESTINATION ${DATA_INSTALL_DIR}/konqueror/servicemenus )
install( FILES kio_isorc DESTINATION ${CONFIG_INSTALL_DIR} )
install( FILES kio_iso.desktop DESTINATION ${APPS_INSTALL_DIR} )


##### kio_iso ###################################

set( target kio_iso )

set( ${target}_SRCS
  kisodirectory.cpp kisofile.cpp qfilehack.cpp
  kiso.cpp iso.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK isofs-static tdeio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
