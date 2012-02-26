#!/bin/sh

bas=`tde-config --localprefix`share/apps/kio_help
mv "$bas/cache" "`tde-config --path cache`kio_help"
rmdir "$bas"
exit 0
