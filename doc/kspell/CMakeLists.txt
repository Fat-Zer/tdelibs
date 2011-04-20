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

tde_create_handbook( SRCDIR ${CMAKE_SOURCE_DIR}/kdoctools DESTINATION kspell )
add_dependencies( kspell-en-handbook meinproc )
