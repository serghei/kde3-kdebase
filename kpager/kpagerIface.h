#ifndef _KPAGERIFACE_H_
#define _KPAGERIFACE_H_

#include <dcopobject.h>

class KPagerIface : virtual public DCOPObject
{
   K_DCOP

k_dcop:
/**
 * If it's closed, open the window at the specified location
 */ 
   virtual void showAt(int x, int y)=0;

/**
 * Toggles the show/hide state of kpager
 */
   virtual void toggleShow(int x, int y)=0;
};
#endif

