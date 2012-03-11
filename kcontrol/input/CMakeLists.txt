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

if( WITH_XCURSOR )
  add_subdirectory( xcursor )
  include_directories( xcursor )
else( )
  add_subdirectory( core )
  include_directories( core )
endif( )

add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    mouse.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

install( FILES
    cursor_large_black.pcf.gz cursor_large_white.pcf.gz
    cursor_small_white.pcf.gz
  DESTINATION ${DATA_INSTALL_DIR}/kcminput )

install( FILES
    mouse_cursor_theme.upd
  DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )


##### kcm_input (module) ########################

kde_add_kpart( kcm_input AUTOMOC
  SOURCES
    mouse.cpp kmousedlg.ui main.cpp logitechmouse.cpp
    logitechmouse_base.ui
  LINK themepage-static kio-shared ${XCURSOR_LIBRARIES} ${XRENDER_LIBRARIES} ${LIBUSB_LIBRARY}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kapplymousetheme (executable) #############

kde_add_executable( kapplymousetheme
  SOURCES
    kapplymousetheme.cpp
  LINK ${XCURSOR_LIBRARIES} X11
  DESTINATION ${BIN_INSTALL_DIR}
)
