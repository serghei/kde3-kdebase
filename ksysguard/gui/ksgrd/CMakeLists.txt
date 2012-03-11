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
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### headers ###################################

install( FILES
    HostConnector.h SensorAgent.h SensorClient.h
    SensorManager.h SensorShellAgent.h SensorSocketAgent.h
    StyleEngine.h StyleSettings.h TimerSettings.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/ksgrd )


##### ksgrd (shared) ############################

kde_add_library( ksgrd SHARED AUTOMOC
  SOURCES
    HostConnector.cc SensorAgent.cc SensorManager.cc
    SensorShellAgent.cc SensorSocketAgent.cc StyleEngine.cc
    StyleSettings.cc TimerSettings.cc
  VERSION 1.2.0
  LINK kdeui-shared
  DESTINATION ${LIB_INSTALL_DIR}
)
