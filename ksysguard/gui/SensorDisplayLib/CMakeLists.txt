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
  ${CMAKE_SOURCE_DIR}/ksysguard/gui/ksgrd
  ${CMAKE_SOURCE_DIR}/ksysguard/gui
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### sensordisplays (static) ###################

kde_add_library( sensordisplays STATIC_PIC AUTOMOC
  SOURCES
    SensorDisplay.cc BarGraph.cc DancingBars.cc DancingBarsSettings.cc
    DummyDisplay.cc FancyPlotter.cc FancyPlotterSettings.cc
    ListView.cc LogFile.cc MultiMeter.cc MultiMeterSettings.cc
    ProcessController.cc ProcessList.cc ReniceDlg.cc
    SensorLogger.cc SensorLoggerDlg.cc SensorLoggerSettings.cc
    ListViewSettings.cc SignalPlotter.cc ListViewSettingsWidget.ui
    LogFileSettings.ui MultiMeterSettingsWidget.ui SensorLoggerDlgWidget.ui
    SensorLoggerSettingsWidget.ui
  LINK ksgrd-shared kio-shared
)
