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

install( FILES KRandom.desktop KBlankscreen.desktop DESTINATION ${APPS_INSTALL_DIR}/System/ScreenSavers )


##### krandom (executable) ######################

kde_add_executable( krandom.kss AUTOMOC
  SOURCES random.cpp
  LINK kdeui-shared ${GL_LIBRARY}
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kblankscrn (executable) ###################

kde_add_executable( kblankscrn.kss AUTOMOC
  SOURCES blankscrn.cpp
  LINK kscreensaver-shared kdeui-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
