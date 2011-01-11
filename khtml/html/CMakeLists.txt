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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/khtml
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
  ${CMAKE_SOURCE_DIR}/kio/kfile
  ${CMAKE_SOURCE_DIR}/kio/kssl
  ${CMAKE_SOURCE_DIR}/kutils
  ${CMAKE_SOURCE_DIR}/kwallet/client
)

##### khtmlhtml-static ###########################

set( target khtmlhtml )

set( ${target}_SRCS
  htmlparser.cpp htmltokenizer.cpp dtd.cpp html_headimpl.cpp
  html_blockimpl.cpp html_elementimpl.cpp html_inlineimpl.cpp
  html_documentimpl.cpp html_baseimpl.cpp html_imageimpl.cpp
  html_listimpl.cpp html_miscimpl.cpp html_formimpl.cpp
  html_objectimpl.cpp html_tableimpl.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
