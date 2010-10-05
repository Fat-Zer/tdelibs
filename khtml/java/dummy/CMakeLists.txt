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

##### kjava #####################################

set( target kjava )

configure_file( ${CMAKE_SOURCE_DIR}/cmake/modules/template_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_library( ${target} SHARED
  SOURCES dummy.cpp
  VERSION 1.0.0
  DESTINATION ${LIB_INSTALL_DIR}
)
