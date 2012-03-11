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

##### check for openexr #########################

if( WITH_OPENEXR )
  pkg_search_module( OPENEXR OpenEXR )
  if( NOT OPENEXR_FOUND )
    message(FATAL_ERROR "\nopenexr are requested, but not found on your system" )
  endif( NOT OPENEXR_FOUND )
  set( HAVE_EXR 1 )
endif( WITH_OPENEXR )


##### compiler/linker flags #####################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${XCURSOR_INCLUDE_DIRS}
  ${OPENEXR_INCLUDE_DIRS}
)


##### other data ################################

install( FILES
    thumbnail.protocol imagethumbnail.desktop textthumbnail.desktop
    htmlthumbnail.desktop djvuthumbnail.desktop
  DESTINATION ${SERVICES_INSTALL_DIR} )

install( FILES thumbcreator.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )


##### kio_thumbnail (module) ####################

set( target kio_thumbnail )

kde_add_kpart( ${target} AUTOMOC
  SOURCES thumbnail.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### imagethumbnail (module) ####################

set( target imagethumbnail )

kde_add_kpart( ${target} AUTOMOC
  SOURCES imagecreator.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### textthumbnail (module) ####################

set( target textthumbnail )

kde_add_kpart( ${target} AUTOMOC
  SOURCES textcreator.cpp
  LINK kio-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### htmlthumbnail (module) ####################

set( target htmlthumbnail )

kde_add_kpart( ${target} AUTOMOC
  SOURCES htmlcreator.cpp
  LINK khtml-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### djvuthumbnail (module) ####################

set( target djvuthumbnail )

kde_add_kpart( ${target} AUTOMOC
  SOURCES djvucreator.cpp
  LINK kdecore-shared
  DESTINATION ${PLUGIN_INSTALL_DIR}
)


##### cursorthumbnail (module) ##################

if( WITH_XCURSOR )

  set( target cursorthumbnail )

  kde_add_kpart( ${target} AUTOMOC
    SOURCES cursorcreator.cpp
    LINK kdecore-shared ${XCURSOR_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )

  install( FILES cursorthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

endif( WITH_XCURSOR )


##### exrthumbnail (module) #####################

if( WITH_OPENEXR )

  set( target exrthumbnail )

  kde_add_kpart( ${target} AUTOMOC
    SOURCES exrcreator.cpp
    LINK kdecore-shared ${OPENEXR_LIBRARIES}
    DESTINATION ${PLUGIN_INSTALL_DIR}
  )

  install( FILES exrthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

endif( WITH_OPENEXR )
