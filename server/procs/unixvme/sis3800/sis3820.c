/*
 * procs/unixvme/sis3800/sis3820.c
 * created 3.Feb.2013 h.xu
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: sis3820.c,v 1.9 2011/04/06 20:30:35 h.xu Exp $";

#include <sconf.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <errorcodes.h>
#include <modultypes.h>
#include <rcs_ids.h>
#include <unistd.h>
#include "../vme_verify.h"
#include "../../procs.h"
#include "../../../commu/commu.h"
#include "../../../objects/domain/dom_ml.h"
#include "../../../lowlevel/unixvme/vme.h"
#include "../../../objects/var/variables.h"
#include "../../../lowlevel/devices.h"
#include "../../../trigger/trigger.h"

#define SIS3820_KEY_RESET						0x400
#define SIS3820_MODID							0x4	


extern ems_u32* outptr;
extern int wirbrauchen;
extern int *memberlist;

#define lumis 21
static int lumi[lumis];

RCS_REGISTER(cvsid, "procs/unixvme/sis3800")

/*****************************************************************************/
/*
 * p[0]: argcount==number of modules
 * p[1...]: indices in memberlist
 */
plerrcode proc_sis3820init(ems_u32* p)
{
printf("The sis3820init!\n");
printf("The p[0] is %d\n",p[0]);
int i, res;

for (i=0; i<=p[0]; i++)
  {
  unsigned int help, id, version;
  ml_entry* module=ModulEnt(p[i]);
  struct vme_dev* dev=module->address.vme.dev;

//int rcsum=0;
//u_int32_t sis3820_base=0x38000000;
//u_int32_t modid;


   /* SIS3820 KEY Reset */
/*   rcsum+=dev->write_a32d32(dev,module->address.vme.addr+SIS3820_KEY_RESET,SIS3820_KEY_RESET);
	printf("the rcsum = %x \n", rcsum);
   if (rcsum!=0) {
	printf("error on access to SIS3820, return code %x\n",rcsum);
        rcsum=0;
   }
*/
   /* read Modid */
/*   rcsum+=dev->read_a32d32(dev,module->address.vme.addr+4,&modid);
printf("Module identification and firmware register reads: %8.8x \n",modid);
   if (rcsum!=0) {
	printf("error on access to SIS3820, return code %x\n",rcsum);
        rcsum=0;
   }
   else {
       printf("Module identification and firmware register reads: %8.8x \n",modid);
   }
*/

 


  /* read module identification */
  res=dev->read_a32d32(dev, module->address.vme.addr+4, &help);
printf("the res = %x \n", res);
printf("Module identification and firmware register reads: %8.8x \n",help);
  if (res!=4) return plErr_System;
  id=(help>>16)&0xffff;
  version=(help>>12)&0xf;
  if (id!=0x3820)
    {
    printf("sis3820init(idx=%d): id=%04x version=%d\n", i, id, version);
    return plErr_HWTestFailed;
    }
  printf("sis3820init(idx=%d): id=%04x version=%d\n", i, id, version);

  /* reset module */
 // dev->write_a32d32(dev, module->address.vme.addr+0x60, 0);

  /* clear FIFO */
  //dev->write_a32d32(dev, module->address.vme.addr+0x20, 0);

  /* enable external next */
  //dev->write_a32d32(dev, module->address.vme.addr+0x0, 0x10000);

  /* enable next clock logic */
  //dev->write_a32d32(dev, module->address.vme.addr+0x28, 0);

  /* dummy clock */
 // dev->write_a32d32(dev, module->address.vme.addr+0x24, 0);

  /* switch LED on (just for fun) */
  dev->write_a32d32(dev, module->address.vme.addr+0x0, 0x1 /*0x100: off*/);
  if (i<lumis) lumi[i]=1;
  }
return plOK;
}

plerrcode test_proc_sis3820init(ems_u32* p)
{
int i;

for (i=1; i<=p[0]; i++)
  {
  ml_entry* module;

  if (!valid_module(p[i], modul_vme, 0)) return plErr_ArgRange;
  module=ModulEnt(p[i]);
  if (module->modultype!=SIS_3820) return plErr_BadModTyp;
  }
wirbrauchen=0;
return plOK;
}

char name_proc_sis3820init[] = "sis3820init";
int ver_proc_sis3820init = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==number of modules
 * p[1]: indices in memberlist
 */
