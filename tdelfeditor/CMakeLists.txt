#################################################
#
#  (C) 2011 Timothy Pearson
#  kb9vqf (AT) pearsoncomputing.net
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${LIBR_INCLUDEDIR}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LIBR_LIBDIR}
)


##### tdelfeditor ################################

tde_add_executable( tdelfeditor
  SOURCES tdelfeditor.cpp
  DESTINATION ${BIN_INSTALL_DIR}
  LINK ${TQT_LIBRARIES} ${LIBR_LIBRARIES} tdecore-shared
)
