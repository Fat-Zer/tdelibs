#################################################
#
#  (C) 2012 Trinity Project
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/update-entities.sh ${CMAKE_CURRENT_BINARY_DIR}/update-entities IMMEDIATE @ONLY )

set( UPDATE_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/update-entities" )
set( TDEVERSION_FILE "${CMAKE_SOURCE_DIR}/tdecore/tdeversion.h" )
set( ENTITIES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/customization/entities/general.entities" )

if( NOT EXISTS ${UPDATE_SCRIPT} )
  tde_message_fatal( "${UPDATE_SCRIPT} not found!\n Check your sources." )
endif( )
if( NOT EXISTS ${TDEVERSION_FILE} )
  tde_message_fatal( "${TDEVERSION_FILE} not found!\n Check your sources." )
endif( )
if( NOT EXISTS ${ENTITIES_FILE} )
  tde_message_fatal( "${ENTITIES_FILE} not found!\n Check your sources." )
endif( )

execute_process( COMMAND chmod +x ${UPDATE_SCRIPT} )
execute_process(
  COMMAND ${UPDATE_SCRIPT}
  RESULT_VARIABLE _result
  OUTPUT_STRIP_TRAILING_WHITESPACE )
if( _result )
  tde_message_fatal( "Unable to update ${ENTITIES_FILE}!\n " )
else( )
  message( STATUS "Updated as follows:" )
  execute_process( COMMAND echo )
  execute_process( COMMAND tail -n3 ${ENTITIES_FILE} )
  execute_process( COMMAND echo )
endif( )
