# Automatically selects the correct makefile for the current platform,
# assumes user is building from source for their own use (won't select static
# or cross-compile makefiles)

ifeq ($(OS),Windows_NT)
 UNAME=$(shell uname)
 ifneq (,$(findstring MINGW,$(UNAME)))
  SYSTEM=msys
 else
  SYSTEM=mingw
 endif
else
 UNAME=$(shell uname)
 ifneq (,$(findstring Linux,$(UNAME)))
  SYSTEM=linux
 else
  ifneq (,$(findstring Darwin,$(UNAME)))
   SYSTEM=macos
  else
   ifneq (,$(findstring MINGW,$(UNAME)))
    SYSTEM=mingw_cross
   else
    $(warning Not sure which system you are on, assuming Linux)
    $(warning   (uname says: '$(UNAME)'))
    SYSTEM=linux
   endif
  endif
 endif
endif

top:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts all

static:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM)_static -I ../scripts all

clean:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts clean

clean_all:
	$(MAKE) -f ../makefile.$(SYSTEM) -I ../scripts clean

release:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts release

package:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts clean
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts package

binary_package:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts binary_package

debian_package:
	$(MAKE) -f ../scripts/makefile.debian -I ../scripts debian_package

android_package:
	$(MAKE) -f ../scripts/makefile.android -I ../scripts android_package

android_package_unsigned:
	$(MAKE) -f ../scripts/makefile.android -I ../scripts android_package_unsigned

windows_package:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts windows_zip

mac_universal_bundle:
	$(MAKE) -f ../scripts/makefile.macos -I ../scripts mac_universal_bundle

mac_bundle:
	$(MAKE) -f ../scripts/makefile.macos -I ../scripts mac_bundle

mac_package:
	$(MAKE) -f ../scripts/makefile.macos -I ../scripts mac_package

iphone_package:
	$(MAKE) -f ../scripts/makefile.iphone -I ../scripts iphone_package

demo:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts demo

demo_gp:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts demo_gp

makeicon:
	$(MAKE) -f ../scripts/makefile.$(SYSTEM) -I ../scripts ../win32/makeicon
