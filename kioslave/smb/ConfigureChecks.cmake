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

check_include_file( libsmbclient.h HAVE_LIBSMBCLIENT_H )

if( HAVE_LIBSMBCLIENT_H )
  set( SMBCLIENT_LIBRARIES smbclient )
  check_library_exists( ${SMBCLIENT_LIBRARIES} smbc_new_context "" HAVE_SMBCLIENT )
endif( )

if( NOT HAVE_LIBSMBCLIENT_H OR NOT HAVE_SMBCLIENT )
  kde_message_fatal( "smbclient is required, but was not found on your system." )
endif( )
