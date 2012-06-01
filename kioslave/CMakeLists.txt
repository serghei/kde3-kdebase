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

include( ConfigureChecks.cmake )

add_subdirectory( about )
add_subdirectory( cgi )
add_subdirectory( floppy )
add_subdirectory( filter )
add_subdirectory( fish )
add_subdirectory( info )
add_subdirectory( mac )
add_subdirectory( man )
add_subdirectory( nfs )
add_subdirectory( nntp )
add_subdirectory( pop3 )
add_subdirectory( smtp )
add_subdirectory( sftp )
add_subdirectory( tar )
add_subdirectory( finger )
add_subdirectory( thumbnail )
add_subdirectory( settings )
add_subdirectory( trash )
add_subdirectory( media )
add_subdirectory( remote )
add_subdirectory( home )
add_subdirectory( system )
kde_conditional_add_subdirectory( WITH_LDAP ldap )
kde_conditional_add_subdirectory( WITH_SAMBA smb )