plerrcode proc_sis3820read(ems_u32* p)
{
int i,j,k,cycle, rcsum=0;
printf("proc_sis3820read\n");
printf("p[0] is %d\n",p[0]);
for (i=1; i<=p[0]; i++)
  {
  unsigned int help, *hlp, count, hh;
//ems_u32* blt_data[0x0001000];
//u_int32_t gotwords;
  ml_entry* module=ModulEnt(p[i]);
  struct vme_dev* dev=module->address.vme.dev;
  unsigned int addr=module->address.vme.addr;

  hlp=outptr++; count=0;
  /* fifo not empty? */  
	for (j=0; j<32; j++)
    dev->read_a32d32(dev, addr+0xA00+j*4, &help);

   for(cycle=0;cycle<2;cycle++) {
       if(cycle==1) {
	   printf("now with reference pulser on ch1 enabled\n");
           /* SIS3820 channel 1 reference pulser enable */
//           rcsum+=vme_A32D32_write(p,sis3820_base+SIS3820_CONTROL_STATUS,CTRL_REFERENCE_CH1_ENABLE);
//           if (rcsum!=0) {
//	      printf("error on access to SIS3820, return code %x\n",rcsum);
 //             rcsum=0;
 //          }

       }

     /* SIS3820 KEY enable */
     rcsum+=dev->write_a32d32(dev,addr+0x418,0x418);
     if (rcsum!=0) {
	printf("error on access to SIS3820, return code %x\n",rcsum);
        rcsum=0;
     }

     /* let the module count for 5s */
     printf("SIS3820 scaler counting\n"); 
     sleep(5);

     /* SIS3820 KEY disable */
     rcsum+=dev->write_a32d32(dev,addr + 0x41C, 0x41C);
     if (rcsum!=0) {
	  printf("error on access to SIS3820, return code %x\n",rcsum);
          rcsum=0;
     }

     /* readout of scaler data */
	for(j=0;j<32;j++)
     rcsum = dev->read_a32d32(dev,addr+0xA00+j*4,&help) ;
//     printf("gotwords %d\n",gotwords);
     if(rcsum==0) {
         printf("scaler data\n");
         k=0;
         for (j=0;j<8;j++) {
            for (i=0;i<4;i++) {
               k++;
	     //  printf("ch%2.2d %8.8x  ",k,blt_data[k-1]);
	    }
            printf("\n");
         }  
     } 
   } /* end of cycle loop */


/*
  dev->read_a32d32(dev, addr+0x800000, &help); 
  if (help&0x100)
    {
    printf("sis3820read(idx=%d): fifo empty\n", i);
    return plErr_HW;
    }
*/
  /* read data */
  do
    {
    dev->read_a32d32(dev, addr+0xA00, outptr++);
    count++;
    }
  while (dev->read_a32d32(dev, addr+0xA00, &hh), !(hh&0x100));
  *hlp=count;
  /* toggle LED on (just for fun) */
  if (i<lumis)
    {
    lumi[i]++;
    if ((lumi[i]>>(i-1))&1)
      dev->write_a32d32(dev, addr+0x0, 0x1);
    else
      dev->write_a32d32(dev, addr+0x0, 0x10000);
    }                                                                         
  }
return plOK;
}

plerrcode test_proc_sis3820read(ems_u32* p)
{
int i;
for (i=1; i<=p[0]; i++)
  {
  ml_entry* module;

  if (!valid_module(p[i], modul_vme, 0)) return plErr_ArgRange;
  module=ModulEnt(p[i]);
  if (module->modultype!=SIS_3820) return plErr_BadModTyp;
  }
wirbrauchen=p[0]+1;
return plOK;
}

char name_proc_sis3820read[] = "sis3820read";
int ver_proc_sis3820read = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==number of modules
 * p[1]: indices in memberlist
 */
plerrcode proc_sis3820clock(ems_u32* p)
{
int i;

for (i=1; i<=p[0]; i++)
  {
  ml_entry* module=ModulEnt(p[i]);
  struct vme_dev* dev=module->address.vme.dev;

  dev->write_a32d32(dev, module->address.vme.addr+0x24, 0);
  }
return plOK;
}

plerrcode test_proc_sis3820clock(ems_u32* p)
{
int i;
for (i=1; i<=p[0]; i++)
  {
  ml_entry* module;

  if (!valid_module(p[i], modul_vme, 0)) return plErr_ArgRange;
  module=ModulEnt(p[i]);
  if (module->modultype!=SIS_3820) return plErr_BadModTyp;
  }
wirbrauchen=p[0]+1;
return plOK;
}

char name_proc_sis3820clock[] = "sis3820clock";
int ver_proc_sis3820clock = 1;

