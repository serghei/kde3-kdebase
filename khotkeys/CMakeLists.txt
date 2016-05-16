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

check_c_source_runs( "
  class A { public: virtual A* me(); };
  class X { public: int x; virtual void ff() {}; };
  class B : public X, public A { public: virtual B* me(); };
  int foo( A* a )
    {
      A* aa = a->me();
      return a == aa;
    }
  int main()
    {
      B* b = new B;
      return foo( b ) == 0;
    }
  A* A::me() { return this; }
  B* B::me() { return this; }
" COVARIANT_RETURN )

if( NOT COVARIANT_RETURN )
  set( COVARIANT_RETURN_BROKEN 1 CACHE INTERNAL "" FORCE )
endif( )

add_subdirectory( shared )
add_subdirectory( kcontrol )
add_subdirectory( app )
add_subdirectory( data )
add_subdirectory( update )

install( FILES khotkeys.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
