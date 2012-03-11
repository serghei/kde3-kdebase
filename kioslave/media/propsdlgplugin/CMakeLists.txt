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

install( FILES media_propsdlgplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### media_propsdlgplugin (module) #############

kde_add_kpart( media_propsdlgplugin AUTOMOC
  SOURCES
    propertiespage.cpp propertiespagegui.ui propsdlgshareplugin.cpp
  LINK mediacommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
