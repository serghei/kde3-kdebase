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
    hdd_mounted_decrypted.desktop hdd_unmounted_encrypted.desktop
    hdd_unmounted_decrypted.desktop removable_mounted.desktop
    removable_unmounted.desktop removable_mounted_decrypted.desktop
    removable_unmounted_encrypted.desktop removable_unmounted_decrypted.desktop
    cdrom_mounted.desktop cdrom_unmounted.desktop cdrom_mounted_decrypted.desktop
    cdrom_unmounted_encrypted.desktop cdrom_unmounted_decrypted.desktop
    dvd_mounted.desktop dvd_unmounted.desktop dvd_mounted_decrypted.desktop
    dvd_unmounted_encrypted.desktop dvd_unmounted_decrypted.desktop
    cdwriter_mounted.desktop cdwriter_unmounted.desktop
    cdwriter_mounted_decrypted.desktop cdwriter_unmounted_encrypted.desktop
    cdwriter_unmounted_decrypted.desktop smb_mounted.desktop
    smb_unmounted.desktop nfs_mounted.desktop nfs_unmounted.desktop
    audiocd.desktop dvdvideo.desktop blankcd.desktop blankdvd.desktop
    svcd.desktop vcd.desktop gphoto2camera.desktop camera_mounted.desktop
    camera_unmounted.desktop mydocuments.desktop mycomputer.desktop
    mynetworkplaces.desktop printers.desktop trash.desktop webbrowser.desktop
  DESTINATION ${MIME_INSTALL_DIR}/media )
