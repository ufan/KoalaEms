/*
 * commu_anno.hxx
 * 
 * $ZEL: commu_anno.hxx,v 2.2 2004/11/18 19:31:02 wuestner Exp $
 * 
 * 20.08.93 PW
 */

#ifndef _commu_anno_hxx_
#define _commu_anno_hxx_

/*****************************************************************************/

class C_announce
  {
  void do_it(int);
  public:
    C_announce(){do_it(1);}
    ~C_announce(){do_it(0);}
  };

#endif
/*****************************************************************************/
/*****************************************************************************/
