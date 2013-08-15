#################################################
#
#  (C) 2013 Golubev Alexander
#  fatzer2 (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

add_subdirectory( hwlibdata )
tde_conditional_add_subdirectory( WITH_TDEHWLIB_DAEMONS hwlibdaemons )
add_subdirectory( networkbackends )

if( NOT DBUS_SYSTEM_BUS )
  set( DBUS_SYSTEM_BUS "unix:path=/var/run/dbus/system_bus_socket" CACHE INTERNAL "" FORCE )
endif()

if( WITH_TDEHWLIB_DAEMONS OR WITH_HAL OR WITH_DEVKITPOWER OR WITH_UPOWER OR WITH_UDISKS OR WITH_UDISKS2 OR
    WITH_NETWORK_MANAGER_BACKEND OR WITH_CONSOLEKIT )
  list( APPEND TDEHW_CUSTOM_INCLUDE_DIRS ${DBUS_TQT_INCLUDE_DIRS} )
  list( APPEND TDEHW_CUSTOM_LIBRARY_DIRS ${DBUS_TQT_LIBRARY_DIRS} )
  list( APPEND TDEHW_CUSTOM_LIBRARIES ${DBUS_TQT_LIBRARIES} )
endif( )

if( WITH_TDEHWLIB_DAEMONS )
  add_definitions( -DWITH_TDEHWLIB_DAEMONS )
endif( )

if( WITH_HAL )
  add_definitions( -DWITH_HAL )
endif( )

if( WITH_DEVKITPOWER )
  add_definitions( -DWITH_DEVKITPOWER )
endif( )

if( WITH_UPOWER )
  add_definitions( -DWITH_UPOWER )
endif( )

if( WITH_UDISKS )
  add_definitions( -DWITH_UDISKS )
endif( )

if( WITH_UDISKS2 )
  add_definitions( -DWITH_UDISKS2 )
endif( )

if( WITH_CONSOLEKIT )
  add_definitions( -DWITH_CONSOLEKIT )
endif( )

if( WITH_NETWORK_MANAGER_BACKEND )
  list( APPEND TDEHW_CUSTOM_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/networkbackends/network-manager )
  list( APPEND TDEHW_CUSTOM_LIBRARIES network_manager_backend-static )
endif( WITH_NETWORK_MANAGER_BACKEND )

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/dcop
  ${TDEHW_CUSTOM_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${TDEHW_CUSTOM_LIBRARY_DIRS}
)

##### headers ###################################

install( FILES tdehardwaredevices.h tdenetworkconnections.h tdegenericdevice.h
    tdestoragedevice.h tdecpudevice.h tdebatterydevice.h tdemainspowerdevice.h
    tdenetworkdevice.h tdebacklightdevice.h tdemonitordevice.h
    tdesensordevice.h tderootsystemdevice.h tdeeventdevice.h tdeinputdevice.h
    tdehwcommontypes.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### libtdecore ################################

set( target tdehw )

set( ${target}_SRCS
  tdehardwaredevices.cpp tdenetworkconnections.cpp tdegenericdevice.cpp
  tdestoragedevice.cpp tdecpudevice.cpp tdebatterydevice.cpp
  tdemainspowerdevice.cpp tdenetworkdevice.cpp tdebacklightdevice.cpp
  tdemonitordevice.cpp tdesensordevice.cpp tderootsystemdevice.cpp
  tdeeventdevice.cpp tdeinputdevice.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK udev ${TDENM_LIBRARIES} ${TDEUPOWER_LIBRARIES}
    ${TDEHW_CUSTOM_LIBRARIES}
)
