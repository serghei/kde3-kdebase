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
  ${CMAKE_SOURCE_DIR}/libkonq
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES keditbookmarksui.rc keditbookmarks-genui.rc DESTINATION ${DATA_INSTALL_DIR}/keditbookmarks )
install( FILES uninstall.desktop DESTINATION ${APPS_INSTALL_DIR}/Internet/keditbookmarks.desktop )
install( FILES keditbookmarks.kcfg DESTINATION ${KCFG_INSTALL_DIR} )
kde_install_icons( )


##### kbookmarkmerger ###########################

set( target kbookmarkmerger )

kde_add_executable( ${target}
  SOURCES kbookmarkmerger.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)


##### keditbookmarks (kdeinit) ##################

set( target keditbookmarks )

set( ${target}_SRCS AUTOMOC
  main.cpp listview.cpp toplevel.cpp actionsimpl.cpp
  commands.cpp importers.cpp dcop.skel_ng dcop.cpp
  bookmarkiterator.cpp testlink.cpp favicons.cpp
  updater.cpp exporters.cpp ${KDE_INCLUDE_DIR}/kbookmarknotifier.stub
  bookmarkinfo.cpp kebsearchline.cpp settings.kcfgc
)

kde_add_kdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK konq-shared
)
