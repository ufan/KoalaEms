/*
 * procs/unixvme/mesytec/madc32.c
 * created 30.09,2011 h.xu
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: madc32.c,v 1.1 2011/07/05 15:22:04 huagen Exp $";

#include <sconf.h>
#include <debug.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <errorcodes.h>
#include <modultypes.h>
#include <stdlib.h>
#include <rcs_ids.h>
#include "../../../commu/commu.h"
#include "../../../objects/domain/dom_ml.h"
#include "../../../objects/var/variables.h"
#include "../../../lowlevel/unixvme/vme.h"
#include "../../../lowlevel/devices.h"
#include "../../../lowlevel/unixvme/sis3100/vme_sis.h"
/*#include "../../../lowlevel/perfspect/perfspect.h"*/
#include "../../../trigger/trigger.h"
#include "../../procs.h"
#include "../vme_verify.h"

#include <unistd.h>

#include "dev/pci/sis1100_var.h"
//#include "sis3100_vme_calls.h"


extern ems_u32* outptr;
extern int wirbrauchen;
extern int *memberlist;

RCS_REGISTER(cvsid, "procs/unixvme/mesytec")

/*
 * madc 32-channel ADC
 * A32D16 (registers) A32D32/BLT32 (data buffer)
 * reserves 65536 Byte
 */
/*
 // Address registers
 *0x6000: address_source
 *0x6002: address_reg
 *0x6004: module_id
 *0x6008: soft_reset
 *0x600E: firmware_revision

 // IRQ(ROACK)
 *0x6010: irq_level
 *0x6012: irq_vector
 *0x6014: irq_test
 *0x6016: irq_reset
 *0x6018: irq_threshold
 *0x601A: Max_transfer_data
 *0x601C: Withdraw IRQ

 // MCST CBLT
 *0x6020: cblt_mcst_control
 *0x6022: cblt_address
 *0x6024: mcst_address
 // FIFO handling
 *0x6030: buffer_data_length
 *0x6032: data_len_format
 *0x6034: readout_reset
 *0x6036: multievent
 *0x6038: marking_type
 *0x603A: start_acq
 *0x603C: fifo_reset
 *0x603E: data_ready

 // Operation mode
 *0x6040: bank_operation
 *0x6042: adc_resolution
 *0x6044: output_format
 *0x6046: adc_override
 *0x6048: slc_off
 *0x604A: skip_oorange
 *0x604C: Ignore Thresholds

 // Gate generator
 *0x6050: hold_delay0
 *0x6052: hold_delay1
 *0x6054: hold_width0
 *0x6056: hold_width1
 *0x6058: use_gg

 // IO
 *0x6060: input_range
 *0x6062: ECL_term
 *0x6064: ECL_gate1_osc
 *0x6066: ECL_fc_res
 *0x6068: ECL_busy
 *0x606A: NIM_gat1_osc
 *0x606C: NIM_fc_reset
 *0x606E: NIM_busy
 *0x6070: pulser_status

 // MRC
 *0x6080: rc_busno
 *0x6082: rc_modnum
 *0x6084: rc_opcode
 *0x6086: rc_adr
 *0x6088: rc_dat
 *0x608A: send return status

 // CTRA
 *0x6090: Reset_ctr_ab
 *0x6092: evctr_lo
 *0x6094: evctr_hi
 *0x6096: ts_sources
 *0x6098: ts_divisor
 *0x609C: ts_counter_lo
 *0x609E: ts_counter_hi

 // CRTB
 *0x60A0: adc_busy_time_lo
 *0x60A2: adc_busy_time_hi
 *0x60A4: gate1_time_lo
 *0x60A6: gate1_time_hi
 *0x60A8: time_0
 *0x60AA: time_1
 *0x60AC: time_2
 *0x60AE: stop_ctr
 *
 */

//static ems_u32 mtypes[]={mesytec_madc32, 0};

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: address source
*/
/*
 * 0: from board coder 
 * 1: from address_reg
 * 
*/

