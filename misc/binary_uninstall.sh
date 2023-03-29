#!/bin/bash

prefix=${PREFIX=/usr/local}
app_title="T3F_APP_TITLE"
app_name=T3F_APP_NAME
app_dir=T3F_APP_DIR
app_categories=T3F_APP_CATEGORIES

if [ $EUID != 0 ]
then
  echo "Uninstallation of this application requires superuser priveleges. If you are not "
  echo "already running this script as a superuser you may be asked to authenticate."
  echo ""
  sudo $0
else
  if which $prefix/$app_dir/$app_name &>/dev/null
  then
    echo "Uninstalling T3F_APP_TITLE..."
    rm -rf $prefix/share/$app_name
    rm -rf $prefix/share/doc/$app_name
    rm -rf $prefix/share/$app_name
    rm -f $prefix/share/icons/$app_name.svg 2>/dev/null
    rm -f $prefix/share/icons/$app_name.png 2>/dev/null
    rm -f $prefix/$app_dir/$app_name
    rm -f $prefix/$app_dir/$app_name-helper.sh
    rm -f $prefix/share/applications/$app_name.desktop
    update-desktop-database
  else
    echo "Application doesn't appear to be installed. Uninstallation cancelled."
    echo ""
  fi
fi
