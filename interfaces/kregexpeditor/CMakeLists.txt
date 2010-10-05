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

##### headers ###################################

install( FILES
    kregexpeditorinterface.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )

##### other data ################################

install( FILES kregexpeditor.desktop DESTINATION ${SHARE_INSTALL_DIR}/servicetypes )
