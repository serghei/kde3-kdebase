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

install( FILES
    AAAAAAGeneral.ad AAAMotif.ad AAATk.ad AAAXaw.ad AcroRead.ad
    Editres.ad Emacs.ad GV.ad ML.ad Nedit.ad Netscape.ad RVPlayer.ad
    WPerfect.ad XCalc.ad Xawtv.ad Xdvi.ad XTerm.ad XOsview.ad XV.ad Xpdf.ad
  DESTINATION ${DATA_INSTALL_DIR}/kdisplay/app-defaults )

install( PROGRAMS
    krdb
  DESTINATION ${BIN_INSTALL_DIR} )
