#################################################
#
#  (C) 2012-2014 Serghei Amelian
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

if( WITH_LOGIND )
    set( KDEDBUS_LIBRARY kdedbus-shared )
endif( )

kde_add_kpart( kpowermanager_panelapplet AUTOMOC
  SOURCES panelapplet.cpp upowerbackend.cpp tooltip.cpp cpumonitor.cpp ${backlight_SRC}
  LINK kickermain-shared ${KDEDBUS_LIBRARY} ${XRANDR_LIBRARIES} ${UPOWER_GLIB_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
