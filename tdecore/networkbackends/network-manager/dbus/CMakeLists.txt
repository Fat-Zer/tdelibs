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

set( INTROSPECTIONPATH "${CMAKE_SOURCE_DIR}/tdecore/networkbackends/network-manager/introspection" )

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
add_dbus_xml( p networkmanagervpnproxy NetworkManagerVPNProxy DBus nm-vpn-manager.xml )
add_dbus_xml( p deviceproxy DeviceProxy DBus nm-device.xml )
add_dbus_xml( p wirelessproxy WirelessDeviceProxy DBus nm-device-802-11-wireless.xml )
add_dbus_xml( p wiredproxy WiredDeviceProxy DBus nm-device-802-3-ethernet.xml )
add_dbus_xml( p gsmproxy GSMDeviceProxy DBus nm-device-gsm.xml )
add_dbus_xml( p cdmaproxy CDMADeviceProxy DBus nm-device-cdma.xml )
add_dbus_xml( p accesspointproxy AccessPointProxy DBus nm-access-point.xml )
add_dbus_xml( p activeconnectionproxy ActiveConnectionProxy DBus nm-active-connection.xml )
add_dbus_xml( p vpnconnectionproxy VPNConnectionProxy DBus nm-vpn-connection-only.xml )
add_dbus_xml( p vpnpluginproxy VPNPluginProxy DBus nm-vpn-plugin.xml )
add_dbus_xml( i networkmanagersettings SettingsInterface DBus nm-settings.xml )
add_dbus_xml( i connection "" DBus nm-exported-connection.xml )

add_custom_command( OUTPUT connectionnode.cpp connectionnode.h introspectableinterface.cpp introspectableinterface.h
    COMMAND ${DBUSXML2QT3_EXECUTABLE} -n connectionnode -c ConnectionNode -N DBus ${INTROSPECTIONPATH}/nm-exported-connection.xml 2>/dev/null
    COMMAND sed s/\#include\\ \\"secrets.h\\"// connectionnode.cpp > connectionnode.cpp_tmp
    COMMAND sed s/\#include\\ \\"introspectable.h\\"/\#include\\ \\"introspectableinterface.h\\"/ connectionnode.cpp_tmp > connectionnode.cpp
    COMMAND rm -f connectionnode.cpp_tmp
    DEPENDS ${INTROSPECTIONPATH}/nm-exported-connection.xml )


# generate moc files

set( MOCHEADERS accesspointproxy.h deviceproxy.h networkmanagerproxy.h networkmanagervpnproxy.h wiredproxy.h
  wirelessproxy.h activeconnectionproxy.h vpnconnectionproxy.h vpnpluginproxy.h gsmproxy.h cdmaproxy.h )

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


# build the library

tde_add_library( tdenm_dbus STATIC_PIC
  SOURCES
    networkmanagerproxy.cpp networkmanagervpnproxy.cpp
    deviceproxy.cpp wiredproxy.cpp wirelessproxy.cpp
    accesspointproxy.cpp networkmanagersettings.cpp
    connection.cpp connectionnode.cpp introspectableinterface.cpp
    mocfiles.cpp activeconnectionproxy.cpp vpnconnectionproxy.cpp
    vpnpluginproxy.cpp gsmproxy.cpp cdmaproxy.cpp
)
