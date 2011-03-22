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
)

##### kjava #####################################

configure_file( ${CMAKE_SOURCE_DIR}/cmake/modules/template_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_library( kjava SHARED
  SOURCES dummy.cpp
  LINK ${TQT_LIBRARIES}
  VERSION 1.0.0
  DESTINATION ${LIB_INSTALL_DIR}
)
