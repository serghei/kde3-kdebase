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

add_subdirectory( config )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/kwin/lib
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES keramik.desktop DESTINATION ${DATA_INSTALL_DIR}/kwin )


##### kwin3_keramik (module) ####################

set_source_files_properties( keramik.cpp
  PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tiles.h )

kde_add_kpart( kwin3_keramik AUTOMOC
  SOURCES keramik.cpp
  LINK kdecorations-shared kdeui-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### tiles.h (header) ##########################

set( pics
  pics/caption-large-left.png pics/caption-small-right.png
  pics/titlebar-center.png pics/titlebutton-square.png
  pics/border-left.png pics/caption-large-right.png
  pics/grabbar-center.png pics/titlebar-left.png
  pics/border-right.png pics/caption-small-center.png
  pics/grabbar-left.png pics/titlebar-right.png
  pics/caption-large-center.png pics/caption-small-left.png
  pics/grabbar-right.png pics/titlebutton-round.png
  pics/bottom-left.png pics/bottom-right.png pics/bottom-center.png
  pics/titlebutton-square-large.png pics/titlebutton-square-huge.png
  pics/titlebutton-round-large.png pics/titlebutton-round-huge.png )

add_custom_command( OUTPUT tiles.h
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/embedtool ${pics}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/embedtool ${pics} )

kde_add_executable( embedtool
  SOURCES embedtool.cpp
  LINK ${QT_LIBRARY}
)
