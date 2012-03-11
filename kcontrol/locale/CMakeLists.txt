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

add_subdirectory( default )
add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES language.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_locale (module) #######################

kde_add_kpart( kcm_locale AUTOMOC
  SOURCES
    main.cpp toplevel.cpp kcmlocale.cpp klocalesample.cpp
    localenum.cpp localemon.cpp localetime.cpp localeother.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
