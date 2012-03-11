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
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES x-konsole.desktop DESTINATION ${MIME_INSTALL_DIR}/application )


##### libkonsolepart (module) ###################

set( target libkonsolepart )

set( ${target}_SRCS
  TEPty.cpp BlockArray.cpp konsole_part.cpp schema.cpp
  session.cpp zmodem_dialog.cpp TEWidget.cpp TEmuVt102.cpp
  TEScreen.cpp konsole_wcwidth.cpp TEmulation.cpp
  TEHistory.cpp keytrans.cpp konsoleiface.skel sessioniface.skel
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kparts-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### kded_kwrited (module) #####################

set( target kded_kwrited )

set( ${target}_SRCS
  kwrited.cpp TEPty.cpp kwrited.skel
)

kde_add_kpart( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeinit_kded-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### konsole (kdeinit) #########################

set( target konsole )

set( ${target}_SRCS
  TEPty.cpp BlockArray.cpp main.cpp konsole.cpp
  schema.cpp session.cpp TEWidget.cpp TEmuVt102.cpp
  TEScreen.cpp TEmulation.cpp TEHistory.cpp keytrans.cpp
  konsoleiface.skel sessioniface.skel konsole_wcwidth.cpp
  konsolebookmarkhandler.cpp konsolebookmarkmenu.cpp
  zmodem_dialog.cpp printsettings.cpp
)

kde_add_kdeinit_executable( ${target} AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK kdeprint-shared kio-shared ${XRENDER_LIBRARIES} dl
)
