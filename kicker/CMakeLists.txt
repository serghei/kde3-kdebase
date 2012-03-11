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

project( kicker )


##### subdirectories ############################

add_subdirectory( libkicker )
add_subdirectory( taskmanager )
add_subdirectory( taskbar )
add_subdirectory( kicker )
add_subdirectory( extensions )
add_subdirectory( applets )
add_subdirectory( menuext )
add_subdirectory( proxy )
add_subdirectory( data )


##### install import cmake modules ###############

kde_install_export( )
