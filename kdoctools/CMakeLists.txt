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
  ${TQT_CFLAGS_OTHER}
)

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kdecore
  ${CMAKE_SOURCE_DIR}/kio
  ${CMAKE_SOURCE_DIR}/kio/kio
  ${CMAKE_SOURCE_DIR}/kioslave
  ${LIBXML2_INCLUDE_DIRS}
  ${LIBXSLT_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### other data ################################

install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/checkXML DESTINATION ${BIN_INSTALL_DIR} )
install( PROGRAMS move_kio_help_cache.sh DESTINATION ${DATA_INSTALL_DIR}/kconf_update )
install( FILES kio_help.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update )
install( FILES help.protocol ghelp.protocol DESTINATION ${SERVICES_INSTALL_DIR} )

# FIXME why automake version is so complicated?
# install-data-local:
#       @(for subdir in customization docbook; do \
#           for dir in `( cd $(srcdir)/$$subdir && find . -type d ! -name CVS | grep -v '\.svn' )`; do \
#               $(mkinstalldirs) $(DESTDIR)$(kde_datadir)/ksgmltools2/$$subdir/$$dir ;\
#               for file in `(cd $(srcdir)/$$subdir && ls -1 $$dir/*)`; do \
#                   if test -f $(srcdir)/$$subdir/$$file; then \
#                       $(INSTALL_DATA) $(srcdir)/$$subdir/$$file  $(DESTDIR)$(kde_datadir)/ksgmltools2/$$subdir/$$file; fi ;\
#               done ;\
#           done ;\
#       done )
install( DIRECTORY customization docbook DESTINATION ${DATA_INSTALL_DIR}/ksgmltools2 PATTERN ".svn" EXCLUDE )


##### kbzipfilter_dummy ##############################

tde_add_library( kbzipfilter_dummy STATIC_PIC
  SOURCES ${CMAKE_SOURCE_DIR}/kioslave/bzip2/kbzip2filter.cpp
)


#### kio_help ###################################

set( target kio_help )

set( ${target}_SRCS
  kio_help.cpp main.cpp xslt.cpp
)

tde_add_kpart( ${target} SHARED
  SOURCES ${${target}_SRCS}
  LINK kbzipfilter_dummy-static kio-shared ${BZIP2_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### kio_ghelp ###################################

set( target kio_ghelp )

set( ${target}_SRCS
  kio_help.cpp main_ghelp.cpp xslt.cpp
)

tde_add_kpart( ${target} SHARED
  SOURCES ${${target}_SRCS}
  LINK kbzipfilter_dummy-static kio-shared ${BZIP2_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


#### meinproc ###################################

set( target meinproc )

set( ${target}_SRCS
  meinproc.cpp xslt_pure.cpp
)

# FIXME I think this can be improved
add_custom_command( OUTPUT xslt_pure.cpp COMMAND cp ARGS ${CMAKE_CURRENT_SOURCE_DIR}/xslt.cpp xslt_pure.cpp )
set_source_files_properties( xslt_pure.cpp PROPERTIES COMPILE_FLAGS -DSIMPLE_XSLT )

tde_add_executable( ${target}
  SOURCES ${${target}_SRCS}
  LINK kbzipfilter_dummy-static kio-shared ${BZIP2_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
