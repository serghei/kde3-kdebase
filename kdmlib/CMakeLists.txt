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
  ${CMAKE_SOURCE_DIR}/kdm/kfrontend
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


if( BUILD_KDM )


##### headers ###################################

install( FILES kgreeterplugin.h DESTINATION ${INCLUDE_INSTALL_DIR} )


##### kgreet_classic (module) ###################

kde_add_kpart( kgreet_classic AUTOMOC
  SOURCES kgreet_classic.cpp
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kgreet_winbind (module) ###################

kde_add_kpart( kgreet_winbind AUTOMOC
  SOURCES kgreet_winbind.cpp
  LINK kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)



endif( BUILD_KDM )


##### dmctl (static) ############################

if( BUILD_KICKER OR BUILD_KDESKTOP OR BUILD_KDM OR BUILD_KSMSERVER )

  kde_add_library( dmctl STATIC_PIC
    SOURCES dmctl.cpp
    LINK Xau
  )

endif( )
