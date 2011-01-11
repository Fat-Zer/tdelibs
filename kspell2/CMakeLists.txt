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

add_subdirectory( ui )
add_subdirectory( plugins )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kutils
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    settings.h filter.h dictionary.h broker.h
    backgroundchecker.h defaultdictionary.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kspell2 )


##### other data ################################

install( FILES kspellclient.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


#### kspell2base ################################

set( target kspell2base )

set( ${target}_SRCS
  settings.cpp broker.cpp client.cpp filter.cpp
  backgroundchecker.cpp backgroundthread.cpp
  backgroundengine.cpp defaultdictionary.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
