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

add_subdirectory( Arcade )
add_subdirectory( Board )
add_subdirectory( Card )
add_subdirectory( Emulators )
add_subdirectory( TacticStrategy )
add_subdirectory( Roguelikes )
add_subdirectory( Toys )

install( FILES
    penguin-command.desktop xshipwars.desktop
    Clanbomber.desktop defendguin.desktop GTali.desktop
  DESTINATION ${DATA_INSTALL_DIR}/kappfinder/apps/Games )
