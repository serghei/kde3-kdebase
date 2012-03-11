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

# FIXME initial work, only PAM and shadow
# FIXME needs more checks (for kerberos, etc)

if( NOT DEFINED KDM_PAM_SERVICE )
  set( KDM_PAM_SERVICE "kde" CACHE INTERNAL "" )
endif( )


# generate config.ci
add_custom_command( OUTPUT config.ci
  COMMAND perl -w ${CMAKE_CURRENT_SOURCE_DIR}/confproc.pl ${CMAKE_CURRENT_SOURCE_DIR}/config.def config.ci
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/config.def )

add_custom_target( config.ci-common DEPENDS config.ci )

add_subdirectory( backend )
add_subdirectory( kfrontend )
