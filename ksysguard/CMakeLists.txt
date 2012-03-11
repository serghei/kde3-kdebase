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

# FIXME no support for lm_sensors yet

add_subdirectory( CContLib )
add_subdirectory( ksysguardd )
add_subdirectory( gui )
add_subdirectory( pics )
add_subdirectory( example )

kde_install_icons( ksysguard )
