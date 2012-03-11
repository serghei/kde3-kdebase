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

add_subdirectory( searchproviders )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES kuriikwsfilter.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES kuriikwsfilter.desktop kurisearchfilter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES searchprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


##### libkuriikwsfilter (module) ################

kde_add_kpart( libkuriikwsfilter AUTOMOC
  SOURCES
    kuriikwsfiltereng.cpp kuriikwsfilter.skel
    kuriikwsfilter.cpp searchprovider.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### libkurisearchfilter (module) ##############

kde_add_kpart( libkurisearchfilter AUTOMOC
  SOURCES
    kurisearchfilter.cpp kurisearchfilter.skel
    kuriikwsfiltereng.cpp ikwsopts.cpp searchproviderdlg.cpp
    searchprovider.cpp searchproviderdlg_ui.ui ikwsopts_ui.ui
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)