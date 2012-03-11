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

install( FILES konq_aboutpage.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
install( FILES konqaboutpage.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

install( FILES
    box-centre-konqueror.png top-right-konqueror.png
    lightning.png launch.html intro.html specs.html
    tips.html plugins.html plugins_rtl.html konq.css
  DESTINATION ${DATA_INSTALL_DIR}/konqueror/about )


##### konq_aboutpage (module) ###################

set( target konq_aboutpage )

kde_add_kpart( ${target} AUTOMOC
  SOURCES konq_aboutpage.cc
  LINK khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
