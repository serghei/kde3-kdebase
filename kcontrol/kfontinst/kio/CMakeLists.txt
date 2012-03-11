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
  ${CMAKE_SOURCE_DIR}/kcontrol/kfontinst/lib
  ${CMAKE_SOURCE_DIR}/kcontrol/fonts
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES fonts.protocol DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES fonts.desktop DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services )
install( FILES folder.desktop system-folder.desktop package.desktop DESTINATION ${MIME_INSTALL_DIR}/fonts )


##### kio_fonts (module) #######################

kde_add_kpart( kio_fonts AUTOMOC
  SOURCES KioFonts.cpp
  LINK kxftconfig-static kfontinst-shared kdesu-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
