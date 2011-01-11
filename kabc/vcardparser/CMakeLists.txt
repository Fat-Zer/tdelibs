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
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    vcard.h vcardline.h vcardparser.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/kabc )


##### vcards ####################################

set( target vcards )

set( ${target}_SRCS
  vcard.cpp vcardline.cpp vcardparser.cpp
)

tde_add_library( ${target} STATIC_PIC
  SOURCES ${${target}_SRCS}
)
