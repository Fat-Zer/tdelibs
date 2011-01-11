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
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES runner.h tester.h module.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kunittest )


##### libkunittest ##############################

set( target kunittest )

set( ${target}_SRCS
  runner.cpp tester.cpp
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 1.0.0
  LINK kdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}
)


##### kunittestmodrunner ########################

tde_add_executable( kunittestmodrunner
  SOURCES modrunner.cpp
  LINK kunittest-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
