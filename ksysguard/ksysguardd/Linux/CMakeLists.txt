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

add_definitions( -D_GNU_SOURCE )

if( WITH_I8K )
  add_definitions( -DHAVE_I8K_SUPPORT )
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/ksysguard/CContLib
  ${CMAKE_SOURCE_DIR}/ksysguard/ksysguardd
)


##### ksysguardd (static) #######################

kde_add_library( ksysguardd STATIC
  SOURCES
    ProcessList.c Memory.c stat.c netdev.c apm.c acpi.c
    loadavg.c cpuinfo.c lmsensors.c netstat.c diskstat.c
    logfile.c i8k.c
)
