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

# FIXME there is used KDE_USE_FPIE

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
)


##### kdesud (executable) #######################

kde_add_executable( kdesud
  SOURCES kdesud.cpp repo.cpp lexer.cpp handler.cpp secure.cpp
  LINK kdesu-shared
  DESTINATION ${BIN_INSTALL_DIR}
)

install( CODE "execute_process( COMMAND chown root:${nogroup} \$ENV{DESTDIR}${_destination}/kdesud )" )
install( CODE "execute_process( COMMAND chmod 2755 \$ENV{DESTDIR}${_destination}/kdesud )" )
