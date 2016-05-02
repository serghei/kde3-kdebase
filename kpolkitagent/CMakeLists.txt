#################################################
#
#  (C) 2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include( ConfigureChecks.cmake )

add_definitions( -D_POLKIT_AGENT_COMPILATION )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${KDE_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${POLKIT_AGENT_INCLUDE_DIRS}
)


##### other data ################################

install( FILES polkit_agent.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


##### kded_polkit_agent (module) #################

kde_add_kpart( kded_polkit_agent AUTOMOC
  SOURCES
    polkit_agent.cpp dialog.cpp authenticator.cpp listener.cpp
  LINK
    kdeinit_kded-shared ${POLKIT_AGENT_LIBRARIES} ${GLIB_LIBRARIES} ${GOBJECT_LIBRARIES}
  DESTINATION ${PLUGIN_INSTALL_DIR}
)
