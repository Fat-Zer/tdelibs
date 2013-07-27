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
  ${CMAKE_CURRENT_BINARY_DIR}
  ${TQT_INCLUDE_DIRS}
  ${DBUS_TQT_INCLUDE_DIRS}
)


##### tdenm_dbus (static) #########################

set( INTROSPECTIONPATH ${CMAKE_CURRENT_SOURCE_DIR} )

# options:
#  p - generate proxy files
#  i - generate interface files
#  n - generate node files

function( add_dbus_xml option basename classname namespace xmlfile )
  if( classname )
    set( classname -c ${classname} )
  endif( )
  add_custom_command( OUTPUT ${basename}.cpp ${basename}.h
    COMMAND ${DBUSXML2QT3_EXECUTABLE} -${option} ${basename} ${classname} -N ${namespace} ${INTROSPECTIONPATH}/${xmlfile} 2>/dev/null
    DEPENDS ${INTROSPECTIONPATH}/${xmlfile} )
endfunction( )

add_dbus_xml( p networkmanagerproxy NetworkManagerProxy DBus nm-manager.xml )
add_dbus_xml( p networkmanagerpppproxy NetworkManagerPPPProxy DBus nm-ppp-manager.xml )
add_dbus_xml( p deviceproxy DeviceProxy DBus nm-device.xml )
add_dbus_xml( p wifiproxy WiFiDeviceProxy DBus nm-device-wifi.xml )
add_dbus_xml( p wimaxproxy WiMaxDeviceProxy DBus nm-device-wimax.xml )
add_dbus_xml( p olpcmeshproxy OlpcMeshDeviceProxy DBus nm-device-olpc-mesh.xml )
add_dbus_xml( p bluetoothproxy BluetoothDeviceProxy DBus nm-device-bt.xml )
add_dbus_xml( p ethernetproxy EthernetDeviceProxy DBus nm-device-ethernet.xml )
add_dbus_xml( p infinibandproxy InfinibandDeviceProxy DBus nm-device-infiniband.xml )
add_dbus_xml( p modemproxy ModemDeviceProxy DBus nm-device-modem.xml )
add_dbus_xml( p accesspointproxy AccessPointProxy DBus nm-access-point.xml )
add_dbus_xml( p activeconnectionproxy ActiveConnectionProxy DBus nm-active-connection.xml )
add_dbus_xml( p vpnconnectionproxy VPNConnectionProxy DBus nm-vpn-connection.xml )
add_dbus_xml( p vpnpluginproxy VPNPluginProxy DBus nm-vpn-plugin.xml )
add_dbus_xml( p networkmanagersettings SettingsInterface DBus nm-settings.xml )
add_dbus_xml( p connectionsettings ConnectionSettingsInterface DBus nm-settings-connection.xml )


# generate moc files

set( MOCHEADERS accesspointproxy.h deviceproxy.h networkmanagerproxy.h networkmanagerpppproxy.h bluetoothproxy.h olpcmeshproxy.h ethernetproxy.h
  infinibandproxy.h wifiproxy.h wimaxproxy.h activeconnectionproxy.h vpnconnectionproxy.h vpnpluginproxy.h modemproxy.h
  networkmanagersettings.h connectionsettings.h )

foreach( _header_file ${MOCHEADERS} )
  get_filename_component( _basename "${_header_file}" NAME_WE )
  set( _moc_file ${_basename}.moc )
  list( APPEND _moc_files ${_moc_file} )
  add_custom_command( OUTPUT ${_moc_file}
    COMMAND ${TMOC_EXECUTABLE} ${_header_file} -o ${_moc_file}
    DEPENDS ${_header_file} )
endforeach( )

add_custom_command( OUTPUT mocfiles.cpp
  COMMAND cat ${_moc_files} > mocfiles.cpp
  DEPENDS ${_moc_files} )

# tde_automoc( accesspointproxy.h deviceproxy.h networkmanagerproxy.h
#   networkmanagerpppproxy.h bluetoothproxy.h olpcmeshproxy.h
#   ethernetproxy.h infinibandproxy.h wifiproxy.h wimaxproxy.h
#   activeconnectionproxy.h vpnconnectionproxy.h vpnpluginproxy.h
#   modemproxy.h networkmanagersettings.h connectionsettings.h ) 

# build the library

tde_add_library( tdenm_dbus STATIC_PIC
  SOURCES
    networkmanagerproxy.cpp networkmanagerpppproxy.cpp
    deviceproxy.cpp bluetoothproxy.cpp olpcmeshproxy.cpp ethernetproxy.cpp
    infinibandproxy.cpp wifiproxy.cpp wimaxproxy.cpp accesspointproxy.cpp
    networkmanagersettings.cpp connectionsettings.cpp
    mocfiles.cpp activeconnectionproxy.cpp vpnconnectionproxy.cpp
    vpnpluginproxy.cpp modemproxy.cpp
)
