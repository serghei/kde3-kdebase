#################################################
#
#  (C) 2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

kde_import( kicker )

add_subdirectory( icons )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../kicker/libkicker
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${UPOWER_GLIB_INCLUDE_DIRS}
)


##### other data ################################

install( FILES kpowermanager.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kicker/applets )


##### kpowermanager_panelapplet (module) ########

if( WITH_XRANDR )
	set( backlight_SRC backlight.cpp )
endif( )

kde_add_kpart( kpowermanager_panelapplet AUTOMOC
  SOURCES panelapplet.cpp upowerbackend.cpp tooltip.cpp ${backlight_SRC}
  LINK kickermain-shared ${XRANDR_LIBRARIES} ${UPOWER_GLIB_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
