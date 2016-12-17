#################################################
#
#  (C) 2016 Alexander Golubev
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### checks ####################################

tde_add_check_executable( dcop_test AUTOMOC
  SOURCES test.cpp ${CMAKE_CURRENT_BINARY_DIR}/test.skel ${CMAKE_CURRENT_BINARY_DIR}/test.h
  LINK tdecore-shared
)

tde_add_check_executable( driver AUTOMOC
  SOURCES driver.cpp ${CMAKE_CURRENT_BINARY_DIR}/test.stub ${CMAKE_CURRENT_BINARY_DIR}/test.h
  LINK tdecore-shared
)

add_test( NAME "dcop/dcop-tests"
    COMMAND sh -x "${CMAKE_CURRENT_SOURCE_DIR}/run-tests.sh"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" )


##### test.h and files ##########################

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.h ${CMAKE_CURRENT_BINARY_DIR}/shell.generated
  COMMAND perl "${CMAKE_CURRENT_SOURCE_DIR}/generate.pl"
    <"${CMAKE_CURRENT_SOURCE_DIR}/testcases"
  DEPENDS testcases generate.pl
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# a hack because *.skel and *.stub files have to be located the same place with the *.h
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.skel
  COMMAND ${CMAKE_COMMAND} -E copy_file
    ${CMAKE_CURRENT_SOURCE_DIR}/test.skel ${CMAKE_CURRENT_BINARY_DIR}/
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.stub
  COMMAND ${CMAKE_COMMAND} -E copy_file
    ${CMAKE_CURRENT_SOURCE_DIR}/test.stub ${CMAKE_CURRENT_BINARY_DIR}/
)
