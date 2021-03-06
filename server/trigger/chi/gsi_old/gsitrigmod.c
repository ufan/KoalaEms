/******************************************************************************
*                                                                             *
*  gsitrigmod.c for chi                                                       *
*                                                                             *
*  OS9                                                                        *
*                                                                             *
*  26.10.94                                                                   *
*                                                                             *
******************************************************************************/

#include <sconf.h>
#include <debug.h>
#include <errorcodes.h>
#include <ssm.h>

#include "gsitrigger.h"

#ifdef NOVOLATILE
#define VOLATILE
#else
#define VOLATILE volatile
#endif

static VOLATILE int *stat, *ctrl, *fca, *cvt;
static int nummer;

static int hund;

/*****************************************************************************/

plerrcode trigmod_init(int base, int master, int ctime, int fcatime)
{
int status;
char* basis;

T(trigmod_init)

if ((base!=1) && (base!=2) && (base!=3)) return(plErr_ArgRange);
#ifdef NANO
ctime/=100;
fcatime/=100;
#endif

/* conv. time > fast clear window ? */
if (!ctime || !fcatime || ((ctime | fcatime)&0xffff0000) || (ctime<=fcatime))
    return(plErr_ArgRange);

basis=GSI_BASE+0x100*base;
stat=(int*)(basis+STATUS);
ctrl=(int*)(basis+CONTROL);
fca=(int*)(basis+FCATIME);
cvt=(int*)(basis+CTIME);

permit(basis, 16, ssm_RW);

*ctrl=CLEAR;
*ctrl=master?MASTER:SLAVE;
*ctrl=BUS_ENABLE;
*cvt=0x10000-ctime;
*fca=0x10000-fcatime;

*ctrl=CLEAR;
*ctrl=GO;

return(plOK);
}

/*****************************************************************************/

plerrcode trigmod_done()
{
T(trigmod_done)
*ctrl=HALT;
*ctrl=BUS_DISABLE;
return(plOK);
}

/*****************************************************************************/

int trigmod_get()
{
register int status, trigger;

T(trigmod_get)
status=*stat;
if ((status&EON)==0) return(0);

status=*stat;
if ((status & EON)==0)
  {
  printf("Trigger: EON==0\n");
  return(0);
  }

trigger=status & 0xf;
if (trigger==0)
  {
  printf("Trigger: trigger==0\n");
  return(0);
  }

if (status & (REJECT | MISMATCH))
  {
  if (status & REJECT)
    {
    printf("Trigger: REJECT\n");
    }
  if (status & MISMATCH)
    {
    printf("Trigger: MISMATCH\n");
    }
  return(0);
  }
return(trigger);
}

/*****************************************************************************/

void trigmod_reset()
{
T(trigmod_reset)
*stat=EV_IRQ_CLEAR;
*stat=DT_CLEAR;
}

/*****************************************************************************/

plerrcode trigmod_soft_init(int base, int id, int ctime, int fcatime)
{
int status;
char* basis;

T(trigmod_soft_init)

if ((base!=1) && (base!=2) && (base!=3)) return(plErr_ArgRange);

if ((id<1) || (id>15)) return(plErr_ArgRange);
nummer=id;
#ifdef NANO
ctime/=100;
fcatime/=100;
#endif

/* conv. time > fast clear window ? */
if (!ctime || !fcatime || ((ctime | fcatime)&0xffff0000) || (ctime<=fcatime))
    return(plErr_ArgRange);

basis=GSI_BASE+0x100*base;
stat=(int*)(basis+STATUS);
ctrl=(int*)(basis+CONTROL);
fca=(int*)(basis+FCATIME);
cvt=(int*)(basis+CTIME);

permit(basis, 16, ssm_RW);

*ctrl=CLEAR;
*ctrl=MASTER;
*ctrl=BUS_ENABLE;
*cvt=0x10000-ctime;
*fca=0x10000-fcatime;

/**ctrl=CLEAR;*/

*ctrl=GO;

return(plOK);
}

/*****************************************************************************/

plerrcode trigmod_soft_done()
{
T(trigmod_soft_done)
*ctrl=HALT;
*ctrl=BUS_DISABLE;
return(plOK);
}

/*****************************************************************************/

int trigmod_soft_get()
{
register int status;
int trigger;

T(trigmod_soft_get)

*stat=nummer;
tsleep(2);

status=*stat;
if ((status&EON)==0) return(0);

status=*stat;
if ((status&EON)==0)
  {
  printf("Trigger: EON==0\n");
  return(0);
  }

trigger=status&0xf;
if (trigger==0)
  {
  printf("Trigger: trigger==0\n");
  return(0);
  }

if (status & (REJECT | MISMATCH))
  {
  if (status & REJECT)
    {
    printf("Trigger: REJECT\n");
    }
  if (status & MISMATCH)
    {
    printf("Trigger: MISMATCH\n");
    }
  return(0);
  }

return(status&0xf);
}

/*****************************************************************************/

void trigmod_soft_reset()
{
T(trigmod_soft_reset)
*stat=EV_IRQ_CLEAR;
*stat=DT_CLEAR;
}

/*****************************************************************************/
/*****************************************************************************/