plerrcode proc_madc32address_reg_set(ems_u32* p)
{
    int i, res;
     
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x6000, p[1]);
    if (res!=2) return plErr_System;

    res=dev->write_a32d16(dev, addr+0x6002, p[2]);
    if (res!=2) return plErr_System;
    }
    
    return plOK;
}

plerrcode test_proc_madc32address_reg_set(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32address_reg_set[] = "madc32address_reg_set";
int ver_proc_madc32address_reg_set = 1;


/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: module id
*/
/*
 * 0xFF: if value = FF, the 8 high bits of base address are used (board coder) 
 * 
*/

plerrcode proc_madc32address_reg_stat(ems_u32* p)
{
    int i, res;
    ems_u16 mod_id, add_source, add_reg, fm_rev; 
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

       res=dev->read_a32d16(dev,addr+0x6000,&add_source);        
       printf("proc_madc32add_source: address source is :%d\n",add_source);
       printf("0: from board coder; 1: from address_registor\n");
       if (res!=2) return plErr_System;

       res=dev->read_a32d16(dev,addr+0x6002,&add_reg);        
       printf("proc_madc32add_registor: address registor is :%d\n",add_reg);
       printf("0: from board coder; 1: from address_registor\n");
       if (res!=2) return plErr_System;

       res=dev->read_a32d16(dev,addr+0x6004,&mod_id);        
       printf("proc_madc32module_id: module_id is :%d\n",mod_id);
       printf("value = 0xFF means the 8 high bits of base address are used (board coder)\n");
       if (res!=2) return plErr_System;

       res=dev->read_a32d16(dev,addr+0x600E,&fm_rev);        
       printf("proc_madc32firmware_rev: firmware revision is :%d\n",fm_rev);
       if (res!=2) return plErr_System;

    }
    
    return plOK;
}

plerrcode test_proc_madc32address_reg_stat(ems_u32* p)
{
    if (p[0] || p[0]>6)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32address_reg_stat[] = "madc32address_reg_stat";
int ver_proc_madc32address_reg_stat = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: marking_type
 */
/*
 * 0: event counter 
 * 1: time stamp
 * 2: extended time stamp( now also for independent bank operation)
*/

plerrcode proc_madc32marking_type(ems_u32* p)
{
    int i, res;
	ems_u16 type;

   printf("memberlist[0] is %d\n",memberlist[0]);
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x6038, p[1]);
    if (res!=2) return plErr_System;

    res=dev->read_a32d16(dev, addr+0x6038, &type);
    printf("The event marking type is : %d\n", type);
    printf("0: event counter; 1: time stamp; 2: extended time stamp (also for independent bank operation)\n");
    if (res!=2) return plErr_System;

    }
    return plOK;
}

plerrcode test_proc_madc32marking_type(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32marking_type[] = "madc32marking_type";
int ver_proc_madc32marking_type = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: bank mode set register
 */
/*
 * 0: b00 banks connected
 * 1: b01 operate banks independent
 * 2: b11 toggle mode for zero dead time (use with internal gate generators enabled)
*/
plerrcode proc_madc32bankset(ems_u32* p)
{
  int i, res;
  ems_u16 bank;
  for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    
    res=dev->write_a32d16(dev, addr+0x6040, p[1]);
    if (res!=2)
        return plErr_System;

       res=dev->read_a32d16(dev,addr+0x6040,&bank);        
       printf("proc_madc32bankset: ADC bank operation is :%d\n",bank);
       printf("0: connected; 1: independt; 2: joggle\n");
       if(res!=2) return plErr_System; 
  }
    return plOK;
}

plerrcode test_proc_madc32bankset(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32bankset[] = "madc32bankset";
int ver_proc_madc32bankset = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: bit set register
 */
/*
 * 0: 2k(800ns conversion time)
 * 1: 4k(1.6us conversion time)
 * 2: 4k hires(3.2us conversion time)
 * 3: 8k(6.4us conversion time)
 * 4: 8k hires(12.5us conv. time)
*/
plerrcode proc_madc32bitset(ems_u32* p)
{
  int i, res;
  for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    
    res=dev->write_a32d16(dev, addr+0x6042, p[1]);
    if (res!=2)
        return plErr_System;
  }
    return plOK;
}

