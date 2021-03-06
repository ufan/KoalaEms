/************************************************************************/
/*                                                                      */
/* manuelle Initialisierung                                             */
/*                                                                      */
/* Die Prozedure ermoeglicht eine manuelle Initialisierung des VME-     */
/* Rahmens.                                                             */
/*                                                                      */
/* Parameter: CFG A24_MEM A24_IO ...                                    */
/*                                                                      */
/* 27.04.'93                                                            */
/*                                                                      */
/* Autor Twardowski                                                     */
/*                                                                      */
/************************************************************************/

#include <types.h>
#include <errorcodes.h>
#include "../../LOWLEVEL/VME/vme.h"

plerrcode proc_InitManuell(p)
int *p;
{
int i;
u_char *dummy;

i=1;

while ( i<=p[0] )
{
   dummy=VMEinit(p[i],p[i+1],p[i+2]);     /*SMP_i*/
   i=i+3;
}
return(plOK);
}

plerrcode test_proc_InitManuell(p)
int *p;
{
if (*p==0) return(plErr_ArgNum);
if (*p%3!=0) return(plErr_ArgNum);
return(plOK);
}

char name_proc_InitManuell[]="InitManuell";
int ver_proc_InitManuell=1;
/***********************************************************************/
