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

if( WITH_XINERAMA )
  check_include_file( "X11/extensions/Xinerama.h" X11_EXTENSIONS_XINERAMA_H )
  if( X11_EXTENSIONS_XINERAMA_H )
    check_library_exists( Xinerama XineramaQueryExtension "" HAVE_XINERAMA )
  endif( X11_EXTENSIONS_XINERAMA_H )
  if( NOT HAVE_XINERAMA )
    kde_message_fatal( "xinerama are requested, but not found on your system" )
  endif( NOT HAVE_XINERAMA )
  set( XINERAMA_LIBRARY Xinerama )
endif( WITH_XINERAMA )


include_directories(
  ${CMAKE_BINARY_DIR}
  ${QT_INCLUDE_DIR}
)


##### ksplashsimple (executable) ################

kde_add_executable( ksplashsimple
  SOURCES main.cpp
  LINK ${TQT_LIBRARIES} X11 ${XINERAMA_LIBRARY}
  DESTINATION ${BIN_INSTALL_DIR}
)
