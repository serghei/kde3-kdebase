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

install( FILES localdomainurifilter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


##### liblocaldomainurifilter (module) ##########

kde_add_kpart( liblocaldomainurifilter AUTOMOC
  SOURCES localdomainurifilter.cpp localdomainurifilter.skel
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### klocaldomainurifilterhelper (executable ) #

kde_add_executable( klocaldomainurifilterhelper
  SOURCES klocaldomainurifilterhelper.c
  LINK
  DESTINATION ${BIN_INSTALL_DIR}
)
