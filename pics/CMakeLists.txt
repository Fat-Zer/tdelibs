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

add_subdirectory( hicolor )
add_subdirectory( crystalsvg )
add_subdirectory( emoticons )


# ksvgtopng is compiled only if we have libart support
if( WITH_LIBART )

add_definitions(
  ${TQT_CFLAGS_OTHER}
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${LIBART_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdefx
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore/svgicons
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

tde_install_symlink( crystalsvg ${SHARE_INSTALL_DIR}/icons/default.kde )


##### ksvgtopng #################################

set( target ksvgtopng )

set( ${target}_SRCS
  ksvgtopng.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kdesvgicons-static kdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

endif( WITH_LIBART )
