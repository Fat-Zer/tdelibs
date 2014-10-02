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
  -D_LARGEFILE64_SOURCE=1
)

add_subdirectory( kssl )
add_subdirectory( tdeio )
add_subdirectory( bookmarks )
add_subdirectory( tdefile )
add_subdirectory( pics )
add_subdirectory( tdeioexec )
add_subdirectory( httpfilter )
add_subdirectory( misc )
add_subdirectory( kpasswdserver )
add_subdirectory( tests )

include_directories(
  ${TQT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LIBR_LIBDIR}
)


##### other data ################################

install( FILES
    application.desktop kurifilterplugin.desktop
    kcomprfilter.desktop kscan.desktop kdatatool.desktop
    tdefileplugin.desktop tdecmodule.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR} )

install( FILES tdeioslave.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( PROGRAMS useragent.pl proxytype.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES renamedlgplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install( FILES kpasswdserver.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )
install( FILES data.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


if( HAVE_ELFICON )
  set( ELFICON_STATIC_LIB tdelficon-static )
endif( HAVE_ELFICON )

if( HAVE_NON_POSIX_ACL_EXTENSIONS )
  set( ACL_LIBRARIES acl )
endif( HAVE_NON_POSIX_ACL_EXTENSIONS )


##### libtdeio ####################################

set( target tdeio )

configure_file( ${CMAKE_SOURCE_DIR}/cmake/modules/template_dummy_cpp.cmake dummy.cpp COPYONLY )

tde_add_library( ${target} SHARED
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
  VERSION 14.0.0
  EMBED kssl-static tdeiocore-static tdesycoca-static kbookmarks-static tdefile-static ${ELFICON_STATIC_LIB}
  LINK ltdlc-static tdeui-shared tdesu-shared tdewalletclient-shared ${LIBR_LIBRARIES} ${ACL_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
