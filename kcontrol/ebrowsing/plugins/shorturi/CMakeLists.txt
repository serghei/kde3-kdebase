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

install( FILES kshorturifilter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES kshorturifilterrc DESTINATION ${CONFIG_INSTALL_DIR} )


##### libkshorturifilter (module) ###############

kde_add_kpart( libkshorturifilter AUTOMOC
  SOURCES kshorturifilter.cpp kshorturifilter.skel
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
