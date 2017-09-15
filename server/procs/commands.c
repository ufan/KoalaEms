/*
 * commands.c
 * created before ???1993
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: commands.c,v 1.12 2011/04/06 20:30:29 wuestner Exp $";

#include <sconf.h>
#include <debug.h>
#include <stdio.h>
#include <errorcodes.h>
#include <rcs_ids.h>
#include "emsctypes.h"
#include "proclist.h"
#include "commands.h"

#ifdef OBJ_IS
#include "../objects/is/is.h"
#endif /* OBJ_IS */

extern int *memberlist;
extern struct IS *current_IS;
#ifdef ISVARS 
extern ISV *isvar;
#endif

RCS_REGISTER(cvsid, "procs")

/*****************************************************************************/
errcode DoCommand(ems_u32* p, unsigned int len)
{
    unsigned int idx= *p++;
    errcode res;
    int limit;

    T(DoCommand)
    D(D_REQ, printf("DoCommand IS %d:   1. Proc.ID = %d\n", idx, *(p+1));)
    if (idx) {
#ifdef OBJ_IS
        if (idx>=MAX_IS) return(Err_IllIS);
        if (!is_list[idx]) return(Err_NoIS);
        current_IS=is_list[idx];
        memberlist=current_IS->members;
        if (memberlist==(int*)0) return(Err_NoISModulList);
#ifdef ISVARS
        isvar=&(current_IS->isvar);
#endif /* ISVARS */
#else  /* OBJ_IS */
        return(Err_NoIS);
#endif /* OBJ_IS */
    } else {
        current_IS=0;
        memberlist=(int*)0;
#ifdef ISVARS
        isvar=(ISV*)0;
#endif /* ISVARS */
    }
    if ((res=test_proclist(p, len, &limit))!=OK) return(res);
    if(limit>OUT_MAX) {
        printf("DoCommand: test_proclist: limit=%d > OUT_MAX=%d\n",
                limit, OUT_MAX);
        return(Err_BufOverfl);
    }
    return(scan_proclist(p));
}
/*****************************************************************************/

errcode TestCommand(ems_u32* p, unsigned int len)
{
unsigned int idx= *p++;
errcode res;
int limit;

T(TestCommand)
D(D_REQ, printf("TestKommando fuer IS %d\n", idx);)
if (idx)
  {
#ifdef OBJ_IS
  if (idx>=MAX_IS) return(Err_IllIS);
  if (!is_list[idx]) return(Err_NoIS);
  current_IS=is_list[idx];
  memberlist=current_IS->members;
  if (memberlist==(int*)0) return(Err_NoISModulList);
#ifdef ISVARS
  isvar=&(current_IS->isvar);
#endif /* ISVARS */
#else  /* OBJ_IS */
  return(Err_NoIS);
#endif /* OBJ_IS */
  }
else
  {
  current_IS=0;
  memberlist=(int*)0;
#ifdef ISVARS
  isvar=(ISV*)0;
#endif /* ISVARS */
  }
res=test_proclist(p,len,&limit);
if(res)return(res);
if(limit>OUT_MAX) {
    printf("TestCommand: test_proclist: limit=%d > OUT_MAX=%d\n",
            limit, OUT_MAX);
    return(Err_BufOverfl);
}
return(OK);
}

/*****************************************************************************/
/*****************************************************************************/

