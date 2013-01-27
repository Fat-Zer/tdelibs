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

add_subdirectory( malloc )
add_subdirectory( network )
add_subdirectory( tdeconfig_compiler )
add_subdirectory( hwlibdata )
add_subdirectory( hwlibdaemons )

if( NOT DBUS_SYSTEM_BUS )
  set( DBUS_SYSTEM_BUS "unix:path=/var/run/dbus/system_bus_socket" CACHE INTERNAL "" FORCE )
endif()

if( WITH_UPOWER )
  add_definitions( -DWITH_UPOWER )
endif( )

if( WITH_LIBART )
  add_subdirectory( svgicons )
  set( KDESVGICONS kdesvgicons-static )
endif( WITH_LIBART )

if( WITH_NETWORK_MANAGER_BACKEND )
  add_subdirectory( networkbackends/network-manager/dbus )
  set ( TDENM_INCLUDE_DIRECTORIES
    ${DBUS_TQT_INCLUDE_DIRS}
    ${NM_UTIL_INCLUDE_DIRS}
    "${CMAKE_BINARY_DIR}/tdecore/networkbackends/network-manager/dbus"
  )
  set ( TDENM_LIBRARIES
    tdenm_dbus-static
    ${DBUS_TQT_LIBRARIES}
  )
  set ( TDENM_BACKEND_SOURCES networkbackends/network-manager/network-manager.cpp )
endif( WITH_NETWORK_MANAGER_BACKEND )


include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/network
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/libltdl
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio/tdeio
  ${LIBART_INCLUDE_DIRS}
  ${LIBIDN_INCLUDE_DIRS}
  ${GAMIN_INCLUDEDIR}
  ${TDENM_INCLUDE_DIRECTORIES}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${LIBIDN_LIBRARY_DIRS}
  ${GAMIN_LIBDIR}
)

##### headers ###################################

