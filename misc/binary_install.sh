#!/bin/bash

prefix=${PREFIX=/usr/local}
app_title="T3F_APP_TITLE"
app_name=T3F_APP_NAME
app_dir=T3F_APP_DIR
app_categories=T3F_APP_CATEGORIES

if [ $EUID != 0 ]
then
  echo "Installation of this application requires superuser priveleges. If you are not "
  echo "already running this script as a superuser you may be asked to authenticate."
  echo ""
  sudo $0
else
  if which bin/$app_name &>/dev/null
  then
    echo "Installing T3F_APP_TITLE for all users..."
    mkdir -p $prefix/share/$app_name
    mkdir -p $prefix/share/doc
    mkdir -p $prefix/share/doc/$app_name
    mkdir -p $prefix/share/icons
    mkdir -p $prefix/share/applications
    mkdir -p $prefix/share/menu
    cp -a bin/data $prefix/share/$app_name
    cp docs/changelog $prefix/share/doc/$app_name/changelog 2>/dev/null || true
    cp docs/README $prefix/share/doc/$app_name/README 2>/dev/null || true
    cp docs/copyright $prefix/share/doc/$app_name/copyright 2>/dev/null || true
    cp icons/icon.svg $prefix/share/icons/$app_name.svg 2>/dev/null || true
    cp icons/icon.png $prefix/share/icons/$app_name.png 2>/dev/null || true
    cp bin/* $prefix/$app_dir/ 2>/dev/null
    printf "[Desktop Entry]\nName=%s\nExec=%s/%s/%s %%F\nIcon=%s\nTerminal=false\nType=Application\nCategories=%s;" "$app_title" "$prefix" "$app_dir" "$app_name" "$app_name" "$app_categories"> $prefix/share/applications/$app_name.desktop
    chmod 755 $prefix/$app_dir/$app_name
    find $prefix/share/$app_name -type f -exec chmod 644 {} \;
    chmod 644 $prefix/share/doc/$app_name/README 2>/dev/null || true
    chmod 644 $prefix/share/doc/$app_name/changelog 2>/dev/null || true
    chmod 644 $prefix/share/doc/$app_name/copyright 2>/dev/null || true
    chmod 644 $prefix/share/icons/$app_name.svg 2>/dev/null || true
    chmod 644 $prefix/share/icons/$app_name.png 2>/dev/null || true
    chmod 644 $prefix/share/applications/$app_name.desktop
    update-desktop-database
  else
    echo "Application executable binary missing. Installation cancelled."
    echo ""
  fi
fi
