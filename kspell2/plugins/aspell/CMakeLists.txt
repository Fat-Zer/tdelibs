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
  ${CMAKE_SOURCE_DIR}/kspell2
  ${CMAKE_SOURCE_DIR}/kdecore
  ${ASPELL_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${ASPELL_LIBRARY_DIRS}
)


##### other data ################################

install( FILES kspell_aspell.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


#### kspell_aspell ##############################

set( target kspell_aspell )

set( ${target}_SRCS
  kspell_aspellclient.cpp kspell_aspelldict.cpp
)

tde_add_kpart( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kspell2-shared ${ASPELL_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
