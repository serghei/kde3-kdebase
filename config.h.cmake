#define VERSION "@VERSION@"

// konsole
#cmakedefine HAVE_PROC_CWD 1

// kpowermanager, kcontrol/randr
#cmakedefine HAVE_XRANDR 1

// kdesktop, konsole, kcontrol, kicker
#cmakedefine HAVE_XRENDER 1

// taskmanager, klipper
#cmakedefine HAVE_XFIXES 1

// kdesktop, kcontrol, ksplashml
#cmakedefine HAVE_XCURSOR 1

// konsole, kdm, kxkb
#cmakedefine HAVE_XKB 1

// kdm, kioslave
#cmakedefine HAVE_TERMIOS_H 1

// kioslave/media
#cmakedefine WITH_HAL 1
#ifdef WITH_HAL
#define COMPILE_HALBACKEND
#define COMPILE_LINUXCDPOLLING
#endif

// kioslave/fish, kcontrol/info
#cmakedefine HAVE_SYS_IOCTL_H 1

// kioslave/smtp, kioslave/pop3
#cmakedefine HAVE_LIBSASL2 1

// kdm, kcontrol
#cmakedefine HAVE_GETIFADDRS 1

// kio_fish
#cmakedefine HAVE_STROPTS 1
#cmakedefine HAVE_LIBUTIL_H 1
#cmakedefine HAVE_UTIL_H 1
#cmakedefine HAVE_PTY_H 1
#cmakedefine HAVE_OPENPTY 1

// kio_man
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_STRING_H 1

// kio_smtp, ksysguard
#cmakedefine kde_socklen_t @kde_socklen_t@

// kfile_media
#cmakedefine HAVE_STATVFS

// taskmanager
#cmakedefine HAVE_XCOMPOSITE

// kcontrol/fonts
#cmakedefine HAVE_FONTCONFIG 1
#cmakedefine HAVE_FREETYPE2 1

// kcontrol/kfontinst
#cmakedefine HAVE_XFT 1
#cmakedefine HAVE_GETOPT_H 1

// kcontrol/energy
#cmakedefine HAVE_DPMS 1

// kdesktop, kcontrol/screensaver, kscreensaver
#cmakedefine HAVE_GLXCHOOSEVISUAL 1

// kcontrol/crypto
#cmakedefine HAVE_SSL 1

// kcontrol/nics
#cmakedefine HAVE_SYS_SOCKIO_H 1
#cmakedefine HAVE_GETNAMEINFO 1
#cmakedefine HAVE_STRUCT_SOCKADDR_SA_LEN 1

// kcontrol/input
#cmakedefine HAVE_LIBUSB 1

// kdeprint
#cmakedefine HAVE_SIGACTION 1
#cmakedefine HAVE_SIGSET 1

// kdesu
#cmakedefine HAVE_STRUCT_UCRED 1
#cmakedefine HAVE_GETPEEREID 1
#cmakedefine HAVE_SYS_SELECT_H 1
#cmakedefine HAVE_SYS_WAIT_H 1
#cmakedefine DEFAULT_SUPER_USER_COMMAND "@DEFAULT_SUPER_USER_COMMAND@"

// kdm, kcheckpass, kdesktop
#cmakedefine HAVE_PAM 1

// kxkb, kdm, khotkeys
#cmakedefine HAVE_XTEST 1

// kcheckpass
#cmakedefine KCHECKPASS_PAM_SERVICE "@KCHECKPASS_PAM_SERVICE@"

// kdesktop
#cmakedefine KSCREENSAVER_PAM_SERVICE "@KSCREENSAVER_PAM_SERVICE@"

// kdm
#cmakedefine XBINDIR "@XBINDIR@"
#define KDE_BINDIR "@BIN_INSTALL_DIR@"
#define KDE_DATADIR "@DATA_INSTALL_DIR@"
#define KDE_CONFDIR "@CONFIG_INSTALL_DIR@"

#cmakedefine HAVE_XKBSETPERCLIENTCONTROLS 1

#cmakedefine HAVE_GETDOMAINNAME 1
#cmakedefine HAVE_INITGROUPS 1
#cmakedefine HAVE_MKSTEMP 1
#cmakedefine HAVE_SETPROCTITLE 1
#cmakedefine HAVE_SYSINFO 1
#cmakedefine HAVE_STRNLEN 1
#cmakedefine HAVE_GETIFADDRS 1
#cmakedefine HAVE_CRYPT 1

#cmakedefine HAVE_SETUSERCONTEXT 1
#cmakedefine HAVE_GETUSERSHELL 1
#cmakedefine HAVE_LOGIN_GETCLASS 1
#cmakedefine HAVE_AUTH_TIMEOK 1

#cmakedefine HAVE_LASTLOG_H 1
#cmakedefine HAVE_TERMIO_H 1

#cmakedefine HAVE_STRUCT_SOCKADDR_IN_SIN_LEN 1
#cmakedefine HAVE_STRUCT_PASSWD_PW_EXPIRE 1
#cmakedefine HAVE_STRUCT_UTMP_UT_USER 1

#cmakedefine HAVE_SETLOGIN 1
#cmakedefine HONORS_SOCKET_PERMS 1

#cmakedefine HAVE_UTMPX 1
#cmakedefine HAVE_LASTLOGX 1
#cmakedefine BSD_UTMP 1

#cmakedefine HAVE_ARC4RANDOM 1
#cmakedefine DEV_RANDOM "@DEV_RANDOM@"

#cmakedefine USE_PAM 1
#cmakedefine KDM_PAM_SERVICE "@KDM_PAM_SERVICE@"

#define USESHADOW 1
#define HAVE_SHADOW 1

#cmakedefine XDMCP 1


// ksmserver
#cmakedefine DBUS_SYSTEM_BUS "@WITH_DBUS_SYSTEM_BUS@"
#cmakedefine WITH_LOGIND 1

// ksplashml
#cmakedefine HAVE_XINERAMA 1

// khotkeys
#cmakedefine HAVE_ARTS 1
#cmakedefine COVARIANT_RETURN_BROKEN 1

// kdm, kxkb
#cmakedefine XLIBDIR "@XLIBDIR@"
