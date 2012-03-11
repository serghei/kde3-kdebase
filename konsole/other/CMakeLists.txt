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

add_subdirectory( icons )
add_subdirectory( wallpapers )

kde_install_icons( )

install( FILES
    BlackOnLightColor.schema BlackOnLightYellow.schema
    BlackOnWhite.schema Example.Schema GreenOnBlack.schema
    WhiteOnBlack.schema README.default.Schema syscolor.schema
    Linux.schema XTerm.schema vim.schema DarkPicture.schema
    LightPicture.schema Transparent.schema GreenTint.schema
    Transparent_MC.schema GreenTint_MC.schema Transparent_lightbg.schema
    Transparent_darkbg.schema LightPaper.schema linux.desktop
    mc.desktop shell.desktop su.desktop sumc.desktop
    linux.keytab vt100.keytab vt420pc.keytab x11r5.keytab
    solaris.keytab README.Schema README.KeyTab README.default.Keytab
    screen.desktop
  DESTINATION ${DATA_INSTALL_DIR}/konsole )

install( FILES
    2shells 3shells 4shells 5shells
  DESTINATION ${DATA_INSTALL_DIR}/konsole/profiles )