plerrcode test_proc_madc32bitset(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32bitset[] = "madc32bitset";
int ver_proc_madc32bitset = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: range set register 2
 */
/*
 * 0: 4V 
 * 1: 10V
 * 2: 8V
*/

plerrcode proc_madc32rangeset(ems_u32* p)
{
    int i, res;

   printf("memberlist[0] is %d\n",memberlist[0]);
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x6060, p[1]);
    if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode test_proc_madc32rangeset(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32rangeset[] = "madc32rangeset";
int ver_proc_madc32rangeset = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: ECL_gate1_osc
 */
/*
 * 0: gate1 input
 * 1: oscillator input (also set 0x6096!!)
*/

plerrcode proc_madc32ECL_gate1_osc(ems_u32* p)
{
    int i, res;
	ems_u16 ecl_gate;

   printf("memberlist[0] is %d\n",memberlist[0]);
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x6064, p[1]);
    if (res!=2) return plErr_System;

	res=dev->read_a32d16(dev, addr+0x6064, &ecl_gate);
	if (res!=2) return plErr_System;
	printf("proc_madc32ECL_gate1_osc: input is %d\n", ecl_gate);
    printf("0: gate1 input; 1: oscillator input\n");
    }
    return plOK;
}

plerrcode test_proc_madc32ECL_gate1_osc(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32ECL_gate1_osc[] = "madc32ECL_gate1_osc";
int ver_proc_madc32ECL_gate1_osc = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: ECL_fc_res
 */
/*
 * 0: fast clear input
 * 1: reset time stamp oscillator input 
*/

plerrcode proc_madc32ECL_fc_res(ems_u32* p)
{
    int i, res;
	ems_u16 fc_res;

   printf("memberlist[0] is %d\n",memberlist[0]);
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x6066, p[1]);
    if (res!=2) return plErr_System;

	res=dev->read_a32d16(dev, addr+0x6066, &fc_res);
	if (res!=2) return plErr_System;
	printf("proc_madc32ECL_fc_res: input is %d\n", fc_res);
    printf("0: fast clear input; 1: reset time stamp oscillator input\n");
    }
    return plOK;
}

plerrcode test_proc_madc32ECL_fc_res(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32ECL_fc_res[] = "madc32ECL_fc_res";
int ver_proc_madc32ECL_fc_res = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: NIM_gate1_osc
 */
/*
 * 0: gate1 input
 * 1: oscillator input (also set 0x6096!!)
*/

plerrcode proc_madc32NIM_gate1_osc(ems_u32* p)
{
    int i, res;
	ems_u16 NIM_gate;

   printf("memberlist[0] is %d\n",memberlist[0]);
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x606A, p[1]);
    if (res!=2) return plErr_System;

	res=dev->read_a32d16(dev, addr+0x606A, &NIM_gate);
	if (res!=2) return plErr_System;
	printf("proc_madc32NIM_gate1_osc: input is %d\n", NIM_gate);
    printf("0: gate1 input; 1: oscillator input\n");
    }
    return plOK;
}

plerrcode test_proc_madc32NIM_gate1_osc(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32NIM_gate1_osc[] = "madc32NIM_gate1_osc";
int ver_proc_madc32NIM_gate1_osc = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: NIM_fc_res
 */
/*
 * 0: fast clear input
 * 1: reset time stamp oscillator input 
*/

plerrcode proc_madc32NIM_fc_res(ems_u32* p)
{
    int i, res;
	ems_u16 fc_res;

   printf("memberlist[0] is %d\n",memberlist[0]);
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x606C, p[1]);
    if (res!=2) return plErr_System;

	res=dev->read_a32d16(dev, addr+0x606C, &fc_res);
	if (res!=2) return plErr_System;
	printf("proc_madc32NIM_fc_res: input is %d\n", fc_res);
    printf("0: fast clear input; 1: reset time stamp oscillator input\n");
    }
    return plOK;
}

