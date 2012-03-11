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

##### check for fontconfig ######################

pkg_search_module( FONTCONFIG fontconfig )
if( NOT FONTCONFIG_FOUND )
    message(FATAL_ERROR "\nfontconfig are required, but not found on your system" )
endif( NOT FONTCONFIG_FOUND )
set( HAVE_FONTCONFIG 1 CACHE INTERNAL "" FORCE )


##### check for freetype2 #######################

pkg_search_module( FREETYPE freetype2 )
if( NOT FREETYPE_FOUND )
    message(FATAL_ERROR "\nfreetype2 are required, but not found on your system" )
endif( NOT FREETYPE_FOUND )
set( HAVE_FREETYPE2 1 CACHE INTERNAL "" FORCE  )


##### compiler/linker settings ##################

add_definitions( -D_LARGEFILE64_SOURCE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${FREETYPE_INCLUDE_DIRS}
)


##### other data ################################

install( FILES fonts.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_fonts (module) ########################

kde_add_kpart( kcm_fonts AUTOMOC
  SOURCES fonts.cpp
  LINK kxftconfig-static krdb-static kdeui-shared ${FONTCONFIG_LIBRARIES} ${FREETYPE_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kxftconfig (static) #######################

kde_add_library( kxftconfig STATIC_PIC
  SOURCES kxftconfig.cpp
)
