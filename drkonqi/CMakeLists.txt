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

add_subdirectory( presets )
add_subdirectory( pics )
add_subdirectory( debuggers )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### drkonqi (executable) ######################

kde_add_executable( drkonqi AUTOMOC
  SOURCES
    krashdcopinterface.skel main.cpp debugger.cpp
    krashconf.cpp drbugreport.cpp backtrace.cpp toplevel.cpp
  LINK kio-shared
  DESTINATION ${BIN_INSTALL_DIR}
)
