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
  ${CMAKE_SOURCE_DIR}
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
    downloaddialog.h engine.h entry.h knewstuffgeneric.h
    knewstuff.h providerdialog.h provider.h uploaddialog.h
    knewstuffbutton.h knewstuffsecure.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/knewstuff )


##### other data ################################

install( FILES types DESTINATION ${DATA_INSTALL_DIR}/knewstuff )
install( FILES khotnewstuffrc DESTINATION ${CONFIG_INSTALL_DIR} )
tde_install_icons( )


##### knewstuff #################################

set( target knewstuff )

set( ${target}_SRCS
  engine.cpp entry.cpp downloaddialog.cpp uploaddialog.cpp
  providerdialog.cpp provider.cpp knewstuff.cpp security.cpp
  knewstuffgeneric.cpp knewstuffbutton.cpp knewstuffsecure.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK kio-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### khotnewstuff ##############################

set( target khotnewstuff )

set( ${target}_SRCS
  khotnewstuff.cpp
)

tde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK knewstuff-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
