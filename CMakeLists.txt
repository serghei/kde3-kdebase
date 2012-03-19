#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
################################################

cmake_minimum_required( VERSION 2.8 )


##### general package setup #####################

project( kdebase )

set( PACKAGE kdebase )
set( VERSION "3.6" )


##### features ##################################

option( WITH_ALL_FEATURES "Enable all possible features" OFF )
option( WITH_SASL "Enable SASL support" ${WITH_ALL_FEATURES} )
option( WITH_LDAP "Enable LDAP support" ${WITH_ALL_FEATURES} )
option( WITH_SAMBA "Enable SAMBA support" ${WITH_ALL_FEATURES} )
option( WITH_OPENEXR "Enable openexr support" ${WITH_ALL_FEATURES} )
option( WITH_XCOMPOSITE "Enable xcomposite support" ${WITH_ALL_FEATURES} )
option( WITH_XCURSOR "Enable xcursor support" ${WITH_ALL_FEATURES} )
option( WITH_XFIXES "Enable xfixes support" ${WITH_ALL_FEATURES} )
option( WITH_XTST "Enable xtst support" ${WITH_ALL_FEATURES} )
option( WITH_XRANDR "Enable xrandr support" ${WITH_ALL_FEATURES} )
option( WITH_XRENDER "Enable xrender support" ${WITH_ALL_FEATURES} )
option( WITH_XDAMAGE "Enable xdamage support" ${WITH_ALL_FEATURES} )
option( WITH_XEXT "Enable xext support" ${WITH_ALL_FEATURES} )
option( WITH_LIBUSB "Enable control of some mouse models through libusb" ${WITH_ALL_FEATURES} )
option( WITH_LIBRAW1394 "Enable visualization of ieee1394 devices through libraw1394" ${WITH_ALL_FEATURES} )
option( WITH_PAM "kdm should use PAM" ${WITH_ALL_FEATURES} )
option( WITH_SHADOW "kdm should use shadow passwords" ${WITH_ALL_FEATURES} )
option( WITH_XDMCP "xdmcp support for kdm" ${WITH_ALL_FEATURES} )
option( WITH_XINERAMA "Enable xinerama extension support" ${WITH_ALL_FEATURES} )
option( WITH_ARTS "Enable aRts support" ${WITH_ALL_FEATURES} )
option( WITH_I8K "Enable Dell laptop support (ksysguard)" ${WITH_ALL_FEATURES} )
option( WITH_SUDO_KDESU_BACKEND "Use sudo as backend for kdesu (default is su)" ${WITH_ALL_FEATURES} )


##### user requested modules ####################

option( BUILD_ALL "Build all" OFF )
option( BUILD_L10N "Build l10n" ${BUILD_ALL} )
option( BUILD_PICS "Build pics" ${BUILD_ALL} )
option( BUILD_APPLNK "Build applnk" ${BUILD_ALL} )
option( BUILD_DOC "Build doc" ${BUILD_ALL} )
option( BUILD_LIBKONQ "Build libkonq" ${BUILD_ALL} )
option( BUILD_KCMINIT "Build kcminit" ${BUILD_ALL} )
option( BUILD_KCONTROL "Build kcontrol" ${BUILD_ALL} )
option( BUILD_KWIN "Build kwin" ${BUILD_ALL} )
option( BUILD_KATE "Build kate" ${BUILD_ALL} )
option( BUILD_KONQUEROR "Build konqueror" ${BUILD_ALL} )
option( BUILD_KIOSLAVES "Build kioslaves" ${BUILD_ALL} )
option( BUILD_KICKER "Build kicker" ${BUILD_ALL} )
option( BUILD_KDESKTOP "Build kdesktop" ${BUILD_ALL} )
option( BUILD_KONSOLE "Build konsole" ${BUILD_ALL} )
option( BUILD_KLIPPER "Build klipper" ${BUILD_ALL} )
option( BUILD_KMENUEDIT "Build kmenuedit" ${BUILD_ALL} )
option( BUILD_KDEPRINT "Build kdeprint" ${BUILD_ALL} )
option( BUILD_KPERSONALIZER "Build kpersonalizer" ${BUILD_ALL} )
option( BUILD_KDEPASSWD "Build kdepasswd" ${BUILD_ALL} )
option( BUILD_KTIP "Build ktip" ${BUILD_ALL} )
option( BUILD_KDEEJECT "Build kdeeject" ${BUILD_ALL} )
option( BUILD_KDIALOG "Build kdialog" ${BUILD_ALL} )
option( BUILD_KREADCONFIG "Build kreadconfig" ${BUILD_ALL} )
option( BUILD_KDESU "Build kdesu" ${BUILD_ALL} )
option( BUILD_KDM "Build kdm" ${BUILD_ALL} )
option( BUILD_KSMSERVER "Build ksmserver" ${BUILD_ALL} )
option( BUILD_KSPLASHML "Build ksplashml" ${BUILD_ALL} )
option( BUILD_STARTKDE "Build startkde" ${BUILD_ALL} )
option( BUILD_KFIND "Build kfind" ${BUILD_ALL} )
option( BUILD_DRKONQI "Build drkonqi" ${BUILD_ALL} )
option( BUILD_KDCOP "Build kdcop" ${BUILD_ALL} )
option( BUILD_KDEBUGDIALOG "Build kdebugdialog" ${BUILD_ALL} )
option( BUILD_KNETATTACH "Build knetattach" ${BUILD_ALL} )
option( BUILD_KPAGER "Build kpager" ${BUILD_ALL} )
option( BUILD_KAPPFINDER "Build kappfinder" ${BUILD_ALL} )
option( BUILD_KCHECKPASS "Build kcheckpass" ${BUILD_ALL} )
option( BUILD_KHELPCENTER "Build khelpcenter" ${BUILD_ALL} )
option( BUILD_KHOTKEYS "Build khotkeys" ${BUILD_ALL} )
option( BUILD_KSCREENSAVER "Build kscreensaver" ${BUILD_ALL} )
option( BUILD_KSTART "Build kstart" ${BUILD_ALL} )
option( BUILD_KSYSTRAYCMD "Build ksystraycmd" ${BUILD_ALL} )
option( BUILD_NSPLUGINS "Build nsplugins"  ${BUILD_ALL} )
option( BUILD_KSYSGUARD "Build ksysguard"  ${BUILD_ALL} )
option( BUILD_KXKB "Build kxkb"  ${BUILD_ALL} )