plerrcode test_proc_madc32NIM_fc_res(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32NIM_fc_res[] = "madc32NIM_fc_res";
int ver_proc_madc32NIM_fc_res = 1;



/*****************************************************************************/
/*
 * p[0]: argcount ==0
 */
plerrcode proc_madc32fiforeset(ems_u32* p)
{   int i, res;
//	ems_u16 fifo;
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

/**********************************************************/
/*modify the address from 603C to 6034 for debug purpose*/
    //   res=dev->write_a32d16(dev,addr+0x603C,p[1]);
       res=dev->write_a32d16(dev,addr+0x6034,p[1]);
       if(res!=2) return plErr_System;

/**********************************************************/
/*modify the address from 603C to 6034 for debug purpose*/
//		res=dev->read_a32d16(dev,addr+0x603C,&fifo);
//		res=dev->read_a32d16(dev,addr+0x6034,&fifo);
//        if(res!=2) return plErr_System;
//        printf("proc_madc32fiforeset: the fifo registor is: %d\n",fifo);
        
      }
    return plOK;
}

plerrcode test_proc_madc32fiforeset(ems_u32* p)
{
    if (p[0])
        return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32fiforeset[] = "madc32fiforeset";
int ver_proc_madc32fiforeset = 1;

/*****************************************************************************/
/*
 * p[0]: argcount ==0
 */
plerrcode proc_madc32multievent(ems_u32* p)
{   int i, res;
	ems_u16 fifo;
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

       res=dev->write_a32d16(dev,addr+0x6036,p[1]);
       if(res!=2) return plErr_System;

		res=dev->read_a32d16(dev,addr+0x6036,&fifo);
        if(res!=2) return plErr_System;
        printf("proc_madc32multievent: the multievent mode is: %d\n",fifo);
        printf("0: no(clear events, allows new conversion; 1=yes, unlimited transfer; 3=yes, but MADS transfers limited by amount of data\n");
      }
    return plOK;
}

plerrcode test_proc_madc32multievent(ems_u32* p)
{
    if (p[0])
        return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32multievent[] = "madc32multievent";
int ver_proc_madc32multievent = 1;


/*****************************************************************************/
/*
 * p[0]: argcount==0/1 
 */

static plerrcode madc32reset_module(int idx)
{
    ml_entry* module=ModulEnt(idx);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    int res, i;
    ems_u16 range,bit;
#if 0
       res=dev->write_a32d16(dev,addr+0x603C,0);
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6034,0);
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x603A,0x1); 
        if(res!=2) return plErr_System;
