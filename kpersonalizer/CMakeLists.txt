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

add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR} # for libkonq/konq_defaults.h
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

kde_install_icons( )
install( FILES kpersonalizer.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/System RENAME kpersonalizer.desktop )


##### kpersonalizer (executable) ################

kde_add_executable( kpersonalizer AUTOMOC
  SOURCES
    stylepreview.ui krefinepage.cpp kstylepage.cpp
    keyecandypage.cpp kospage.cpp kcountrypage.cpp
    kpersonalizer.cpp main.cpp kfindlanguage.cpp
    kcountrypagedlg.ui kospagedlg.ui keyecandypagedlg.ui
    kstylepagedlg.ui krefinepagedlg.ui ksysinfo.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
