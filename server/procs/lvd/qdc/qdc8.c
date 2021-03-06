/*
 * procs/lvd/qdc/qdc8.c
 * created 2012-Aug-30(?) PK
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: qdc8.c,v 1.4 2017/10/20 23:11:44 wuestner Exp $";

#include <sconf.h>
#include <debug.h>
#include <errorcodes.h>
#include <modultypes.h>
#include <rcs_ids.h>
#include "../../procs.h"
#include "../../../objects/domain/dom_ml.h"
#include "../../../lowlevel/lvd/lvdbus.h"
#include "../../../lowlevel/lvd/qdc/qdc8.h"
#include "../../../lowlevel/devices.h"
#include "../lvd_verify.h"

RCS_REGISTER(cvsid, "procs/lvd/qdc")

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Set register outp */

plerrcode proc_lvd_qdc_c_outp(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_outp(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_outp(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_outp(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_outp[] = "lvd_qdc_c_outp";
int ver_proc_lvd_qdc_c_outp = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add search window to output data (time,baseline) */

plerrcode proc_lvd_qdc_c_festart(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_festart(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_festart(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_festart(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_festart[] = "lvd_qdc_c_festart";
int ver_proc_lvd_qdc_c_festart = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add minimum to output data (time,amplitude) */

plerrcode proc_lvd_qdc_c_femin(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_femin(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_femin(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_femin(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_femin[] = "lvd_qdc_c_femin";
int ver_proc_lvd_qdc_c_femin = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add maximum to output data (time,amplitude) */

plerrcode proc_lvd_qdc_c_femax(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_femax(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_femax(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_femax(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_femax[] = "lvd_qdc_c_femax";
int ver_proc_lvd_qdc_c_femax = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add pulse end  to output data (time,amplitude) */

plerrcode proc_lvd_qdc_c_feend(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feend(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feend(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feend(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feend[] = "lvd_qdc_c_feend";
int ver_proc_lvd_qdc_c_feend = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add tot to output data (timeup)(timedown) */

plerrcode proc_lvd_qdc_c_fetot(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_fetot(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_fetot(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_fetot(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_fetot[] = "lvd_qdc_c_fetot";
int ver_proc_lvd_qdc_c_fetot = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add cfd to output data (time) */

plerrcode proc_lvd_qdc_c_fecfd(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_fecfd(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_fecfd(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_fecfd(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_fecfd[] = "lvd_qdc_c_fecfd";
int ver_proc_lvd_qdc_c_fecfd = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add zero crossing time to output data (time dx(1,2,4)) */

plerrcode proc_lvd_qdc_c_fezero(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_fezero(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_fezero(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_fezero(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_fezero[] = "lvd_qdc_c_fezero";
int ver_proc_lvd_qdc_c_fezero = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add amplitude at the end of pulse integral to output data */

plerrcode proc_lvd_qdc_c_feqae(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feqae(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feqae(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feqae(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feqae[] = "lvd_qdc_c_feqae";
int ver_proc_lvd_qdc_c_feqae = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add pulse integral to output data (Integral 20 bit) */

plerrcode proc_lvd_qdc_c_feqpls(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feqpls(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feqpls(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feqpls(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feqpls[] = "lvd_qdc_c_feqpls";
int ver_proc_lvd_qdc_c_feqpls = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add main Cluster integral,amplitude at begin to output data (Integral 20 bit) */

plerrcode proc_lvd_qdc_c_feqclab(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feqclab(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feqclab(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feqclab(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feqclab[] = "lvd_qdc_c_feqclab";
int ver_proc_lvd_qdc_c_feqclab = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add main Cluster integral,amplitude at end to output data (Integral 20 bit) */

plerrcode proc_lvd_qdc_c_feqclae(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feqclae(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feqclae(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feqclae(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feqclae[] = "lvd_qdc_c_feqclae";
int ver_proc_lvd_qdc_c_feqclae = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add IW Amplitudes at start and end to output data (Integral 20 bit)  */

plerrcode proc_lvd_qdc_c_feiwedge(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feiwedge(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feiwedge(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feiwedge(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feiwedge[] = "lvd_qdc_c_feiwedge";
int ver_proc_lvd_qdc_c_feiwedge = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add IW Amplitudes at start and end to output data (Integral 20 bit)  */

plerrcode proc_lvd_qdc_c_feiwmax(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feiwmax(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feiwmax(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feiwmax(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feiwmax[] = "lvd_qdc_c_feiwmax";
int ver_proc_lvd_qdc_c_feiwmax = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add IW time of centre  to output data */

plerrcode proc_lvd_qdc_c_feiwcenter(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feiwcenter(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feiwcenter(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feiwcenter(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feiwcenter[] = "lvd_qdc_c_feiwcenter";
int ver_proc_lvd_qdc_c_feiwcenter = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add overruns since last readout to output data */

plerrcode proc_lvd_qdc_c_feovrun(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_feovrun(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_feovrun(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_feovrun(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_feovrun[] = "lvd_qdc_c_feovrun";
int ver_proc_lvd_qdc_c_feovrun = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Add maximum noise peak since last redout to output data */

plerrcode proc_lvd_qdc_c_fenoise(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_fenoise(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_fenoise(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_fenoise(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_fenoise[] = "lvd_qdc_c_fenoise";
int ver_proc_lvd_qdc_c_fenoise = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Set register chtrl */

plerrcode proc_lvd_qdc_c_chctrl(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_chctrl(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_chctrl(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_chctrl(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_chctrl[] = "lvd_qdc_c_chctrl";
int ver_proc_lvd_qdc_c_chctrl = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Inhibit channel */

plerrcode proc_lvd_qdc_c_chaena(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_chaena(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_chaena(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_chaena(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_chaena[] = "lvd_qdc_c_chaena";
int ver_proc_lvd_qdc_c_chaena = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Use level in pulse search algorithm */

plerrcode proc_lvd_qdc_c_plev(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_plev(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_plev(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_plev(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_plev[] = "lvd_qdc_c_plev";
int ver_proc_lvd_qdc_c_plev = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Select positive pulse */

plerrcode proc_lvd_qdc_c_polarity(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_polarity(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_polarity(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_polarity(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_polarity[] = "lvd_qdc_c_polarity";
int ver_proc_lvd_qdc_c_polarity = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Select gradient for zero crossing to 2 */

plerrcode proc_lvd_qdc_c_gradient(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_gradient(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_gradient(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_gradient(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_gradient[] = "lvd_qdc_c_gradient";
int ver_proc_lvd_qdc_c_gradient = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Do not extend search window */

plerrcode proc_lvd_qdc_c_noext(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_noext(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_noext(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_noext(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_noext[] = "lvd_qdc_c_noext";
int ver_proc_lvd_qdc_c_noext = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Use tot_thr for raw selection */

plerrcode proc_lvd_qdc_c_tot4raw(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_tot4raw(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_tot4raw(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_tot4raw(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_tot4raw[] = "lvd_qdc_c_tot4raw";
int ver_proc_lvd_qdc_c_tot4raw = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Set register cf_qrise */

plerrcode proc_lvd_qdc_c_cfqrise(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_cfqrise(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_cfqrise(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_cfqrise(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_cfqrise[] = "lvd_qdc_c_cfqrise";
int ver_proc_lvd_qdc_c_cfqrise = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Begin charge value for pulse search algorithm */

plerrcode proc_lvd_qdc_c_qrise(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_qrise(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_qrise(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_qrise(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_qrise[] = "lvd_qdc_c_qrise";
int ver_proc_lvd_qdc_c_qrise = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* fraction for CF timing */

plerrcode proc_lvd_qdc_c_cf(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_cf(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_cf(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_cf(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_cf[] = "lvd_qdc_c_cf";
int ver_proc_lvd_qdc_c_cf = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Set register tot_level */

plerrcode proc_lvd_qdc_c_totlevel(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_totlevel(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_totlevel(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_totlevel(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_totlevel[] = "lvd_qdc_c_totlevel";
int ver_proc_lvd_qdc_c_totlevel = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* level for ToT and RAW threshold  */

plerrcode proc_lvd_qdc_c_totthr(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_totthr(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_totthr(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_totthr(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_totthr[] = "lvd_qdc_c_totthr";
int ver_proc_lvd_qdc_c_totthr = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* minimal width for ToT and RAW level threshold  */

plerrcode proc_lvd_qdc_c_totwidth(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_totwidth(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_totwidth(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_totwidth(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_totwidth[] = "lvd_qdc_c_totwidth";
int ver_proc_lvd_qdc_c_totwidth = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Set register logic_level */

plerrcode proc_lvd_qdc_c_logiclevel(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_logiclevel(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_logiclevel(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_logiclevel(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_logiclevel[] = "lvd_qdc_c_logiclevel";
int ver_proc_lvd_qdc_c_logiclevel = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* level for scaler and logic threshold  */

plerrcode proc_lvd_qdc_c_logicthr(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_logicthr(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_logicthr(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_logicthr(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_logicthr[] = "lvd_qdc_c_logicthr";
int ver_proc_lvd_qdc_c_logicthr = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* width for scaler and logic level threshold  */

plerrcode proc_lvd_qdc_c_logicwidth(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_logicwidth(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_logicwidth(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_logicwidth(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_logicwidth[] = "lvd_qdc_c_logicwidth";
int ver_proc_lvd_qdc_c_logicwidth = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Q threshold for integral in integral window */

plerrcode proc_lvd_qdc_c_iwqthr(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_iwqthr(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_iwqthr(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_iwqthr(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_iwqthr[] = "lvd_qdc_c_iwqthr";
int ver_proc_lvd_qdc_c_iwqthr = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Q threshold for cluster  integral  */

plerrcode proc_lvd_qdc_c_swqthr(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_swqthr(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_swqthr(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_swqthr(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_swqthr[] = "lvd_qdc_c_swqthr";
int ver_proc_lvd_qdc_c_swqthr = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Search window pulse integral length */

plerrcode proc_lvd_qdc_c_swilen(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_swilen(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_swilen(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_swilen(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_swilen[] = "lvd_qdc_c_swilen";
int ver_proc_lvd_qdc_c_swilen = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Cluster start integral length */

plerrcode proc_lvd_qdc_c_clqstlen(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_clqstlen(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_clqstlen(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_clqstlen(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_clqstlen[] = "lvd_qdc_c_clqstlen";
int ver_proc_lvd_qdc_c_clqstlen = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Cluster integral length */

plerrcode proc_lvd_qdc_c_cllen(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_cllen(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_cllen(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_cllen(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_cllen[] = "lvd_qdc_c_cllen";
int ver_proc_lvd_qdc_c_cllen = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Delay for Cluster integral */

plerrcode proc_lvd_qdc_c_clqdel(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_clqdel(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_clqdel(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_clqdel(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_clqdel[] = "lvd_qdc_c_clqdel";
int ver_proc_lvd_qdc_c_clqdel = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Set register coinc_par */

plerrcode proc_lvd_qdc_c_coincpar(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_coincpar(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_coincpar(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_coincpar(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_coincpar[] = "lvd_qdc_c_coincpar";
int ver_proc_lvd_qdc_c_coincpar = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Delay for coincidence */

plerrcode proc_lvd_qdc_c_coincdel(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_coincdel(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_coincdel(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_coincdel(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_coincdel[] = "lvd_qdc_c_coincdel";
int ver_proc_lvd_qdc_c_coincdel = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Length for coincidence */

plerrcode proc_lvd_qdc_c_coincwidth(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_coincwidth(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_coincwidth(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_coincwidth(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_coincwidth[] = "lvd_qdc_c_coincwidth";
int ver_proc_lvd_qdc_c_coincwidth = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* Tau value for base line algorithm */

plerrcode proc_lvd_qdc_c_dttau(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_dttau(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_dttau(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_dttau(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_dttau[] = "lvd_qdc_c_dttau";
int ver_proc_lvd_qdc_c_dttau = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: channel idx (c0..15 or -1 for all channels)
 *  p[4]: argument(s)]
 */

/* lookup table for RAW request */

plerrcode proc_lvd_qdc_c_rawtable(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setc_rawtable(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getc_rawtable(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_c_rawtable(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16*16:0;
  return plOK;
}

char name_proc_lvd_qdc_c_rawtable[] = "lvd_qdc_c_rawtable";
int ver_proc_lvd_qdc_c_rawtable = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: FPGA idx (0..3 or -1 for all four groups)
 *  p[4]: argument(s)]
 */

/* Search window latency */

plerrcode proc_lvd_qdc_g_swstart(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setg_swstart(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getg_swstart(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?4:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_g_swstart(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?64:0;
  return plOK;
}

char name_proc_lvd_qdc_g_swstart[] = "lvd_qdc_g_swstart";
int ver_proc_lvd_qdc_g_swstart = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: FPGA idx (0..3 or -1 for all four groups)
 *  p[4]: argument(s)]
 */

/* Search window length */

plerrcode proc_lvd_qdc_g_swlen(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setg_swlen(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getg_swlen(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?4:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_g_swlen(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?64:0;
  return plOK;
}

char name_proc_lvd_qdc_g_swlen[] = "lvd_qdc_g_swlen";
int ver_proc_lvd_qdc_g_swlen = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: FPGA idx (0..3 or -1 for all four groups)
 *  p[4]: argument(s)]
 */

/* Integral window latency */

plerrcode proc_lvd_qdc_g_iwstart(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setg_iwstart(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getg_iwstart(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?4:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_g_iwstart(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?64:0;
  return plOK;
}

char name_proc_lvd_qdc_g_iwstart[] = "lvd_qdc_g_iwstart";
int ver_proc_lvd_qdc_g_iwstart = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: FPGA idx (0..3 or -1 for all four groups)
 *  p[4]: argument(s)]
 */

/* Integral window length */

plerrcode proc_lvd_qdc_g_iwlen(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setg_iwlen(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getg_iwlen(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?4:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_g_iwlen(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?64:0;
  return plOK;
}

char name_proc_lvd_qdc_g_iwlen[] = "lvd_qdc_g_iwlen";
int ver_proc_lvd_qdc_g_iwlen = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 4)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: FPGA idx (0..3 or -1 for all four groups)
 *  p[4]: argument(s)]
 */

/* Lookup table for inputs FPGA coincidences */

plerrcode proc_lvd_qdc_g_coinctab(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setg_coinctab(dev, ip[2], ip[3], p[4]);
    }
  else
    {
      pres=lvd_qdc_getg_coinctab(dev, ip[2], ip[3], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1)*(ip[3]<0?4:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_g_coinctab(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=4))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?64:0;
  return plOK;
}

char name_proc_lvd_qdc_g_coinctab[] = "lvd_qdc_g_coinctab";
int ver_proc_lvd_qdc_g_coinctab = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Set cr register */

plerrcode proc_lvd_qdc_m_cr(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_cr(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_cr(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_cr(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_cr[] = "lvd_qdc_m_cr";
int ver_proc_lvd_qdc_m_cr = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Enable module */

plerrcode proc_lvd_qdc_m_ena(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_ena(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_ena(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_ena(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_ena[] = "lvd_qdc_m_ena";
int ver_proc_lvd_qdc_m_ena = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Select trigger mode */

plerrcode proc_lvd_qdc_m_trgmode(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_trgmode(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_trgmode(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_trgmode(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_trgmode[] = "lvd_qdc_m_trgmode";
int ver_proc_lvd_qdc_m_trgmode = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Enable TDC on external trigger input */

plerrcode proc_lvd_qdc_m_tdcena(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_tdcena(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_tdcena(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_tdcena(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_tdcena[] = "lvd_qdc_m_tdcena";
int ver_proc_lvd_qdc_m_tdcena = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Fix baseline with last value */

plerrcode proc_lvd_qdc_m_fixbaseline(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_fixbaseline(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_fixbaseline(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_fixbaseline(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_fixbaseline[] = "lvd_qdc_m_fixbaseline";
int ver_proc_lvd_qdc_m_fixbaseline = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Switch on test signal */

plerrcode proc_lvd_qdc_m_tstsig(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_tstsig(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_tstsig(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_tstsig(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_tstsig[] = "lvd_qdc_m_tstsig";
int ver_proc_lvd_qdc_m_tstsig = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Switch on ADC power */

plerrcode proc_lvd_qdc_m_adcpwr(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_adcpwr(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_adcpwr(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_adcpwr(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_adcpwr[] = "lvd_qdc_m_adcpwr";
int ver_proc_lvd_qdc_m_adcpwr = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Switch to F1 mode */

plerrcode proc_lvd_qdc_m_f1mode(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_f1mode(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_f1mode(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_f1mode(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_f1mode[] = "lvd_qdc_m_f1mode";
int ver_proc_lvd_qdc_m_f1mode = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Lookup table for main coincidences */

plerrcode proc_lvd_qdc_m_grpcoinc(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_grpcoinc(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_grpcoinc(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_grpcoinc(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_grpcoinc[] = "lvd_qdc_m_grpcoinc";
int ver_proc_lvd_qdc_m_grpcoinc = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Set scaler readout rate */

plerrcode proc_lvd_qdc_m_scalerrout(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_scalerrout(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_scalerrout(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_scalerrout(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_scalerrout[] = "lvd_qdc_m_scalerrout";
int ver_proc_lvd_qdc_m_scalerrout = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Set coinmin_traw register */

plerrcode proc_lvd_qdc_m_coinmintraw(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_coinmintraw(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_coinmintraw(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_coinmintraw(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_coinmintraw[] = "lvd_qdc_m_coinmintraw";
int ver_proc_lvd_qdc_m_coinmintraw = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Raw cycle frequency */

plerrcode proc_lvd_qdc_m_traw(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_traw(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_traw(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_traw(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_traw[] = "lvd_qdc_m_traw";
int ver_proc_lvd_qdc_m_traw = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Minimum coincidence length */

plerrcode proc_lvd_qdc_m_coinmin(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_coinmin(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_coinmin(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_coinmin(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_coinmin[] = "lvd_qdc_m_coinmin";
int ver_proc_lvd_qdc_m_coinmin = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Test pulse level */

plerrcode proc_lvd_qdc_m_tpdac(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_tpdac(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_tpdac(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_tpdac(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_tpdac[] = "lvd_qdc_m_tpdac";
int ver_proc_lvd_qdc_m_tpdac = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Baseline correction cycle */

plerrcode proc_lvd_qdc_m_blcorcycle(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  if (p[0]>2)
    {
      pres=lvd_qdc_setm_blcorcycle(dev, ip[2], ip[3]);
    }
  else
    {
      pres=lvd_qdc_getm_blcorcycle(dev, ip[2], outptr);
      if (pres==plOK)
        outptr+=(ip[2]<0?16:1);
    }
  return pres;
}

plerrcode test_proc_lvd_qdc_m_blcorcycle(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if ((p[0]!=2) && (p[0]!=3))
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=p[0]<3?16:0;
  return plOK;
}

char name_proc_lvd_qdc_m_blcorcycle[] = "lvd_qdc_m_blcorcycle";
int ver_proc_lvd_qdc_m_blcorcycle = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Read actual baseline */

plerrcode proc_lvd_qdc_r_baseline(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  pres=lvd_qdc_getm_baseline(dev, ip[2], outptr);
  if (pres==plOK)
    outptr+=(ip[2]<0?16:1);
  return pres;
}

plerrcode test_proc_lvd_qdc_r_baseline(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if (p[0]!=2)
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=16;
  return plOK;
}

char name_proc_lvd_qdc_r_baseline[] = "lvd_qdc_r_baseline";
int ver_proc_lvd_qdc_r_baseline = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Read tcornoise register */

plerrcode proc_lvd_qdc_r_tcornoise(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  pres=lvd_qdc_getm_tcornoise(dev, ip[2], outptr);
  if (pres==plOK)
    outptr+=(ip[2]<0?16:1);
  return pres;
}

plerrcode test_proc_lvd_qdc_r_tcornoise(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if (p[0]!=2)
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=16;
  return plOK;
}

char name_proc_lvd_qdc_r_tcornoise[] = "lvd_qdc_r_tcornoise";
int ver_proc_lvd_qdc_r_tcornoise = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Read max tau correction */

plerrcode proc_lvd_qdc_r_tcor(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  pres=lvd_qdc_getm_tcor(dev, ip[2], outptr);
  if (pres==plOK)
    outptr+=(ip[2]<0?16:1);
  return pres;
}

plerrcode test_proc_lvd_qdc_r_tcor(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if (p[0]!=2)
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=16;
  return plOK;
}

char name_proc_lvd_qdc_r_tcor[] = "lvd_qdc_r_tcor";
int ver_proc_lvd_qdc_r_tcor = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Read max tau correction */

plerrcode proc_lvd_qdc_r_noise(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  pres=lvd_qdc_getm_noise(dev, ip[2], outptr);
  if (pres==plOK)
    outptr+=(ip[2]<0?16:1);
  return pres;
}

plerrcode test_proc_lvd_qdc_r_noise(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if (p[0]!=2)
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=16;
  return plOK;
}

char name_proc_lvd_qdc_r_noise[] = "lvd_qdc_r_noise";
int ver_proc_lvd_qdc_r_noise = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Capacity correction assessment */

plerrcode proc_lvd_qdc_r_dttauquality(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  pres=lvd_qdc_getm_dttauquality(dev, ip[2], outptr);
  if (pres==plOK)
    outptr+=(ip[2]<0?16:1);
  return pres;
}

plerrcode test_proc_lvd_qdc_r_dttauquality(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if (p[0]!=2)
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=16;
  return plOK;
}

char name_proc_lvd_qdc_r_dttauquality[] = "lvd_qdc_r_dttauquality";
int ver_proc_lvd_qdc_r_dttauquality = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (2 or 3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Read and clear overruns bits */

plerrcode proc_lvd_qdc_r_ovr(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  pres=lvd_qdc_getm_ovr(dev, ip[2], outptr);
  if (pres==plOK)
    outptr+=(ip[2]<0?16:1);
  return pres;
}

plerrcode test_proc_lvd_qdc_r_ovr(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if (p[0]!=2)
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=16;
  return plOK;
}

char name_proc_lvd_qdc_r_ovr[] = "lvd_qdc_r_ovr";
int ver_proc_lvd_qdc_r_ovr = 1;

/*****************************************************************************/
/*
 * p[0]: argcount (3)
 * p[1]: branch
 * p[2]: module addr (0..15 or -1 for all modules of appropriate type)
 * [p[3]: argument(s)]
 */

/* Generate test pulse */

plerrcode proc_lvd_qdc_w_ctrl(ems_u32* p)
{
  struct lvd_dev* dev=get_lvd_device(p[1]);
  plerrcode pres;
  ems_i32* ip=(ems_i32*)p;

  pres=lvd_qdc_setm_ctrl(dev, ip[2], ip[3]);
  return pres;
}

plerrcode test_proc_lvd_qdc_w_ctrl(ems_u32* p)
{
  struct lvd_dev* dev;
  plerrcode pres;

  if (p[0]!=3)
    return plErr_ArgNum;
  if ((pres=_find_lvd_odevice(p[1], &dev))!=plOK)
    return pres;

  wirbrauchen=0;
  return plOK;
}

char name_proc_lvd_qdc_w_ctrl[] = "lvd_qdc_w_ctrl";
int ver_proc_lvd_qdc_w_ctrl = 1;