#endif

       res=dev->read_a32d16(dev,addr+0x6060,&range); 
       printf("proc_madc32reset: ADC input range :%d\n",range);
       if(res!=2) return plErr_System; 

       res=dev->read_a32d16(dev,addr+0x6042,&bit);        
       printf("proc_madc32reset: ADC bit resolution :%d\n",bit);
       if(res!=2) return plErr_System; 


    // check amnesia and write GEO address if necessary//
    printf("read addr 0x%x offs 0x600e\n", addr);

    // clear threshold memory //
    for (i=0; i<32; i++) {
        res=dev->write_a32d16(dev, addr+0x4000+2*i, 0x0);
        if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode proc_madc32reset(ems_u32* p)
{
    int i, pres=plOK;

    if (p[0] && (p[1]>-1)) {
        pres=madc32reset_module(p[1]);
    } else {
        for (i=memberlist[0]; i>0; i--) {
            pres=madc32reset_module(i);
            if (pres!=plOK)
                break;
        }
    }
    return pres;
}

plerrcode test_proc_madc32reset(ems_u32* p)
{
    if ((p[0]!=0) && (p[0]!=1))
        return plErr_ArgNum;
 
    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32reset[] = "madc32reset";
int ver_proc_madc32reset = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: level (level==0, IRQ off, priority 1..7)
 * p[2]: vector (IRQ return value, default 0)
 * p[3]: IRQ_threshold (default 1, max 8120)
 * p[4]: max_transfer_data(default 1,max 2047,usually the same or higher value used in irq_threshold)
 * p[5]: withdraw irq (default 1, withdraw IRQ when data empty)
 */
plerrcode proc_madc32irq_set(ems_u32* p)
{
  // ems_u16 level, vector, irq_thr, max_data, withdraw_irq;
   int i, res;
   for(i=0; i<memberlist[0];i++)
	{
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    
    if (p[0]>1) {
    res=dev->write_a32d16(dev, addr+0x6010, p[1]); /* level */
    if (res!=2)        return plErr_System;
    }

    if (p[0]>2) {
        res=dev->write_a32d16(dev, addr+0x6012, p[2]); /* vector */
        if (res!=2)     return plErr_System;
    }
    if (p[0]>3) {
        res=dev->write_a32d16(dev, addr+0x6018, p[3]); /* irq_threshold */
        if (res!=2)     return plErr_System;
    }
    if (p[0]>4) {
        res=dev->write_a32d16(dev, addr+0x601A, p[4]); /* max_transfer_data */
        if (res!=2)     return plErr_System;
    }   
    if (p[0]>5) {
        res=dev->write_a32d16(dev, addr+0x601C, p[5]); /* withdraw IRQ */
        if (res!=2)     return plErr_System;
    }     
	}

    return plOK;
}

plerrcode test_proc_madc32irq_set(ems_u32* p)
{
    if (p[0]<2 || p[0]>6)
        return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32irq_set[] = "madc32irq_set";
int ver_proc_madc32irq_set = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: level (level==0, IRQ off, priority 1..7)
 * p[2]: vector (IRQ return value, default 0)
 * p[3]: IRQ_threshold (default 1, max 8120)
 * p[4]: max_transfer_data(default 1,max 2047,usually the same or higher value used in irq_threshold)
 * p[5]: withdraw irq (default 1, withdraw IRQ when data empty)
 */
plerrcode proc_madc32irq_stat(ems_u32* p)
{
   ems_u16 level, vector, irq_thr, max_data, withdraw_irq;
   	
    ml_entry* module=ModulEnt(0);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    int res;
    
    res=dev->read_a32d16(dev, addr+0x6010, &level); /* level */
    printf("proc_madc32irq_stat: IRQ level is %d\n",level);
    if (res!=2)        return plErr_System;
   
    res=dev->read_a32d16(dev, addr+0x6012, &vector); /* vector */
    printf("proc_madc32irq_stat: IRQ vector is %d\n",vector);
    if (res!=2)     return plErr_System;

    res=dev->read_a32d16(dev, addr+0x6018, &irq_thr); /* irq_threshold */
    printf("proc_madc32irq_stat: IRQ threshold is %d\n",irq_thr);
    if (res!=2)     return plErr_System;
  
    res=dev->read_a32d16(dev, addr+0x601A, &max_data); /* max_transfer_data */
    printf("proc_madc32irq_stat: max_transfer_data is %d\n",max_data);
    if (res!=2)     return plErr_System;

    res=dev->read_a32d16(dev, addr+0x601C, &withdraw_irq); /* withdraw IRQ */
    printf("proc_madc32irq_stat: withdraw_irq is %d\n",withdraw_irq);
    if (res!=2)     return plErr_System;       
	

    return plOK;
}

plerrcode test_proc_madc32irq_stat(ems_u32* p)
{
    if (p[0] || p[0]>6)
        return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32irq_stat[] = "madc32irq_stat";
int ver_proc_madc32irq_stat = 1;


/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_madc32read_irq(ems_u32* p)
{
    int i, res;

    *outptr++=memberlist[0];
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data,level=(ems_u32)1;
        ems_u32 *help, vector,error;
        ems_u16 datalength;
       help=outptr++;

     res = dev->irq_ack(dev,level,&vector,&error);   

   /*Ready for IRQ triggered readout loop?*/

   //   if(status==1){
       res=dev->read_a32d16(dev,addr+0x6030,&datalength);
      if(res!=2) return plErr_System;
       printf("datalength is:%d\n",datalength);
       if(datalength>0){    
        for(i=0;i<datalength;i++)  {     
           res=dev->read_a32d32(dev,addr,&data);
         if (res!=4){
             *help=outptr-help-1;
             return plErr_System;
             }
         if((data&0x7000000)!=0x60000000&&(data&0x7000000)!=0x40000000)
          {
            *outptr++=data;
           }
//          printf("the no %d data is:0x%d\n",i,data);
     } 
}

/*
      do{
          res=dev->read_a32d32(dev,addr,&data);
          if(res!=4){
          *help=outptr-help-1;
           return plErr_System;

}
         if((data&0x70000000)!=0x60000000&&(data&0x70000000)!=0x40000000){
         *outptr++=data;
}
         printf("the data is:0x%d\n",data);

}while((data&0x70000000)!=0x60000000);
*/
       *help=outptr-help-1;

       res=dev->write_a32d16(dev,addr+0x6034,0);
//       printf("readout the buffer until Buserror emited!\n");
       if(res!=2) return plErr_System;
    } 
    return plOK;
}


plerrcode test_proc_madc32read_irq(ems_u32* p)
{
    if (p[0])
        return plErr_ArgNum;

    wirbrauchen=memberlist[0]*34*32;
    return plOK;
}

char name_proc_madc32read_irq[] = "madc32read_irq";
int ver_proc_madc32read_irq = 1;


/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: ts_sources
 */
/*
 * 0: VME internally
 * 1: external source
*/

plerrcode proc_madc32ts_source(ems_u32* p)
{
    int i, res;
    ems_u16 ts;

   printf("memberlist[0] is %d\n",memberlist[0]);
    for(i=memberlist[0];i>0;i--){
    ml_entry* module=ModulEnt(i);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;

    res=dev->write_a32d16(dev, addr+0x6096, p[1]);
    if (res!=2) return plErr_System;

	res=dev->read_a32d16(dev, addr+0x6096, &ts);
	if (res!=2) return plErr_System;
	printf("proc_madc32ts_source: input is %d\n", ts);
    printf("0: VME internal; 1: external source\n");
    }
    return plOK;
}

plerrcode test_proc_madc32ts_source(ems_u32* p)
{
    if (p[0]!=1)
        return plErr_ArgNum;
   wirbrauchen=0;
    return plOK;
}

char name_proc_madc32ts_source[] = "madc32ts_source";
int ver_proc_madc32ts_source = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_madc32read_one(ems_u32* p)
{
    int i, res;

    *outptr++=memberlist[0];
    for (i=1; i<=memberlist[0]; i++) 
   {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data;
        ems_u32 *help;
        ems_u16 datalength;
        ems_u16 status;

        help=outptr++;
   
 //      res=dev->read_a32d16(dev,addr+0x603E,&status); 
 //      printf("proc_madc32read_one: status of data :%d\n",status);
 //      if(res!=2) return plErr_System; 
/*
        do {
            res=dev->read_a32d32(dev, addr, &data);

            if (res!=4) {
                *help=outptr-help-1;
                continue;
              //  return plErr_System;
            }
            
            if ((data&0x7000000)!=0x6000000&&(data&0X7000000)!=0X0000022) { 
//valid word 
                // printf("v792read_one: valid word is coming\n");          
// if((data&0X7000000)!=0X0000001&&(data&0X7000000)!=0X0000022)                   
                 *outptr++=data;
            } else {
                continue;
            }
            *outptr++=data;
        } while ((data&0x7000000)!=0x4000000);
        *help=outptr-help-1;

        // read until code==6 (invalid word) and discard data 
        do {
            res=dev->read_a32d32(dev, addr, &data);
            if (res!=4) {
                return plErr_System;
            }
        } while ((data&0x7000000)!=0x6000000);
*/


	/*readout triggered by checking the register status */ 
       // usleep(12000);
  do{ 
      res=dev->read_a32d16(dev,addr+0x603E,&status);        
       if(res!=2) return plErr_System; 
     //  printf("proc_madc32read_one: status of data :%d\n",status);
    //  if(status!=0) goto TOBEREAD;
    } while(status==0);

//printf("proc_madc32read_one: status of data :%d\n",status);
//TOBEREAD:
      if(status!=0)
      {
         res=dev->read_a32d16(dev,addr+0x6030,&datalength);
         if(res!=2) return plErr_System;
       // printf("datalength is:%d\n",datalength);

        for(i=0;i<datalength;i++)  
          {     
            res=dev->read_a32d32(dev,addr,&data);
            if (res!=4)
             {
              *help=outptr-help-1;
              return plErr_System;
             }
            if((data&0x7000000)!=0x60000000&&(data&0x7000000)!=0x40000000)
              {
               *outptr++=data;
              }
           //  printf("the no %d data is:0x%d\n",i,data);
           } 
       } 

  //  }//  else continue;
  
       *help=outptr-help-1;
/****************************************************************************/
/* comment out reset for debug purpose on 20131126**************************/

       res=dev->write_a32d16(dev,addr+0x6034,0);
       if(res!=2) return plErr_System;
    }
     return plOK;
}

plerrcode test_proc_madc32read_one(ems_u32* p)
{
    if (p[0])
        return plErr_ArgNum;

    wirbrauchen=memberlist[0]*34*32;
// printf("test_proc_madc32read_one: skipped\n");
    return plOK;
}

char name_proc_madc32read_one[] = "madc32read_one";
int ver_proc_madc32read_one = 1;


/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_madc32read_multi(ems_u32* p)
{
//printf("proc_madc32read_multi: start to readout\n");
    int i, res;

    *outptr++=memberlist[0];
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data, *help;
        ems_u16 status;

        help=outptr++;

       res=dev->write_a32d16(dev,addr+0x603A,0x0); 
       printf("proc_madc32read_multi: stop acq!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6090,3);
       printf("proc_madc32read_multi: Reset all counters!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6036,2);
       printf("proc_madc32read_multi: multievent!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x603c,0);
       printf("proc_madc32read_multi: Fifo_reset!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6034,0);
       printf("proc_madc32read_multi: readout reset!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x603A,0x1); 
       printf("proc_madc32read_multi: start acq!\n");
       if(res!=2) return plErr_System;

       res=dev->read_a32d16(dev,addr+0x603E,&status); 
       printf("proc_madc32read_one: status of data :%d\n",status);
       if(res!=2) return plErr_System; 
        /* read until code==4 (trailer) */
  if(!(status & 0x1)) {
/*     printf("***** v1290readout: id=%1d not ready, status=0x%1x\n", */
/* 	   memberidx-1, status); */
    return 1;
  }

      for (i=0;i<1024;i++){
            res=dev->read_a32d32(dev, addr+4*i, &data);
            if (res!=4) {
                *help=outptr-help-1;
                return plErr_System;
            }
            *outptr++=data;
           printf("data is:0x%08X\n",data);

        }
// while ((data&0x7000000)!=0x4000000);
        *help=outptr-help-1;

       res=dev->write_a32d16(dev,addr+0x6034,0x0);
       printf("proc_madc32read_multi: readout reset!\n");
       if(res!=2) return plErr_System;

    }
    return plOK;
}

plerrcode test_proc_madc32read_multi(ems_u32* p)
{
#if 0
    plerrcode res;
#endif
    if (p[0])
        return plErr_ArgNum;
    wirbrauchen=memberlist[0]*34*32;
    return plOK;
}

char name_proc_madc32read_multi[] = "madc32read_multi";
int ver_proc_madc32read_multi = 1;



