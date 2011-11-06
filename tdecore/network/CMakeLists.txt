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
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_BINARY_DIR}/tdecore
)


##### headers ###################################

install( FILES
    kresolver.h kreverseresolver.h ksocketaddress.h
    ksocketbase.h ksocketdevice.h kclientsocketbase.h
    kstreamsocket.h kserversocket.h kdatagramsocket.h
    kmulticastsocketdevice.h kmulticastsocket.h
    knetworkinterface.h khttpproxysocketdevice.h
    ksockssocketdevice.h kbufferedsocket.h kiobuffer.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### other files ###############################

install( FILES ipv6blacklist DESTINATION ${CONFIG_INSTALL_DIR} )


##### tdecorenetwork ############################

set( target tdecorenetwork )

set( ${target}_SRCS
  kresolver.cpp kresolvermanager.cpp kresolverworkerbase.cpp
  ksocketaddress.cpp kresolverstandardworkers.cpp
  kreverseresolver.cpp ksocketdevice.cpp ksocketbase.cpp
  kclientsocketbase.cpp kstreamsocket.cpp kserversocket.cpp
  kdatagramsocket.cpp khttpproxysocketdevice.cpp
  ksockssocketdevice.cpp kbufferedsocket.cpp ksocketbuffer.cpp
  ksrvresolverworker.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)
