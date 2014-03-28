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
  ${TDECORE_LIBRARY_DIRS}
  ${LIBR_LIBDIR}
)


##### tdelfeditor ################################

tde_add_executable( tdelfeditor
  SOURCES tdelfeditor.cpp
  DESTINATION ${BIN_INSTALL_DIR}
  LINK ${TQT_LIBRARIES} ${LIBR_LIBRARIES} tdecore-shared
)


##### embed scm data into important libraries ####

# look for SCM data if present
if( EXISTS "${CMAKE_SOURCE_DIR}/.tdescmmodule" )
  file( STRINGS "${CMAKE_SOURCE_DIR}/.tdescmmodule" TDE_SCM_MODULE_NAME )
endif( EXISTS "${CMAKE_SOURCE_DIR}/.tdescmmodule" )
if( EXISTS "${CMAKE_SOURCE_DIR}/.tdescmrevision" )
  file( STRINGS "${CMAKE_SOURCE_DIR}/.tdescmrevision" TDE_SCM_MODULE_REVISION )
endif( EXISTS "${CMAKE_SOURCE_DIR}/.tdescmrevision" )

tde_curdatetime( _datetime )
set( ELF_EMBEDDING_METADATA "\"\" \"\" \"\" \"\" \"\" \"Trinity Desktop Environment\" \"\" \"\" \"${_datetime}\" \"x-sharedlib\" \"${TDE_SCM_MODULE_NAME}\" \"${TDE_SCM_MODULE_REVISION}\" \"\"" )
separate_arguments( ELF_EMBEDDING_METADATA )

if( NOT "${TDE_SCM_MODULE_NAME}" STREQUAL "" )
  if( NOT "${TDE_SCM_MODULE_REVISION}" STREQUAL "" )
    add_custom_target(
      # embed name and metadata
      update_libDCOP_metadata ALL
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -m ${CMAKE_BINARY_DIR}/dcop/libDCOP.so ${ELF_EMBEDDING_METADATA} || true
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -e ${CMAKE_BINARY_DIR}/dcop/libDCOP.so || true
      DEPENDS tdelfeditor
      DEPENDS DCOP-shared
      COMMENT "Storing SCM metadata in dcop/libDCOP.so"
    )

    add_custom_target(
      # embed name and metadata
      update_libtdecore_metadata ALL
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -m ${CMAKE_BINARY_DIR}/tdecore/libtdecore.so ${ELF_EMBEDDING_METADATA} || true
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -e ${CMAKE_BINARY_DIR}/tdecore/libtdecore.so || true
      DEPENDS tdelfeditor
      DEPENDS tdecore-shared
      COMMENT "Storing SCM metadata in tdecore/libtdecore.so"
    )

    add_custom_target(
      # embed name and metadata
      update_libtdeio_metadata ALL
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -m ${CMAKE_BINARY_DIR}/tdeio/libtdeio.so ${ELF_EMBEDDING_METADATA} || true
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -e ${CMAKE_BINARY_DIR}/tdeio/libtdeio.so || true
      DEPENDS tdelfeditor
      DEPENDS tdeio-shared
      COMMENT "Storing SCM metadata in tdeui/libtdeio.so"
    )

    add_custom_target(
      # embed name and metadata
      update_libtdeui_metadata ALL
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -m ${CMAKE_BINARY_DIR}/tdeui/libtdeui.so ${ELF_EMBEDDING_METADATA} || true
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -e ${CMAKE_BINARY_DIR}/tdeui/libtdeui.so || true
      DEPENDS tdelfeditor
      DEPENDS tdeui-shared
      COMMENT "Storing SCM metadata in tdeui/libtdeui.so"
    )

    add_custom_target(
      # embed name and metadata
      update_libtdeutils_metadata ALL
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -m ${CMAKE_BINARY_DIR}/tdeutils/libtdeutils.so ${ELF_EMBEDDING_METADATA} || true
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -e ${CMAKE_BINARY_DIR}/tdeutils/libtdeutils.so || true
      DEPENDS tdelfeditor
      DEPENDS tdeutils-shared
      COMMENT "Storing SCM metadata in tdeutils/libtdeutils.so"
    )

    add_custom_target(
      # embed name and metadata
      update_libtdeprint_metadata ALL
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -m ${CMAKE_BINARY_DIR}/tdeprint/libtdeprint.so ${ELF_EMBEDDING_METADATA} || true
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -e ${CMAKE_BINARY_DIR}/tdeprint/libtdeprint.so || true
      DEPENDS tdelfeditor
      DEPENDS tdeprint-shared
      COMMENT "Storing SCM metadata in tdeprint/libtdeprint.so"
    )

    add_custom_target(
      # embed name and metadata
      update_libtdehtml_metadata ALL
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -m ${CMAKE_BINARY_DIR}/tdehtml/libtdehtml.so ${ELF_EMBEDDING_METADATA} || true
      COMMAND ${CMAKE_BINARY_DIR}/tdelfeditor/tdelfeditor -e ${CMAKE_BINARY_DIR}/tdehtml/libtdehtml.so || true
      DEPENDS tdelfeditor
      DEPENDS tdehtml-shared
      COMMENT "Storing SCM metadata in tdehtml/libtdehtml.so"
    )
  endif( NOT "${TDE_SCM_MODULE_REVISION}" STREQUAL "" )
endif( NOT "${TDE_SCM_MODULE_NAME}" STREQUAL "" )