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
  ${CMAKE_CURRENT_BINARY_DIR}/../libmediacommon
  ${CMAKE_CURRENT_SOURCE_DIR}/../libmediacommon
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES media.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_media (module) ########################

if( WITH_UDISKS2 )
  set_source_files_properties( managermodule.cpp PROPERTIES COMPILE_FLAGS "-DCOMPILE_UDISKS2BACKEND" )
endif( )

kde_add_kpart( kcm_media AUTOMOC
  SOURCES
    notifiermodule.cpp notifiermoduleview.ui serviceconfigdialog.cpp
    serviceview.ui mimetypelistboxitem.cpp managermodule.cpp
    managermoduleview.ui main.cpp
  LINK mediacommon-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
