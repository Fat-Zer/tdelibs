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
  -UQT_NO_ASCII_CAST
  -DQT_PLUGIN
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdefx
  ${CMAKE_SOURCE_DIR}/kdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### genembed ##################################

set( target genembed )

set( ${target}_SRCS
  genembed.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kdefx-shared
)


##### pixmaps.keramik ###########################

file(GLOB _pics "${CMAKE_CURRENT_SOURCE_DIR}/pics/*.png" )

add_custom_command(
  OUTPUT pixmaps.keramik keramikrc.h
  COMMAND ./genembed ${_pics} 2>/dev/null > pixmaps.keramik
  DEPENDS genembed )


##### keramik ###################################

set( target keramik )

set( ${target}_SRCS
  keramik.cpp pixmaploader.cpp gradients.cpp colorutil.cpp
)

set_source_files_properties( pixmaploader.cpp PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/pixmaps.keramik )

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdefx-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/styles
)
