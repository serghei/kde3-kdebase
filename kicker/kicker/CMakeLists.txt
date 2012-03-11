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

if( NOT BUILD_LIBKONQ )
  include( "${TDE_CMAKE_DIR}/libkonq.cmake" )
endif( NOT BUILD_LIBKONQ )

add_subdirectory( core )
add_subdirectory( ui )
add_subdirectory( buttons )

include_directories(
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR} )


##### other data ################################

install( FILES panel.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
install( FILES kcmkicker.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES kickerrc.upd DESTINATION ${DATA_INSTALL_DIR}/kconf_update )

install( PROGRAMS
    kicker-3.1-properSizeSetting.pl kicker-3.5-taskbarEnums.pl
    kicker-3.5-kconfigXTize.pl
  DESTINATION ${DATA_INSTALL_DIR}/kconf_update )


##### kicker-3.4-reverseLayout (executable) #####

kde_add_executable( kicker-3.4-reverseLayout
  SOURCES kicker-3.4-reverseLayout.cpp
  LINK kdecore-shared
  DESTINATION ${LIB_INSTALL_DIR}/kconf_update_bin
)


##### kicker (kdeinit) ##########################

configure_file( ${TEMPLATE_DUMMY_CPP} dummy.cpp COPYONLY )

kde_add_kdeinit_executable( kicker
  SOURCES dummy.cpp
  EMBED kicker_core-static
  LINK kicker_buttons-static kicker_ui-static dmctl-static kickermain-shared kabc-shared kutils-shared
)
