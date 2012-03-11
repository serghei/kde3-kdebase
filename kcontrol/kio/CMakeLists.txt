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

add_subdirectory( uasproviders )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    cookies.desktop useragent.desktop cache.desktop
    lanbrowser.desktop netpref.desktop proxy.desktop
  DESTINATION ${XDG_APPS_INSTALL_DIR} )

install( FILES uninstall.desktop
  RENAME smb.desktop
  DESTINATION ${APPS_INSTALL_DIR}/Settings/WebBrowsing )

install( FILES socks.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR} )
install( FILES uasprovider.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )
install( FILES smb.desktop DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### kcm_kio (module) ##########################

kde_add_kpart( kcm_kio AUTOMOC
  SOURCES
    policydlg.cpp kcookiesmain.cpp kcookiespolicies.cpp
      main.cpp smbrodlg.cpp useragentdlg.cpp kproxydlgbase.cpp
      kmanualproxydlg.cpp kenvvarproxydlg.cpp kproxydlg.cpp
      kcookiesmanagement.cpp fakeuaprovider.cpp uagentproviderdlg.cpp
      netpref.cpp cache.cpp ksaveioconfig.cpp socks.cpp socksbase.ui
      kproxydlg_ui.ui kcookiesmanagementdlg_ui.ui kcookiespoliciesdlg_ui.ui
      policydlg_ui.ui useragentdlg_ui.ui uagentproviderdlg_ui.ui
      manualproxy_ui.ui envvarproxy_ui.ui cache_ui.ui
  LINK kio-shared kutils-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