##### configure checks ##########################

include( ConfigureChecks.cmake )


##### kdebase directories #######################

add_subdirectory( kdmlib )
add_subdirectory( kcontrol )

kde_conditional_add_subdirectory( BUILD_L10N l10n )
kde_conditional_add_subdirectory( BUILD_PICS pics )
kde_conditional_add_subdirectory( BUILD_APPLNK applnk )
kde_conditional_add_subdirectory( BUILD_DOC doc )
kde_conditional_add_subdirectory( BUILD_LIBKONQ libkonq )
kde_conditional_add_subdirectory( BUILD_KCMINIT kcminit )
kde_conditional_add_subdirectory( BUILD_KWIN kwin )
kde_conditional_add_subdirectory( BUILD_KATE kate )
kde_conditional_add_subdirectory( BUILD_KONQUEROR konqueror )
kde_conditional_add_subdirectory( BUILD_KIOSLAVES kioslave )
kde_conditional_add_subdirectory( BUILD_KICKER kicker )
kde_conditional_add_subdirectory( BUILD_KDESKTOP kdesktop )
kde_conditional_add_subdirectory( BUILD_KONSOLE konsole )
kde_conditional_add_subdirectory( BUILD_KLIPPER klipper )
kde_conditional_add_subdirectory( BUILD_KMENUEDIT kmenuedit )
kde_conditional_add_subdirectory( BUILD_KDEPRINT kdeprint )
kde_conditional_add_subdirectory( BUILD_KPERSONALIZER kpersonalizer )
kde_conditional_add_subdirectory( BUILD_KDEPASSWD kdepasswd )
kde_conditional_add_subdirectory( BUILD_KTIP ktip )
kde_conditional_add_subdirectory( BUILD_KDEEJECT kdeeject )
kde_conditional_add_subdirectory( BUILD_KDIALOG kdialog )
kde_conditional_add_subdirectory( BUILD_KREADCONFIG kreadconfig )
kde_conditional_add_subdirectory( BUILD_KDESU kdesu )
kde_conditional_add_subdirectory( BUILD_KDM kdm )
kde_conditional_add_subdirectory( BUILD_KSMSERVER ksmserver )
kde_conditional_add_subdirectory( BUILD_KSPLASHML ksplashml )
kde_conditional_add_subdirectory( BUILD_KFIND kfind )
kde_conditional_add_subdirectory( BUILD_DRKONQI drkonqi )
kde_conditional_add_subdirectory( BUILD_KDCOP kdcop )
kde_conditional_add_subdirectory( BUILD_KDEBUGDIALOG kdebugdialog )
kde_conditional_add_subdirectory( BUILD_KNETATTACH knetattach )
kde_conditional_add_subdirectory( BUILD_KPAGER kpager )
kde_conditional_add_subdirectory( BUILD_KAPPFINDER kappfinder )
kde_conditional_add_subdirectory( BUILD_KCHECKPASS kcheckpass )
kde_conditional_add_subdirectory( BUILD_KHELPCENTER khelpcenter )
kde_conditional_add_subdirectory( BUILD_KHOTKEYS khotkeys )
kde_conditional_add_subdirectory( BUILD_KSCREENSAVER kscreensaver )
kde_conditional_add_subdirectory( BUILD_KSTART kstart )
kde_conditional_add_subdirectory( BUILD_KSYSTRAYCMD ksystraycmd )
kde_conditional_add_subdirectory( BUILD_NSPLUGINS nsplugins )
kde_conditional_add_subdirectory( BUILD_KSYSGUARD ksysguard )
kde_conditional_add_subdirectory( BUILD_KXKB kxkb )


##### install startkde & related stuff ##########

if( BUILD_STARTKDE )
  install( PROGRAMS startkde DESTINATION ${BIN_INSTALL_DIR} )
endif()


##### write configure files #####################

configure_file( config.h.cmake config.h @ONLY )
