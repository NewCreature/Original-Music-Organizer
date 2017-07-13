# Automatically selects the correct makefile for the current platform,
# assumes user is building from source for their own use (won't select static
# or cross-compile makefiles)

ifeq ($(OS),Windows_NT)
 SYSTEM=mingw
else
 UNAME=$(shell uname)
 ifneq (,$(findstring Linux,$(UNAME)))
  SYSTEM=linux
 else
  ifneq (,$(findstring Darwin,$(UNAME)))
   SYSTEM=macosx
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
	$(MAKE) -f makefile.$(SYSTEM) all

clean:
	$(MAKE) -f makefile.$(SYSTEM) clean

clean_all:
	$(MAKE) -f makefile.$(SYSTEM) clean

package:
	$(MAKE) -f makefile.$(SYSTEM) clean
	$(MAKE) -f makefile.$(SYSTEM) package

debian_package:
	$(MAKE) -f makefile.$(SYSTEM)_static debian_package

android_package:
	$(MAKE) -f makefile.android android_package

android_repackage:
	$(MAKE) -f makefile.android android_repackage

android_sign:
	$(MAKE) -f makefile.android android_sign

windows_package:
	$(MAKE) -f makefile.mingw_cross windows_zip

mac_package:
	$(MAKE) -f makefile.macosx mac_package

demo:
	$(MAKE) -f makefile.$(SYSTEM) demo

demo_gp:
	$(MAKE) -f makefile.$(SYSTEM) demo_gp
