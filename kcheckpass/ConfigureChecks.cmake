#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

find_library( CRYPT_LIBRARY crypt )

if( WITH_PAM AND (NOT DEFINED KCHECKPASS_PAM_SERVICE) )
  set( KCHECKPASS_PAM_SERVICE "kde" CACHE INTERNAL "" )
endif( )