install( FILES
    tdeconfig.h tdeconfigskeleton.h tdeconfigdata.h ksimpleconfig.h
    tdeconfigdialogmanager.h tdeconfigbase.h kdesktopfile.h
    kurl.h ksock.h kaboutdata.h kcmdlineargs.h tdeconfigbackend.h
    kapp.h kapplication.h kuniqueapp.h kuniqueapplication.h
    kcharsets.h tdeversion.h kpty.h kprocess.h kprocctrl.h
    klocale.h kicontheme.h kiconloader.h kdebug.h twinmodule.h
    twin.h krootprop.h kshortcut.h kkeynative.h kaccel.h
    kglobalaccel.h kstdaccel.h kshortcutlist.h kcatalogue.h
    kregexp.h kcompletion.h kstringhandler.h kstddirs.h
    kstandarddirs.h kglobal.h kglobalsettings.h ksharedptr.h
    kallocator.h kvmallocator.h kcrash.h krfcdate.h kinstance.h
    kpalette.h kipc.h klibloader.h ktempfile.h ksavefile.h
    krandomsequence.h knotifyclient.h kiconeffect.h kaudioplayer.h
    kdcoppropertyproxy.h netwm.h kaccelmanager.h netwm_def.h
    kpixmapprovider.h kunload.h kstaticdeleter.h kextsock.h
    kextendedsocket.h ksockaddr.h kprocio.h kasyncio.h kbufferedio.h
    kurldrag.h kmimesourcefactory.h kmdcodec.h ksocks.h tdesycoca.h
    tdesycocaentry.h tdesycocatype.h kxmessages.h kstartupinfo.h
    klargefile.h tdemultipledrag.h kgenericfactory.h kgenericfactory.tcc
    ktypelist.h ksortablevaluelist.h kdebugclasses.h kclipboard.h
    kcalendarsystem.h kcalendarsystemfactory.h kmacroexpander.h
    kmanagerselection.h kmountpoint.h kuser.h klockfile.h
    kidna.h ktempdir.h kshell.h fixx11h.h kxerrorhandler.h
    tdelibs_export.h kde_file.h ktimezones.h tdehardwaredevices.h
    tdenetworkconnections.h
    ${CMAKE_CURRENT_BINARY_DIR}/kdemacros.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other data ################################

install( FILES all_languages.desktop
  DESTINATION ${LOCALE_INSTALL_DIR} RENAME all_languages )

install( FILES 40.colors Web.colors Royal.colors Rainbow.colors
  DESTINATION ${CONFIG_INSTALL_DIR}/colors )

install( FILES kdebug.areas kdebugrc language.codes
  DESTINATION ${CONFIG_INSTALL_DIR} )

install( FILES kmdcodec_compat.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeio RENAME kmdcodec.h )

install( FILES eventsrc
  DESTINATION ${DATA_INSTALL_DIR}/knotify )

##### libtdecore ################################

set( target tdecore )

set( ${target}_SRCS
  libintl.cpp kapplication.cpp kdebug.cpp netwm.cpp
  tdeconfigbase.cpp tdeconfig.cpp ksimpleconfig.cpp tdeconfigbackend.cpp
  kmanagerselection.cpp kdesktopfile.cpp kstandarddirs.cpp
  ksock.cpp kpty.cpp kprocess.cpp kprocctrl.cpp klocale.cpp
  krfcdate.cpp kiconeffect.cpp kicontheme.cpp kiconloader.cpp
  twin.cpp twinmodule.cpp krootprop.cpp kcharsets.cpp
  kckey.cpp kshortcut.cpp kkeynative_x11.cpp kkeyserver_x11.cpp
  kaccelaction.cpp kshortcutmenu.cpp kaccelbase.cpp kaccel.cpp
  kglobalaccel_x11.cpp kglobalaccel.cpp kstdaccel.cpp kshortcutlist.cpp
  kcrash.cpp kurl.cpp kregexp.cpp kglobal.cpp kglobalsettings.cpp
  kallocator.cpp kvmallocator.cpp kmimesourcefactory.cpp
  kinstance.cpp kpalette.cpp kipc.cpp klibloader.cpp ktempfile.cpp
  kuniqueapplication.cpp kaccelmanager.cpp ksavefile.cpp
  krandomsequence.cpp kstringhandler.cpp kcompletion.cpp
  kcmdlineargs.cpp kaboutdata.cpp kcompletionbase.cpp knotifyclient.cpp
  kaudioplayer.cpp kdcoppropertyproxy.cpp ksockaddr.cpp
  kextsock.cpp netsupp.cpp kprocio.cpp kbufferedio.cpp
  kpixmapprovider.cpp kurldrag.cpp kmdcodec.cpp ksocks.cpp
  fakes.c vsnprintf.c tdesycoca.cpp tdesycocadict.cpp tdesycoca.skel
  tdesycocafactory.cpp kxmessages.cpp kstartupinfo.cpp
  kcatalogue.cpp kasyncio.cpp tdemultipledrag.cpp kstaticdeleter.cpp
  kappdcopiface.cpp kappdcopiface.skel kclipboard.cpp
  kcheckaccelerators.cpp tdeversion.cpp kdebugdcopiface.cpp
  kdebugdcopiface.skel kcalendarsystem.cpp kcalendarsystemgregorian.cpp
  kcalendarsystemhijri.cpp kcalendarsystemhebrew.cpp
  kcalendarsystemfactory.cpp kmacroexpander.cpp kidna.cpp
  ktempdir.cpp kshell.cpp kmountpoint.cpp kcalendarsystemjalali.cpp
  kprotocolinfo_tdecore.cpp kprotocolinfofactory.cpp kxerrorhandler.cpp
  kuser.cpp tdeconfigskeleton.cpp tdeconfigdialogmanager.cpp klockfile.cpp
  kqiodevicegzip_p.cpp ktimezones.cpp tdehardwaredevices.cpp tdenetworkconnections.cpp
  ksimpledirwatch.cpp
  ${TDENM_BACKEND_SOURCES}
)

tde_add_library( ${target} SHARED AUTOMOC
  SOURCES ${${target}_SRCS}
  VERSION 4.2.0
  EMBED tdecorenetwork-static
  LINK ltdlc-static ${KDESVGICONS} DCOP-shared tdefx-shared ${ZLIB_LIBRARIES} ${LIBIDN_LIBRARIES} ${XCOMPOSITE_LIBRARIES} ICE SM udev ${GAMIN_LIBRARIES} ${TDENM_LIBRARIES}
  DEPENDENCIES dcopidl dcopidl2cpp
  DESTINATION ${LIB_INSTALL_DIR}
)


##### tdefakes_nonpic ###########################

set( target tdefakes_nonpic )

set( ${target}_SRCS
  fakes.c vsnprintf.c
)

tde_add_library( ${target} STATIC
  SOURCES ${${target}_SRCS}
  DESTINATION ${LIB_INSTALL_DIR}
)

##### tdefakes_pic ##############################

set( target tdefakes_pic )

set( ${target}_SRCS
  fakes.c vsnprintf.c
)

tde_add_library( ${target} STATIC_PIC
  SOURCES ${${target}_SRCS}
  DESTINATION ${LIB_INSTALL_DIR}
)


##### libtdefakes ###############################

set( target tdefakes )

set( ${target}_SRCS
  fakes.c vsnprintf.c
)

tde_add_library( ${target} SHARED
  SOURCES ${${target}_SRCS}
  VERSION 4.2.0
  DESTINATION ${LIB_INSTALL_DIR}
)


##### tde-config ################################

# FIXME still need some research and improvements
configure_file( tde-config.cpp.cmake tde-config.cpp @ONLY )


tde_add_executable( tde-config
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/tde-config.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

##### kgrantpty #################################

# FIXME there is something named KDE_USE_FPIE, I'm not sure what is this
#set_target_properties( kgrantpty PROPERTIES COMPILE_FLAGS ${KDE_USE_FPIE} )
#set_target_properties( kgrantpty PROPERTIES LINK_FLAGS ${KDE_USE_FPIE} )

tde_add_executable(kgrantpty SETUID
  SOURCES kgrantpty.c
  DESTINATION ${BIN_INSTALL_DIR}
)

##### kdetcompmgr #################################

tde_add_executable( kdetcompmgr
  SOURCES kdetcompmgr.cpp
  LINK tdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
