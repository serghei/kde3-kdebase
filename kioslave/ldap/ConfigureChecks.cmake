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

check_include_file( ldap.h HAVE_LDAP_H )

if( HAVE_LDAP_H )
  set( LDAP_LIBRARIES ldap )
  check_library_exists( ${LDAP_LIBRARIES} ldap_initialize "" HAVE_LDAP )
endif( )

if( NOT HAVE_LDAP_H OR NOT HAVE_LDAP )
  kde_message_fatal( "ldap is required, but was not found on your system." )
endif( )
