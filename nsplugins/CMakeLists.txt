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

add_subdirectory( viewer )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES khtml_plugins.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES nspluginpart.rc DESTINATION ${DATA_INSTALL_DIR}/plugin )


##### libnsplugin (module) ######################

kde_add_kpart( libnsplugin AUTOMOC
  SOURCES
    viewer/NSPluginClassIface.stub
    NSPluginCallbackIface.skel
    plugin_part.cpp nspluginloader.cpp
  LINK kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kcm_nsplugins (module) ####################

kde_add_kpart( kcm_nsplugins AUTOMOC
  SOURCES kcm_nsplugins.cpp
  LINK pluginpaths-static kdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### nspluginscan (executable) #################

kde_add_executable( nspluginscan AUTOMOC
  SOURCES pluginscan.cpp
  LINK pluginpaths-static kio-shared Xt
  DESTINATION ${BIN_INSTALL_DIR}
)


##### pluginpaths (static) ######################

kde_add_library( pluginpaths STATIC_PIC
  SOURCES plugin_paths.cpp
)
