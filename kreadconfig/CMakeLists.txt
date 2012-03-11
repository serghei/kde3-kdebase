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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kreadconfig (executable) ##################

kde_add_executable( kreadconfig
  SOURCES kreadconfig.cpp
  LINK kdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### kwriteconfig (executable) #################

kde_add_executable( kwriteconfig
  SOURCES kwriteconfig.cpp
  LINK kdecore-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
