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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### libtdewalletbackend #########################

set( target tdewalletbackend )

set( ${target}_SRCS
  blockcipher.cc blowfish.cc
  cbc.cc sha1.cc tdewalletentry.cc
  tdewalletbackend.cc
)

tde_add_library( ${target} SHARED
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK tdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
