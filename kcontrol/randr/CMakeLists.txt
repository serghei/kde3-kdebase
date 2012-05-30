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

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES krandrtray.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES randr.desktop DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### randrinternal (static) ####################

kde_add_library( randrinternal STATIC_PIC AUTOMOC
  SOURCES ktimerdialog.cpp randr.cpp
)


##### kcm_gandr (module) ########################

kde_add_kpart( kcm_randr AUTOMOC
  SOURCES krandrmodule.cpp
  LINK randrinternal-static kdeui-shared ${XRANDR_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### krandrtray (executable) ###################

kde_add_executable( krandrtray AUTOMOC
  SOURCES
    main.cpp krandrtray.cpp krandrapp.cpp
    krandrpassivepopup.cpp
  LINK randrinternal-static kutils-shared ${XRANDR_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
