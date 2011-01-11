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
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_BINARY_DIR}/kjs
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore/network
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
  ${CMAKE_SOURCE_DIR}/kded
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

##### other data ################################

install(FILES proxyscout.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install(FILES eventsrc DESTINATION ${DATA_INSTALL_DIR}/proxyscout )


##### kded_proxyscout ###########################

set( target kded_proxyscout )

set( ${target}_SRCS
  proxyscout.cpp proxyscout.skel script.cpp
  downloader.cpp discovery.cpp
)

set( ${target}_SKELS
  proxyscout.h
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeinit_kded-shared kjs-shared ${RESOLV_LIBRARIES}
  DEPENDENCIES dcopidl
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kpac_dhcp_helper ##########################

set( target kpac_dhcp_helper )

# FIXME on Gentoo this binary is not suided
tde_add_executable( ${target} SETUID
  SOURCES kpac_dhcp_helper.c
  DESTINATION ${BIN_INSTALL_DIR}
)
