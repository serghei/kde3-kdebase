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

add_subdirectory( ftp )
add_subdirectory( web )


##### other data ################################

install( FILES
    .directory
  DESTINATION ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/remote )
