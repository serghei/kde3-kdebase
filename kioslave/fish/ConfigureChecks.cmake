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

include( CheckCSourceRuns )

check_include_file( stropts.h HAVE_STROPTS )
check_include_file( libutil.h HAVE_LIBUTIL_H )
check_include_file( util.h HAVE_UTIL_H )
check_include_file( pty.h HAVE_PTY_H )


kde_save( CMAKE_REQUIRED_LIBRARIES )
set( CMAKE_REQUIRED_LIBRARIES util )

check_c_source_runs("
    #include <pty.h>
    int main(int argc, char* argv) {
      int master_fd, slave_fd;
      int result;
      result = openpty(&master_fd, &slave_fd, 0, 0, 0);
      return 0;
  }"
  HAVE_OPENPTY
)

kde_restore( CMAKE_REQUIRED_LIBRARIES )
