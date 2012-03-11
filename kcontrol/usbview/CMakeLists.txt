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
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kcmusb.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

if( WITH_USBIDS )
  set_source_files_properties( usbdb.cpp PROPERTIES COMPILE_FLAGS -DUSBIDS_FILE=\\"${WITH_USBIDS}\\" )
else( )
  install( FILES usb.ids DESTINATION ${DATA_INSTALL_DIR} )
endif( )


##### kcm_usb (module) ##########################

kde_add_kpart( kcm_usb AUTOMOC
  SOURCES
    kcmusb.cpp usbdevices.cpp usbdb.cpp
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
