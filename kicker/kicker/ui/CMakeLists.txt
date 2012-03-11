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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/kicker/kicker/core
  ${CMAKE_BINARY_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/kicker/core
  ${CMAKE_SOURCE_DIR}/kicker/kicker/buttons
  ${CMAKE_SOURCE_DIR}/kdmlib
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kicker_ui (static) ########################

kde_add_library( kicker_ui STATIC_PIC AUTOMOC
  SOURCES
    addbutton_mnu.cpp appletitem.ui appletview.ui addapplet.cpp
    addapplet_mnu.cpp appletop_mnu.cpp browser_mnu.cpp client_mnu.cpp
    dirdrop_mnu.cpp nonKDEButtonSettings.ui exe_dlg.cpp
    k_mnu.cpp k_mnu.skel quickbrowser_mnu.cpp service_mnu.cpp
    addextension_mnu.cpp extensionop_mnu.cpp recentapps.cpp
    browser_dlg.cpp removeapplet_mnu.cpp removeextension_mnu.cpp
    removecontainer_mnu.cpp removebutton_mnu.cpp popupmenutitle.cpp
    hidebutton.cpp addappletvisualfeedback.cpp
  DEPENDENCIES kicker_core-static kickermain-static
)
