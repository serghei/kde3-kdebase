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


##### getopt.h ##################################

check_include_file( getopt.h HAVE_GETOPT_H )


##### check for freetype2 #######################

pkg_search_module( FREETYPE freetype2 )
if( FREETYPE_FOUND )
  set( HAVE_FREETYPE2 1 CACHE INTERNAL "" FORCE  )
else( )
  kde_message_fatal( "freetype2 is required, but was not found on your system" )
endif( )


##### check for fontconfig ######################

pkg_search_module( FONTCONFIG fontconfig )
if( FONTCONFIG_FOUND )
  set( HAVE_FONTCONFIG 1 CACHE INTERNAL "" FORCE )
else( )
  kde_message_fatal( "fontconfig is required, but was not found on your system" )
endif( )


##### check for xft #############################

pkg_search_module( XFT xft )
if( XFT_FOUND )
  set( HAVE_XFT 1 CACHE INTERNAL "" FORCE )
else( )
  kde_message_fatal( "xft is required, but was not found on your system" )
endif( )


##### check for Xrandr ##########################

if( WITH_XRANDR )
  pkg_search_module( XRANDR xrandr )
  if( NOT XRANDR_FOUND )
    kde_message_fatal( "xrandr is required, but was not found on your system" )
  endif( )
endif( )


##### check for libusb ##########################

if( WITH_LIBUSB )
  kde_search_config( LIBUSB libusb )
endif( )


##### check for libraw1394 ######################

if( WITH_LIBRAW1394 )
  pkg_search_module( LIBRAW1394 libraw1394 )
  if( NOT LIBRAW1394_FOUND )
    kde_message_fatal( "libraw1394 is required, but was not found on your system" )
  endif( )
endif( )
