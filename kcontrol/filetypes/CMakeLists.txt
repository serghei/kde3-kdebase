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

install( FILES filetypes.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )


##### kcm_filetypes (module) ####################

kde_add_kpart( kcm_filetypes AUTOMOC
  SOURCES control.cpp
  LINK filetypes-static kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### filetypes (static) ########################

kde_add_library( filetypes STATIC_PIC AUTOMOC
  SOURCES
    filetypesview.cpp filetypedetails.cpp filegroupdetails.cpp
    kservicelistwidget.cpp typeslistitem.cpp newtypedlg.cpp
    kserviceselectdlg.cpp
)

##### keditfiletype (executable) ######################

kde_add_executable( keditfiletype AUTOMOC
  SOURCES keditfiletype.cpp
  LINK filetypes-static kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
