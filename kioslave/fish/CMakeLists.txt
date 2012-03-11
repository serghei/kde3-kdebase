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

include( ConfigureChecks.cmake )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES fish.protocol nxfish.protocol DESTINATION ${SERVICES_INSTALL_DIR} )


##### kio_fish (module) #########################

add_custom_command( OUTPUT fishcode.h
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/genfishcode.pl ${CMAKE_CURRENT_SOURCE_DIR}/fish.pl > fishcode.h
  DEPENDS fish.pl )

set_property( SOURCE fish.cpp APPEND PROPERTY OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/fishcode.h )

set( target kio_fish )

kde_add_kpart( ${target} AUTOMOC
  SOURCES fish.cpp
  LINK kio-shared util
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
