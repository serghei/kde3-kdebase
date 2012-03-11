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

option( WITH_DPMS "Enable DPMS power saving" ON )

if( WITH_DPMS )

  include( CheckCSourceCompiles )

  set( CMAKE_REQUIRED_LIBRARIES Xext )

  check_c_source_compiles("
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <arpa/nameser.h>
    #include <resolv.h>
    #include <X11/Xproto.h>
    #include <X11/X.h>
    #include <X11/Xlib.h>
    #include <X11/extensions/dpms.h>
    int main()
    { return DPMSSetTimeouts( 0, 0, 0, 0 ); }"
    HAVE_DPMS )

  if( NOT HAVE_DPMS )
    kde_message_fatal( "DPMS support are requested, but is not detected on you system.\n libXext (and its devel package) are installed?" )
  endif()

  set( XEXT_LIBRARY Xext )

endif()

add_subdirectory( pics )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### other data ################################

install( FILES energy.desktop DESTINATION ${APPS_INSTALL_DIR}/.hidden )


##### kcm_energy (module) #######################

kde_add_kpart( kcm_energy AUTOMOC
  SOURCES energy.cpp
  LINK kio-shared ${XEXT_LIBRARY}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
