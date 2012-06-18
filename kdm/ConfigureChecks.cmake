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

include( CheckStructHasMember )
include( CheckCSourceRuns )

find_library( UTIL_LIBRARY util )

check_function_exists( getdomainname HAVE_GETDOMAINNAME )
check_function_exists( initgroups HAVE_INITGROUPS )
check_function_exists( mkstemp HAVE_MKSTEMP )
check_function_exists( setproctitle HAVE_SETPROCTITLE )
check_function_exists( sysinfo HAVE_SYSINFO )
check_function_exists( strnlen HAVE_STRNLEN )
check_function_exists( getifaddrs HAVE_GETIFADDRS )

kde_save( CMAKE_REQUIRED_LIBRARIES )
set( CMAKE_REQUIRED_LIBRARIES ${UTIL_LIBRARY} )
check_function_exists( setusercontext HAVE_SETUSERCONTEXT )
check_function_exists( getusershell HAVE_GETUSERSHELL )
check_function_exists( login_getclass HAVE_LOGIN_GETCLASS )
check_function_exists( auth_timeok HAVE_AUTH_TIMEOK )
kde_restore( CMAKE_REQUIRED_LIBRARIES )

check_function_exists( crypt LIBC_HAVE_CRYPT )
if( LIBC_HAVE_CRYPT )
  set( HAVE_CRYPT 1 CACHE INTERNAL "" FORCE )
else( )
  check_library_exists( crypt crypt "" HAVE_CRYPT )
endif( )
if( HAVE_CRYPT )
  set( CRYPT_LIBRARY crypt )
endif( )
set( CRYPT_LIBRARY crypt )

check_include_file( lastlog.h HAVE_LASTLOG_H )
check_include_file( termio.h HAVE_TERMIO_H )

check_struct_has_member( "struct sockaddr_in" "sin_len" "sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN_SIN_LEN )
check_struct_has_member( "struct passwd" "pw_expire" "pwd.h" HAVE_STRUCT_PASSWD_PW_EXPIRE )
check_struct_has_member( "struct utmp" "ut_user" "utmp.h" HAVE_STRUCT_UTMP_UT_USER )

check_c_source_runs( "
  #include <errno.h>
  #include <unistd.h>
  int main()
  {
      setlogin(0);
      return errno == ENOSYS;
  }
" HAVE_SETLOGIN )

check_c_source_runs( "
  #include <sys/socket.h>
  #include <sys/un.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <string.h>
  #include <unistd.h>
  #include <errno.h>
  int main()
  {
    int fd, fd2;
    struct sockaddr_un sa;

    if((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
      return 2;
    sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path, \"testsock\");
    unlink(sa.sun_path);
    if(bind(fd, (struct sockaddr *)&sa, sizeof(sa)))
      return 2;
    chmod(sa.sun_path, 0);
    setuid(getuid() + 1000);
    if((fd2 = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
      return 2;
    connect(fd2, (struct sockaddr *)&sa, sizeof(sa));
    return errno != EACCES;
  }
" HONORS_SOCKET_PERMS )

if( CMAKE_SYSTEM_NAME MATCHES Linux OR CMAKE_SYSTEM_NAME MATCHES Darwin OR CMAKE_SYSTEM_NAME MATCHES GNU/FreeBSD )
    unset( HAVE_UTMPX )
    unset( HAVE_LASTLOGX )
else( )
    check_function_exists( getutxent HAVE_UTMPX )
    check_function_exists( updlastlogx HAVE_LASTLOGX )
endif( )

unset( BSD_UTMP )
if( NOT HAVE_UTMPX )
    check_function_exists( getutent have_getutent )
    if( NOT have_getutent )
        set( BSD_UTMP 1 )
    endif( )
endif( )

check_function_exists( arc4random HAVE_ARC4RANDOM )
if( NOT HAVE_ARC4RANDOM )
    # assume that /dev/random is non-blocking if /dev/urandom does not exist
    if( EXISTS /dev/urandom )
      set( DEV_RANDOM "/dev/urandom" CACHE INTERNAL "" FORCE )
    elseif( EXISTS /dev/random )
      set( DEV_RANDOM "/dev/random" CACHE INTERNAL "" FORCE )
    endif( )
endif (NOT HAVE_ARC4RANDOM)

# Xau
pkg_search_module( XAU xau )
if( NOT XAU_FOUND )
  kde_message_fatal( "Xau is required, but not found on your system" )
endif()


# xdmcp
if( WITH_XDMCP )
  pkg_search_module( XDMCP xdmcp )
  if( XDMCP_FOUND )
    set( XDMCP 1 CACHE INTERNAL "" FORCE )
  else()
    kde_message_fatal( "xdmcp is required, but was not found on your system" )
  endif()
endif()


# dbus-1
kde_conditional_search_module( WITH_CONSOLE_KIT DBUS dbus-1 )


if( WITH_PAM )

  set( USE_PAM 1 CACHE INTERNAL "" FORCE )

elseif( WITH_SHADOW )

  set( HAVE_SHADOW 1 CACHE INTERNAL "" FORCE )
  set( USESHADOW 1 CACHE INTERNAL "" FORCE )

endif( )
