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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdewallet/client
  ${CMAKE_SOURCE_DIR}/tdewallet/backend
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

tde_add_check_executable( backendtest AUTOMOC
    LINK tdewalletbackend-shared tdewalletclient-shared TEST)
tde_add_check_executable( testbf AUTOMOC
    LINK tdewalletbackend-shared tdewalletclient-shared TEST )
tde_add_check_executable( testsha AUTOMOC
    LINK tdewalletbackend-shared tdewalletclient-shared TEST )
