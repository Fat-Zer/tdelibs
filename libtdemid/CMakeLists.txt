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
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tdecore
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### headers ###################################

install( FILES
    midiout.h player.h track.h midimapper.h midfile.h
    dattypes.h midistat.h deviceman.h synthout.h
    fmout.h gusout.h alsaout.h voiceman.h notearray.h
    mt32togm.h midispec.h libtdemid.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/libtdemid )


##### tdemid ######################################

set( target tdemid )

set( ${target}_SRCS
  midiout.cc player.cc track.cc midimapper.cc
  midfile.cc dattypes.cc midistat.cc deviceman.cc
  synthout.cc fmout.cc gusout.cc alsaout.cc voiceman.cc
  mt32togm.cc notearray.cc libtdemid.cc
)

tde_add_library( ${target} SHARED
  SOURCES ${${target}_SRCS}
  VERSION 0.0.95
  LINK tdecore-shared ${ALSA_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)
