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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
  ${CMAKE_SOURCE_DIR}/kio/kfile
)


##### headers ###################################

install( FILES
    kbookmark.h kbookmarkbar.h kbookmarkdrag.h kbookmarkexporter.h
    kbookmarkimporter.h kbookmarkmanager.h kbookmarkmenu.h
    kbookmarknotifier.h kbookmarkimporter_crash.h
    kbookmarkimporter_opera.h kbookmarkimporter_ie.h
    kbookmarkimporter_ns.h kbookmarkimporter_kde1.h
    kbookmarkdombuilder.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### kbookmarks ################################

set( target kbookmarks )

set( ${target}_SRCS
  kbookmark.cc kbookmarkbar.cc kbookmarkdrag.cc kbookmarkexporter.cc
  kbookmarkimporter.cc kbookmarkmanager.cc kbookmarkmenu.cc
  kbookmarkimporter_crash.cc kbookmarkimporter_opera.cc
  kbookmarkimporter_ie.cc kbookmarkimporter_ns.cc
  kbookmarkimporter_kde1.cc kbookmarkdombuilder.cc
  kbookmarkmanager.skel kbookmarknotifier.skel
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  DEPENDENCIES dcopidl
)
