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

add_subdirectory( Applications )
add_subdirectory( Manpages )
add_subdirectory( Tutorials )
add_subdirectory( Scrollkeeper )

install( FILES
  applets.desktop contact.desktop faq.desktop info.desktop
  links.desktop support.desktop userguide.desktop welcome.desktop
  kicmodules.desktop kcontrolmodules.desktop konqplugins.desktop
  kioslaves.desktop
  DESTINATION ${DATA_INSTALL_DIR}/khelpcenter/plugins )
