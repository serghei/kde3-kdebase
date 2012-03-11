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

find_program( SETXKBMAP setxkbmap )
if( SETXKBMAP-NOTFOUND )
  kde_message_fatal( "setxkbmap is required, but not found on your system" )
endif()
