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

add_custom_command(
  OUTPUT index.cache.bz2
  COMMAND ${CMAKE_BINARY_DIR}/kdoctools/meinproc --srcdir=${CMAKE_SOURCE_DIR}/kdoctools --check --cache index.cache.bz2 ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/index.docbook meinproc )

install( FILES
    index.docbook ${CMAKE_CURRENT_BINARY_DIR}/index.cache.bz2
  DESTINATION ${HTML_INSTALL_DIR}/en/kspell )

add_custom_target( kspell-doc ALL DEPENDS index.cache.bz2 )
