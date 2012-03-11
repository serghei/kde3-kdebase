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
    linkProgram.desktop linkURL.desktop linkFloppy.desktop
    linkHD.desktop linkCDROM.desktop Directory.desktop
    TextFile.desktop HTMLFile.desktop linkZIP.desktop
    linkDVDROM.desktop linkCAMERA.desktop linkNFS.desktop
    linkCDWRITER.desktop linkMO.desktop
  DESTINATION ${TEMPLATES_INSTALL_DIR} )

install( FILES
    Program.desktop URL.desktop Floppy.desktop HD.desktop
    CDROM-Device.desktop TextFile.txt HTMLFile.html
    ZIP-Device.desktop DVDROM-Device.desktop CAMERA-Device.desktop
    NFS.desktop CDWRITER-Device.desktop MO-Device.desktop
  DESTINATION ${TEMPLATES_INSTALL_DIR}/.source )

kde_install_empty_directory( ${TEMPLATES_INSTALL_DIR}/.source/emptydir )
