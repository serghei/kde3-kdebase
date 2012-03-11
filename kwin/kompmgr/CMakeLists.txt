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


if( NOT WITH_XRENDER )
  kde_message_fatal( "xrender support is needed to build kompmgr.\n Pass -DWITH_XRENDER=ON to cmake arguments." )
elseif( NOT WITH_XFIXES )
  kde_message_fatal( "xfixes support is needed to build kompmgr.\n Pass -DWITH_XFIXES=ON to cmake arguments." )
elseif( NOT WITH_XDAMAGE )
  kde_message_fatal( "xdamage support is needed to build kompmgr.\n Pass -DWITH_XDAMAGE=ON to cmake arguments." )
elseif( NOT WITH_XEXT )
  kde_message_fatal( "xext support is needed to build kompmgr.\n Pass -DWITH_XEXT=ON to cmake arguments." )
endif( )


include_directories(
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kompmgr (executable) #######################

kde_add_executable( kompmgr
  SOURCES kompmgr.c
  LINK m ${XRENDER_LIBRARIES} ${XFIXES_LIBRARIES} ${XDAMAGE_LIBRARIES} ${XEXT_LIBRARIES} ${XCOMPOSITE_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)
