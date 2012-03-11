#################################################
#
#  (C) 2010-2012 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#################################################

install( FILES
    northafrica.desktop westafrica.desktop southafrica.desktop
    centralafrica.desktop caribbean.desktop oceania.desktop
    centralamerica.desktop northamerica.desktop southamerica.desktop
    easteurope.desktop westeurope.desktop centraleurope.desktop
    northeurope.desktop southeurope.desktop southasia.desktop
    southeastasia.desktop eastasia.desktop centralasia.desktop
    middleeast.desktop
  DESTINATION
    ${LOCALE_INSTALL_DIR}/l10n )


file( GLOB _dirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} * )
foreach( _dir ${_dirs} )
  if( IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_dir} )
    if( NOT (${_dir} STREQUAL ".svn" OR ${_dir} STREQUAL "CMakeFiles") )
      install( FILES ${_dir}/entry.desktop ${_dir}/flag.png
        DESTINATION ${LOCALE_INSTALL_DIR}/l10n/${_dir} )
    endif( )
  endif( )
endforeach( )
