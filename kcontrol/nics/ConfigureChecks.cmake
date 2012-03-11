#################################################
#
#  (C) 2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include( CheckFunctionExists )
include( CheckStructHasMember )

check_include_file( "sys/sockio.h" HAVE_SYS_SOCKIO_H )
check_function_exists( getnameinfo HAVE_GETNAMEINFO )
check_struct_has_member( "struct sockaddr" sa_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_SA_LEN )
