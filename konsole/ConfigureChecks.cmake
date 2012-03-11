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

if( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  set( HAVE_PROC_CWD 1 CACHE INTERNAL "" FORCE )
endif()
