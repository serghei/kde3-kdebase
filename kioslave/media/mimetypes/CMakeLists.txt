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

##### other data ################################

install( FILES
    floppy_mounted.desktop floppy_unmounted.desktop
    floppy5_unmounted.desktop floppy5_mounted.desktop
    zip_mounted.desktop zip_unmounted.desktop
    hdd_mounted.desktop hdd_unmounted.desktop
    removable_mounted.desktop
    removable_unmounted.desktop
    cdrom_mounted.desktop cdrom_unmounted.desktop
    dvd_mounted.desktop dvd_unmounted.desktop
    cdwriter_mounted.desktop cdwriter_unmounted.desktop
    smb_mounted.desktop
    smb_unmounted.desktop nfs_mounted.desktop nfs_unmounted.desktop
    audiocd.desktop dvdvideo.desktop blankcd.desktop blankdvd.desktop
    svcd.desktop vcd.desktop gphoto2camera.desktop camera_mounted.desktop
    camera_unmounted.desktop
  DESTINATION ${MIME_INSTALL_DIR}/media )
