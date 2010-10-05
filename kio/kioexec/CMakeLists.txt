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
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdeui
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### kioexec ###################################

set( target kioexec )

set( ${target}_SRCS
  main.cpp
)

tde_add_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

# FIXME need a review
# create symlink to kfmexec
file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/make_kfmexec_symlink.cmake "exec_program(${CMAKE_COMMAND} ARGS -E create_symlink ${target} \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${BIN_INSTALL_DIR}/kfmexec\")\n")
set_target_properties( ${target} PROPERTIES POST_INSTALL_SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/make_kfmexec_symlink.cmake)
