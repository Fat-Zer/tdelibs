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

add_definitions(
  ${TQT_CFLAGS_OTHER}
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  #${CMAKE_CURRENT_SOURCE_DIR}
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
    part.h plugin.h partmanager.h mainwindow.h dockmainwindow.h
    event.h browserextension.h factory.h historyprovider.h
    browserinterface.h genericfactory.h componentfactory.h
    browserrun.h statusbarextension.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kparts )


##### other data ################################

install( FILES
    kpart.desktop krop.desktop krwp.desktop browserview.desktop
  DESTINATION ${SHARE_INSTALL_DIR}/servicetypes )


#### kparts #####################################

set( target kparts )

set( ${target}_SRCS
  part.cpp plugin.cpp partmanager.cpp mainwindow.cpp
  dockmainwindow.cpp event.cpp browserextension.cpp
  factory.cpp  historyprovider.cpp browserinterface.cpp
  browserrun.cpp statusbarextension.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 2.1.0
  LINK kio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
