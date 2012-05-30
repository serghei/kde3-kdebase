#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include( CheckIncludeFile )
include( CheckFunctionExists )
include( CheckLibraryExists )
include( FindPkgConfig )


# find kdelibs
# FIXME should be more flexibile
set( CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/cmake" )
if( NOT EXISTS "${CMAKE_MODULE_PATH}/FindKDE3.cmake" )
  message( FATAL_ERROR
      "\n"
      "####################################################################\n "
      "${CMAKE_MODULE_PATH}/FindKDE3.cmake does not exists.\n "
      "Please pass corect CMAKE_INSTALL_PREFIX to cmake.\n"
      "####################################################################\n" )
endif( )
include( FindKDE3 )


# termios.h (kdm, kioslave)
if( BUILD_KDM OR BUILD_KIOSLAVES )
  check_include_file( termios.h HAVE_TERMIOS_H )
endif( )


# sys/ioctl.h (kioslave/fish, kcontrol/info)
if( BUILD_KIOSLAVES OR BUILD_KCONTROL )
  check_include_file( sys/ioctl.h HAVE_SYS_IOCTL_H )
endif( )


# pam
if( WITH_PAM AND (BUILD_KCHECKPASS OR BUILD_KDM) )
  check_library_exists( pam pam_start "" HAVE_PAM )
  if( HAVE_PAM )
    check_include_file( "security/pam_appl.h" SECURITY_PAM_APPL_H )
  endif( )
  if( HAVE_PAM AND SECURITY_PAM_APPL_H )
    set( PAM_LIBRARY pam;dl )
  else( )
    kde_message_fatal( "pam is required, but not found on your system" )
  endif( )
endif( )

# xtst
if( WITH_XTST AND (BUILD_KXKB OR BUILD_KDM OR BUILD_KHOTKEYS) )
  kde_search_module( XTEST xtst )
endif( )

# hal (ksmserver, kioslaves)
if( WITH_HAL )
  pkg_search_module( HAL hal )
  if( NOT HAL_FOUND )
    kde_message_fatal( "hal is required, but was not found on your system" )
  endif( )
endif( )


# xrender (kdesktop, konsole, kcontrol, kicker, kwin)
if( WITH_XRENDER OR BUILD_KDESKTOP OR BUILD_KONSOLE OR BUILD_KCONTROL OR BUILD_KICKER )
  pkg_search_module( XRENDER xrender )
  if( XRENDER_FOUND )
    set( HAVE_XRENDER 1 )
  elseif( WITH_XRENDER )
    kde_message_fatal( "xrender is required, but was not found on your system" )
  endif( )
endif( )


# xcursor (kioslave, kcontrol)
if( WITH_XCURSOR )
  pkg_search_module( XCURSOR xcursor )
  if( XCURSOR_FOUND )
    set( HAVE_XCURSOR 1 )
  else( )
    kde_message_fatal( "xcursor is required, but was not found on your system" )
  endif( )
endif( )


# xcomposite (kicker, kwin)
if( WITH_XCOMPOSITE )
  pkg_search_module( XCOMPOSITE xcomposite )
  if( XCOMPOSITE_FOUND )
    set( HAVE_XCOMPOSITE 1 )
  else( XCOMPOSITE_FOUND )
    kde_message_fatal( "xcomposite is required, but was not found on your system" )
  endif( XCOMPOSITE_FOUND )
endif( )


# xfixes (klipper, kicker)
if( WITH_XFIXES )
  pkg_search_module( XFIXES xfixes )
  if( XFIXES_FOUND )
    set( HAVE_XFIXES 1 CACHE INTERNAL "" FORCE )
  else( )
    kde_message_fatal( "xfixes is required, but was not found on your system" )
  endif( )
endif( )


# xdamage (kwin/kompmgr)
if( WITH_XDAMAGE )
  pkg_search_module( XDAMAGE xdamage )
  if( NOT XDAMAGE_FOUND )
    kde_message_fatal( "xdamage is required, but was not found on your system" )
  endif( )
endif( )


# xext (kwin/kompmgr/kdesktop)
if( WITH_XEXT OR BUILD_KDESKTOP )
  pkg_search_module( XEXT xext )
  if( NOT XEXT_FOUND )
    kde_message_fatal( "xext is required, but was not found on your system" )
  endif( )
endif( )


# GL
if( BUILD_KDESKTOP OR BUILD_KCONTROL OR BUILD_KSCREENSAVER )
  check_library_exists( GL glXChooseVisual "" HAVE_GLXCHOOSEVISUAL )
  if( HAVE_GLXCHOOSEVISUAL )
    set( GL_LIBRARY "GL" )
  endif( )
endif( )


# glib-2.0
if( BUILD_NSPLUGINS )
  pkg_search_module( GLIB2 glib-2.0 )
  if( NOT GLIB2_FOUND )
    kde_message_fatal( "glib-2.0 is required, but not found on your system" )
  endif( )
endif( )


# kde_socklen_t
if( BUILD_KIOSLAVES OR BUILD_KSYSGUARD )
  set( kde_socklen_t socklen_t )
endif( )


# getifaddrs (kcontrol, kdm)
if( BUILD_KCONTROL OR BUILD_KDM )
  check_function_exists( getifaddrs HAVE_GETIFADDRS )
endif( )


# xkb (konsole, kdm, kxkb)
if( BUILD_KONSOLE OR BUILD_KDM OR BUILD_KXKB )
  check_include_file( X11/XKBlib.h HAVE_X11_XKBLIB_H )
  if( HAVE_X11_XKBLIB_H )
    check_library_exists( X11 XkbLockModifiers "" HAVE_XKB )
    if( BUILD_KDM )
      check_library_exists( X11 XkbSetPerClientControls "" HAVE_XKBSETPERCLIENTCONTROLS )
    endif( )
  endif( )
endif( )


# XBINDIR, XLIBDIR (kdm, kxkb)
if( BUILD_KDM OR BUILD_KXKB )
  find_program( some_x_program NAMES iceauth xrdb xterm )
  if( NOT some_x_program )
    set( some_x_program /usr/bin/xrdb )
    message( STATUS "Warning: Could not determine X binary directory. Assuming /usr/bin." )
  endif( )
  get_filename_component( proto_xbindir "${some_x_program}" PATH )
  get_filename_component( XBINDIR "${proto_xbindir}" ABSOLUTE )
  get_filename_component( xrootdir "${XBINDIR}" PATH )
  set( XBINDIR ${XBINDIR} CACHE INTERNAL "" )
  set( XLIBDIR "${xrootdir}/lib/X11" CACHE INTERNAL "" )
endif( )
