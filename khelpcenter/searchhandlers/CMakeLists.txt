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

set( prefix ${CMAKE_INSTALL_PREFIX} )
configure_file( khc_docbookdig.pl.cmake khc_docbookdig.pl @ONLY )
configure_file( khc_htdig.pl.cmake khc_htdig.pl @ONLY )

install( FILES
    htdig.desktop man.desktop docbook.desktop
  DESTINATION ${DATA_INSTALL_DIR}/khelpcenter/searchhandlers )

install( PROGRAMS
    ${CMAKE_CURRENT_BINARY_DIR}/khc_htdig.pl khc_htsearch.pl
    ${CMAKE_CURRENT_BINARY_DIR}/khc_docbookdig.pl khc_beagle_search.pl
     khc_mansearch.pl  khc_beagle_index.pl
  DESTINATION ${BIN_INSTALL_DIR} )

install( FILES
    htdig_long.html
  DESTINATION ${DATA_INSTALL_DIR}/khelpcenter/searchhandlers/htdig )
