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
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kstyle_highcontrast_config ################

set( target kstyle_highcontrast_config )

set( ${target}_SRCS
  highcontrastconfig.cpp
)

tde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
