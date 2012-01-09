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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
  ${FREETYPE_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


#### tdeinit ####################################

set( target tdeinit )

set( ${target}_SRCS
  kinit.cpp setproctitle.cpp
)

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK ltdlc-static kparts-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdeinit_wrapper ############################

set( target tdeinit_wrapper )

tde_add_executable( ${target}
  SOURCES wrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### kshell #####################################

set( target kshell )

tde_add_executable( ${target}
  SOURCES shell.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### tdeinit_shutdown ###########################

set( target tdeinit_shutdown )

tde_add_executable( ${target}
  SOURCES wrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### lnusertemp #################################

set( target lnusertemp )

tde_add_executable( ${target}
  SOURCES lnusertemp.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### kwrapper ###################################

set( target kwrapper )

tde_add_executable( ${target}
  SOURCES kwrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### kioslave ###################################

set( target kioslave )

tde_add_executable( ${target} AUTOMOC
  SOURCES kioslave.cpp
  LINK ltdlc-static kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


#### kstartupconfig #############################

set( target kstartupconfig )

tde_add_executable( ${target} AUTOMOC
  SOURCES kstartupconfig.cpp
  LINK tdefakes-shared ${TQT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


#### kdostartupconfig ###########################

set( target kdostartupconfig )

tde_add_executable( ${target} AUTOMOC
  SOURCES kdostartupconfig.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


#### start_tdeinit ##############################

set( target start_tdeinit )

if( TDEINIT_SETUID )
  set( _setuid SETUID )
endif( TDEINIT_SETUID )

tde_add_executable( ${target} ${_setuid}
  SOURCES start_tdeinit.c
  DESTINATION ${BIN_INSTALL_DIR}
)

set_target_properties( ${target} PROPERTIES COMPILE_FLAGS -DEXECUTE=\\"${BIN_INSTALL_DIR}/tdeinit\\" )


#### start_tdeinit_wrapper ######################

set( target start_tdeinit_wrapper )

tde_add_executable( ${target}
  SOURCES start_tdeinit_wrapper.c
  DESTINATION ${BIN_INSTALL_DIR}
)


#### klauncher ##################################

set( target klauncher )

set( ${target}_SRCS
  klauncher.cpp klauncher_main.cpp autostart.cpp
)

tde_add_tdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kio-shared
)
