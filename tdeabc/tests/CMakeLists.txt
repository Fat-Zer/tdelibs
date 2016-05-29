#################################################
#
#  (C) 2016 Slávek Banko
#  slavek.banko (AT) axis.cz
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${CMAKE_BINARY_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/tdeabc
  ${CMAKE_SOURCE_DIR}/kab
  ${CMAKE_SOURCE_DIR}/tdeabc/plugins/sql
  ${CMAKE_SOURCE_DIR}/tdeabc/vcardparser
  ${CMAKE_SOURCE_DIR}/tdeabc/vcard/include
  ${CMAKE_SOURCE_DIR}/tdeabc/vcard/include/generated
  ${CMAKE_SOURCE_DIR}/tdeabc/vcardparser
)

link_directories(
  ${TDECORE_LIBRARY_DIRS}
)

set( TDEABC_TESTS_LINK
  ${TQT_LIBRARIES}
  DCOP-shared
  tdecore-shared
  tdeui-shared
  tdefx-shared
  tdeio-shared
  tdetexteditor-shared
  tdeabc-shared
)


##### test programs ##############################

tde_add_executable( testlock
  SOURCES testlock.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testldapclient
  SOURCES testldapclient.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testkabc
  SOURCES testkabc.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testaddressee
  SOURCES testaddressee.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testaddresseelist
  SOURCES testaddresseelist.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testaddressfmt
  SOURCES testaddressfmt.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testkabcdlg
  SOURCES testkabcdlg.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testdistlist
  SOURCES testdistlist.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( testaddresslineedit
  SOURCES testaddresslineedit.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK}
)

tde_add_executable( bigread
  SOURCES bigread.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK} tdeabc_file-shared
)

tde_add_executable( bigwrite
  SOURCES bigwrite.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK} tdeabc_file-shared
)

# Plugin ResourceSql is not built == can not be tested
#
# tde_add_executable( testdb
#   SOURCES testdb.cpp AUTOMOC
#   LINK ${TDEABC_TESTS_LINK} tdeabc_file-shared
# )

tde_add_executable( kabcargl
  SOURCES kabcargl.cpp AUTOMOC
  LINK ${TDEABC_TESTS_LINK} tdeabc_file-shared
)
