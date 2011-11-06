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
  ${ARTS_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${ARTS_LIBRARY_DIRS}
)

# pkg-config stripping -I/usr/include
if( NOT ARTS_INCLUDE_DIRS )
  set( ARTS_INCLUDE_DIRS "/usr/include" )
endif()


##### headers ###################################

install( FILES
    kplayobject.h kplayobjectfactory.h kartsfloatwatch.h
    ${CMAKE_CURRENT_BINARY_DIR}/artskde.h kartsdispatcher.h
    kaudiorecordstream.h kaudioplaystream.h kartsserver.h
    kvideowidget.h kaudiomanagerplay.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/arts )


##### artskde ###################################

set( target artskde )

set( ${target}_SRCS
  artskde.cc kioinputstream_impl.cpp kplayobject.cc
  kplayobjectfactory.cc kartsfloatwatch.cpp kartsdispatcher.cc
  kaudiorecordstream.cpp kaudioplaystream.cpp kartsserver.cpp
  kdatarequest_impl.cpp kaudioconverter.cc kvideowidget.cpp
  kplayobjectcreator.cc kaudiomanagerplay.cpp
)

add_custom_command(
  OUTPUT artskde.cc artskde.h
  COMMAND ${ARTS_MCOPIDL_EXECUTABLE}
    -I${ARTS_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/artskde.idl
  DEPENDS artskde.idl
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.2.0
  LINK kio-shared mcop artsflow_idl kmedia2_idl soundserver_idl artsflow qtmcop
  DESTINATION ${LIB_INSTALL_DIR}
)
