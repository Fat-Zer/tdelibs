#!/bin/bash

# A script to dynamically update general.entities with the current version release information.

# As the help files are updated/revised, DocBook entities can be used rather than static strings for
# release version, release date, and copyright date. This allows for a professional touch with each
# updated help file to show the file is relevant to the current Trinity release.

TDEVERSION_FILE="@CMAKE_SOURCE_DIR@/tdecore/tdeversion.h"
ENTITIES_FILE="@CMAKE_SOURCE_DIR@/kdoctools/customization/entities/general.entities"

echo "-- Updating $ENTITIES_FILE:"
# Extract the Trinity version number.
if [ -f "$TDEVERSION_FILE" ]; then
  TDE_RELEASE_VERSION="`grep TDE_VERSION_STRING \"$TDEVERSION_FILE\"`"
  #echo "     TDE_RELEASE_VERSION: $TDE_RELEASE_VERSION"
  if [ -z "$TDE_RELEASE_VERSION" ]; then
    echo "Cannot determine the Trinity version number. Please verify $TDEVERSION_FILE exists."
    echo
    exit 1
  fi
  if [ -n "`echo \"$TDE_RELEASE_VERSION\" | grep DEVELOPMENT`" ]; then
    TDE_RELEASE_VERSION="`echo $TDE_RELEASE_VERSION | awk '{print $3,$4}' | sed -e 's/"//g'`"
  else
    TDE_RELEASE_VERSION="`echo $TDE_RELEASE_VERSION | awk '{print $3}' | sed -e 's/"//g'`"
  fi
  echo "     TDE Release Version: $TDE_RELEASE_VERSION"
  if [ -z "$TDE_RELEASE_VERSION" ]; then
    echo "Cannot determine the Trinity version number. Please verify $TDEVERSION_FILE exists."
    echo
    exit 1
  fi
else
  echo "Please verify $TDEVERSION_FILE exists."
  echo
  exit 1
fi

# Extract the file date stamp to use as the release date.
TDE_RELEASE_DATE=`find $TDEVERSION_FILE -printf "%TB %Te, %TY\n"`
echo "     TDE Release Date: $TDE_RELEASE_DATE"
# Create a copyright date string. First release of Trinity was 3.5.11, April 29, 2010.
TDE_RELEASE_COPYRIGHT="2010-`date +%Y`"
echo "     TDE Release Copyright: $TDE_RELEASE_COPYRIGHT"

# Now update $ENTITIES_FILE.
if [ -r "$ENTITIES_FILE" ]; then
  echo "" >> $ENTITIES_FILE
  echo -e "<!ENTITY tde-release-version \"${TDE_RELEASE_VERSION}\">" >> $ENTITIES_FILE
  echo -e "<!ENTITY tde-release-date \"${TDE_RELEASE_DATE}\">" >> $ENTITIES_FILE
  echo -e "<!ENTITY tde-copyright-date \"${TDE_RELEASE_COPYRIGHT}\">" >> $ENTITIES_FILE
else
  echo "Please verify $ENTITIES_FILE exists."
  echo
  exit 1
fi
exit 0
