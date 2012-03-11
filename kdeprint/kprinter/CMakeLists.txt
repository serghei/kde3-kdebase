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

include( CheckFunctionExists )
check_function_exists( sigset HAVE_SIGACTION )
check_function_exists( sigaction HAVE_SIGSET )

add_definitions( -D_KDEPRINT_COMPILE )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kprinter (kdeinit) ########################

kde_add_kdeinit_executable( kprinter AUTOMOC
  SOURCES main.cpp printwrapper.cpp
  LINK kdeprint-shared
)
