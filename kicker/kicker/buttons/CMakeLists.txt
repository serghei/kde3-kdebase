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
  ${CMAKE_BINARY_DIR}/kicker/libkicker
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kicker/libkicker
  ${CMAKE_SOURCE_DIR}/kicker/kicker/core
  ${CMAKE_SOURCE_DIR}/kicker/kicker/ui
  ${CMAKE_SOURCE_DIR}/libkonq
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES
    bookmarks.desktop browser.desktop desktop.desktop
    exec.desktop kmenu.desktop windowlist.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kicker/builtins )


##### kicker_buttons (static) ###################

set( target kicker_buttons )

set( ${target}_SRCS
  servicebutton.cpp bookmarksbutton.cpp browserbutton.cpp
  desktopbutton.cpp extensionbutton.cpp
  kbutton.cpp nonkdeappbutton.cpp servicemenubutton.cpp
  urlbutton.cpp windowlistbutton.cpp
)

kde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  DEPENDENCIES kicker_core-static kicker-static kicker-shared
)
