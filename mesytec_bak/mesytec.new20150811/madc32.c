/*
 * procs/unixvme/caen/madc32.c
 * created 2011-07-05 h.xu
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: madc32.c,v 1.2 2011/08/02 15:15:08 huagen Exp $";

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
/*#include "../../../lowlevel/perfspect/perfspect.h"*/
#include "../../../trigger/trigger.h"
#include "../../procs.h"
#include "../vme_verify.h"

extern ems_u32* outptr;
extern int wirbrauchen;
extern int *memberlist;

RCS_REGISTER(cvsid, "procs/unixvme/caen")

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

#define ANY 0x0815

struct madc32_statist {
    struct timeval updated;
    u_int64_t scanned;  /* how often scan_events was called */
    u_int64_t events;   /* number of event headers found */
    u_int64_t words;    /* number of data words */
    u_int32_t timestamp_high;
    u_int32_t timestamp_low;
    u_int32_t triggers; /* trigger counter in footer */
};

struct madc32_private {
    ems_u32 module_id;
    ems_u32 marking_type;
    struct madc32_statist statist;
};

static struct madc32_statist madc32_statist;

/*****************************************************************************/

static struct madc32_private*
find_private_by_id(int id) {
    struct madc32_private *private;
    ml_entry *module;
    int i;

    if (memberlist) { /* iterate over memberlist */
        for (i=1; i<=memberlist[0]; i++) {
            module=&modullist->entry[memberlist[i]];
            if (module->modulclass==modul_vme &&
                module->modultype==mesytec_madc32 &&
                module->private_data &&
                module->private_length==sizeof(struct madc32_private)) {
                    private=(struct madc32_private*)module->private_data;
                    if (private->module_id==id)
                        return private;
            }
        }
    } else {          /* iterate over modullist */
        for (i=0; i<modullist->modnum; i++) {
            module=&modullist->entry[i];
            if (module->modulclass==modul_vme &&
                module->modultype==mesytec_madc32 &&
                module->private_data &&
                module->private_length==sizeof(struct madc32_private)) {
                    private=(struct madc32_private*)module->private_data;
                    if (private->module_id==id)
                        return private;
            }
        }
    }
    return 0; /* modules not found */
}

static plerrcode
madc32_clear_statist(ml_entry *module) {
    memset(&madc32_statist, 0, sizeof(struct madc32_statist));
    if (module) {
        struct madc32_private *private;
        private=(struct madc32_private*)module->private_data;
        if (private && module->private_length==sizeof(struct madc32_private))
            memset(&private->statist, 0, sizeof(struct madc32_statist));
    }
    return plOK;
}

static plerrcode
madc32_init_private(ml_entry *module, int module_id, int marking_type) {
    struct madc32_private *private;

    free(module->private_data);
    module->private_data=calloc(1, sizeof(struct madc32_private));
    module->private_length=sizeof(struct madc32_private);

    private=(struct madc32_private*)module->private_data;
    private->module_id=module_id;
    private->marking_type=marking_type;
    return plOK;
}

static void
madc32_scan_events(ems_u32 *data, int words)
{
    struct madc32_private *private=0;
    struct timeval now;
    int i;

    gettimeofday(&now, 0);
    madc32_statist.updated=now;
    madc32_statist.scanned++;

    for (i=0; i<words; i++) {
        ems_u32 d=data[i];
        switch (d&0xc0000000) {
        case 0x40000000: { /* header */
                int id=(d>>16)&0xff;
                int len=d&0xff;
                private=find_private_by_id(id);

                madc32_statist.events++;
                madc32_statist.words+=len;

                if (private) {
                    private->statist.updated=now;
                    private->statist.events++;
                    private->statist.words+=len;
                }
            }
            break;
        case 0xc0000000: /* footer */
            if (private) {
                int stamp=d&0x3fffffff;
                if (private->marking_type==0) { /* event counter */
                    private->statist.triggers=stamp;
                } else {                        /* time stamp */
                    private->statist.timestamp_low=stamp;
                }
            }
            break;
        case 0x80000000: /* does not exist */
            printf("madc32: illegal data word %08x\n", d);
            break;
        case 0x00000000: /* data, ext. timestamp or dummy */
            switch (d>>21) {
            case 0x00: /* dummy */
                /* do nothing */
                break;
            case 0x20: /* data */
                /* do nothing */
                break;
            case 0x24: /* extended timestamp */
                if (private) {
                    int stamp=d&0xffff;
                    private->statist.timestamp_high=stamp;
                }
                break;
            default:
                printf("madc32: illegal data word %08x\n", d);
            }
        }
    }
}
/*****************************************************************************/
/*
 * 'for_each_member' calls 'proc' for each member of the IS modullist or
 * (if the IS modullist does not exist) for each member of the global
 * modullist.
 * p[1] has to be the module index (given as -1). It will be replaced by
 * the actual module idx.
 */
static plerrcode
for_each_member(ems_u32 *p, ems_u32 modtype, plerrcode(*proc)(ems_u32*))
{
    ml_entry* module;
    ems_u32 *pc;
    plerrcode pres=plOK;
    int i, psize;

    /* create a copy of p */
    psize=4*(p[0]+1);
    pc=malloc(psize);
    if (!pc) {
        complain("madc32:for_each: %s", strerror(errno));
        return plErr_NoMem;
    }
    memcpy(pc, p, psize);

    if (memberlist) { /* iterate over memberlist */
        for (i=1; i<=memberlist[0]; i++) {
            module=&modullist->entry[memberlist[i]];
            if (module->modulclass==modul_vme &&
                    module->modultype==modtype) {
                pc[1]=i;
                pres=proc(pc);
                if (pres!=plOK)
                    break;
            }
        }
    } else {          /* iterate over modullist */
        for (i=0; i<modullist->modnum; i++) {
            module=&modullist->entry[i];
            if (module->modulclass==modul_vme &&
                    module->modultype==modtype) {
                pc[1]=i;
                pres=proc(pc);
                if (pres!=plOK)
                    break;
            }
        }
    }

    free(pc);
    return pres;
}
/*****************************************************************************/
/*
 * 'for_each_module' calls 'proc' for each member of the IS modullist or
 * (if the IS modullist does not exist) for each member of the global
 * modullist.
 */
static plerrcode
for_each_module(ems_u32 modtype, plerrcode(*proc)(ml_entry*))
{
    ml_entry* module;
    plerrcode pres=plOK;
    int i;

    if (memberlist) { /* iterate over memberlist */
        for (i=1; i<=memberlist[0]; i++) {
            module=&modullist->entry[memberlist[i]];
            if (module->modulclass==modul_vme &&
                    module->modultype==modtype) {
                pres=proc(module);
                if (pres!=plOK)
                    break;
            }
        }
    } else {          /* iterate over modullist */
        for (i=0; i<modullist->modnum; i++) {
            module=&modullist->entry[i];
            if (module->modulclass==modul_vme &&
                    module->modultype==modtype) {
                pres=proc(module);
                if (pres!=plOK)
                    break;
            }
        }
    }

    return pres;
}
/*****************************************************************************/
static int
nr_modules(ems_u32 modtype)
{
    ml_entry* module;
    int count=0, i;

    if (memberlist) { /* iterate over memberlist */
        for (i=1; i<=memberlist[0]; i++) {
            module=&modullist->entry[memberlist[i]];
            if (module->modulclass==modul_vme &&
                    module->modultype==modtype) {
                count++;
            }
        }
    } else {          /* iterate over modullist */
        for (i=0; i<modullist->modnum; i++) {
            module=&modullist->entry[i];
            if (module->modulclass==modul_vme &&
                    module->modultype==modtype) {
                count++;
            }
        }
    }

    return count;
}
/*****************************************************************************/
/*
 * p[0]: argcount==0/1
 * [p[1]: index of module; -1: all modules of intrumentation system]
 */

static plerrcode
madc32reset_module(int idx)
{
    ml_entry* module=ModulEnt(idx);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
//    ems_u16 stat;
    int res, i;

    // reset module and start to be readout//
       res=dev->write_a32d16(dev,addr+0x603A,0x0); 
       printf("proc_madc32reset: stop acq!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6036,0x0); 
       printf("proc_madc32reset: Single event readout!\n");
       if(res!=2) return plErr_System;
 
       res=dev->write_a32d16(dev,addr+0x6012,0x0);
       printf("proc_madc32reset: set IRQ Vector to 0!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6010,0x1);
       printf("proc_madc32reset: set IRQ level to 1!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6034,0x0);
       printf("proc_madc32reset: Reset fifo!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6044,0x1);
       printf("proc_madc32reset: Set bit resolution as 4k!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x603A,0x1); 
       printf("proc_madc32reset:start_acq!\n");
       if(res!=2) return plErr_System;

    // check amnesia and write GEO address if necessary//
    printf("read addr 0x%x offs 0x600e\n", addr);


    // clear threshold memory //
    for (i=0; i<32; i++) {
        res=dev->write_a32d16(dev, addr+0x4000+2*i, 0);
//printf("madc32reset: clear threshold memory!\n");
        if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode proc_madc32reset(ems_u32* p)
{ 
    int i, pres=plOK;
    if (p[0] && (p[1]>-1)) {
        pres=madc32reset_module(p[1]);
//printf("proc_madc32reset is OK4\n");
    } else {
        for (i=memberlist[0]; i>0; i--) {
//printf("proc_madc32reset: OK44\n");
            pres=madc32reset_module(i);
            if (pres!=plOK)
                break;
        }
    }
    return pres;
}

plerrcode test_proc_madc32reset(ems_u32* p)
{
#if 0
    plerrcode res;
#endif
 
    if ((p[0]!=0) && (p[0]!=1))
        return plErr_ArgNum;
  //   printf("test_proc_madc32reset: OK6\n");
  //   printf("memberlist=%p, mtypes=%p\n",memberlist,mtypes);
#if 0
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) {
        printf("test_proc_madc32reset: test_proc_vme failed!,return res=%d\n",res);
        return res;
    }
#endif
  //   printf("test_proc_v792reset  OK7\n");
  //   printf("res is: %d\n",res);

    wirbrauchen=0;
//printf("test_proc_madc32reset  OK8\n");
    return plOK;

}

char name_proc_madc32reset[] = "madc32reset";
int ver_proc_madc32reset = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_madc32read_one(ems_u32* p)
{
    int i, res;

    *outptr++=memberlist[0];
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data, *help;
        ems_u16 datalength;

        help=outptr++;
/*
       res=dev->write_a32d16(dev,addr+0x603A,0x0); 
       printf("proc_madc32read_one: stop acq!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6036,0x0); 
       printf("proc_madc32read_one: Single event readout!\n");
       if(res!=2) return plErr_System;
 
       res=dev->write_a32d16(dev,addr+0x6012,0x0);
       printf("proc_madc32read_one: set IRQ Vector to 0!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6010,0x1);
       printf("proc_madc32read_one: set IRQ level to 1!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x6034,0x0);
       printf("proc_madc32read_one: Reset fifo!\n");
       if(res!=2) return plErr_System;

       res=dev->write_a32d16(dev,addr+0x603A,0x1); 
       printf("proc_madc32read_one: start_acq!\n");
       if(res!=2) return plErr_System;
*/

       res=dev->read_a32d16(dev,addr+0x6030,&datalength);
      if(res!=2) return plErr_System;
       printf("datalength is:0x%d\n",datalength);

        /* read until code==4 (trailer) */
        do {
printf("proc_madc32read_one: start to read_1!\n");
            res=dev->read_a32d32(dev, addr, &data);
printf("proc_madc32read_one: res1 is %d\n", res);
printf("data is:0x%d\n",data);
            if (res!=4) {
                *help=outptr-help-1;
                return plErr_System;
            }
            *outptr++=data;
           printf("data is:0x%d\n",data);

        } while ((data&0x7000000)!=0x4000000);
        *help=outptr-help-1;

       res=dev->write_a32d16(dev,addr+0x6034,0x0);
       printf("proc_madc32read_one: readout reset!\n");
       if(res!=2) return plErr_System;

        /* read until code==6 (invalid word) and discard data */

    }
    return plOK;
}

plerrcode test_proc_madc32read_one(ems_u32* p)
{
#if 0
    plerrcode res;
#endif
    if (p[0])
        return plErr_ArgNum;
#if 0
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;
#endif
    wirbrauchen=memberlist[0]*34*32;
// printf("test_proc_madc32read_one: skipped\n");
    return plOK;
}

char name_proc_madc32read_one[] = "madc32read_one";
int ver_proc_madc32read_one = 1;

/*****************************************************************************/
/*
 * p[0]: argcount==7
 * p[1]: module idx (or -1 for all madc32 of this IS)
 * p[2]: module ID  (-1 if the default should be used (recommended))
 * p[3]: resolution (0..4) (-1: don't change the default)
 * p[4]: input_range (0..2) (-1: don't change the default)
 * p[5]: marking_type (0: event counter  1: time stamp  3: extended time stamp)
 * p[6]: sliding_scale off
 * p[7]: bank_operation (0..2)
 */

plerrcode
proc_madc32_init(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ems_u16 val;
    plerrcode pres;
    int res, i;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_init);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u16 module_id, marking_type;

        pres=plOK;

        /* read firmware revision */
        res=dev->read_a32d16(dev, addr+0x600e, &val);
        if (res!=2) {
            complain("madc32_init: firmware revision: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
        printf("madc32(%08x): fw=%02x.%02x\n", addr, (val>>8)&0xff, val&0xff);

        /* soft reset (there is no hard reset) */
        res=dev->write_a32d16(dev, addr+0x6008, 1);
        if (res!=2) {
            complain("madc32_init: soft reset: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* clear threshold memory */
        for (i=0; i<32; i++) {
            res=dev->write_a32d16(dev, addr+0x4000+2*i, 0);
            if (res!=2) {
                complain("madc32_init: clear thresholds: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* set module ID */
        if (ip[2]>=0) {
            res=dev->write_a32d16(dev, addr+0x6004, p[2]);
            if (res!=2) {
                complain("madc32_init: set module ID: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
            module_id=ip[2];
        } else {
            res=dev->read_a32d16(dev, addr+0x6042, &module_id);
            if (res!=2) {
                complain("madc32_init: read module ID: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* set resolution */
        if (ip[3]>=0) {
            res=dev->write_a32d16(dev, addr+0x6042, p[3]);
            if (res!=2) {
                complain("madc32_init: set resolution: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* set input range */
        if (ip[4]>=0) {
            res=dev->write_a32d16(dev, addr+0x6060, p[4]);
            if (res!=2) {
                complain("madc32_init: set input range: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* set marking type */
        marking_type=p[5];
        res=dev->write_a32d16(dev, addr+0x6038, marking_type);
        if (res!=2) {
            complain("madc32_init: set marking type: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set sliding scale */
        res=dev->write_a32d16(dev, addr+0x6048, p[6]);
        if (res!=2) {
            complain("madc32_init: sliding scale: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set bank operation */
        res=dev->write_a32d16(dev, addr+0x6040, p[7]);
        if (res!=2) {
            complain("madc32_init: bank operation: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        madc32_init_private(module, module_id, marking_type);

    }

    return pres;
}

plerrcode test_proc_madc32_init(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=7) {
        complain("madc32_init: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_init: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_init: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_init[] = "madc32_init";
int ver_proc_madc32_init = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==4
 * p[1]: module idx (or -1 for all madc32 of this IS)
 * p[2]: level
 * p[3]: vector
 * p[4]: irq threshold (maximimum 956)
 */

plerrcode
proc_madc32_irq(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_irq);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        /* level */
        res=dev->write_a32d16(dev, addr+0x6010, p[2]);
        if (res!=2) {
            complain("madc32_irq: set level: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* vector */
        res=dev->write_a32d16(dev, addr+0x6012, p[3]);
        if (res!=2) {
            complain("madc32_irq: set vector: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* irq threshold */
        res=dev->write_a32d16(dev, addr+0x6018, p[4]);
        if (res!=2) {
            complain("madc32_irq: set irq threshold: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_madc32_irq(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=4) {
        complain("madc32_irq: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_irq: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_irq: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_irq[] = "madc32_irq";
int ver_proc_madc32_irq = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3 or 33
 * p[1]: module idx (or -1 for all madc32 of this IS)
 *
 * A (with p[0]==3):
 * p[2]: channel (-1 for all channels)
 * p[3]: threshold (0x1FFF to disable the channel)
 *
 * B (with p[0]==33)
 * p[2..33]: threshold[0..31]
 */

plerrcode
proc_madc32_threshold(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res, i;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_threshold);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        if (p[0]==33) {       /* 32 individual channels */
             for (i=0; i<32; i++) {
                res=dev->write_a32d16(dev, addr+0x4000+2*i, p[i+2]);
                if (res!=2) {
                    complain("madc32_threshold: res=%d errno=%s",
                            res, strerror(errno));
                    return plErr_System;
                }
           }
       } else if (ip[2]<0) { /* argcount=3 and all channels */
            for (i=0; i<32; i++) {
                res=dev->write_a32d16(dev, addr+0x4000+2*i, p[3]);
                if (res!=2) {
                    complain("madc32_threshold: res=%d errno=%s",
                            res, strerror(errno));
                    return plErr_System;
                }
           }
        } else {              /* argcount=3 and one channel */
            res=dev->write_a32d16(dev, addr+0x4000+2*p[2], p[3]);
            if (res!=2) {
                complain("madc32_threshold: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }
    }

    return pres;
}

plerrcode test_proc_madc32_threshold(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=3 && p[0]!=33) {
        complain("madc32_threshold: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_threshold: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_threshold: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_threshold[] = "madc32_threshold";
int ver_proc_madc32_threshold = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==2 or 5
 * p[1]: module idx (or -1 for all madc32 of this IS)
 * p[2]: bank (-1 for both)
 * [p[3]: delay
 *  p[4]: width
 * ]
 */
plerrcode
proc_madc32_gate(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_gate);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u16 use_gg, bankpattern;

        pres=plOK;

        bankpattern=ip[2]<0?3:p[2]+1; /* -1 --> b11, 0 --> b01, 1 --> b10 */

        /* read old use_gg */
        if (ip[2]>=0) { /* only one gate changed, we need the current value */
            res=dev->read_a32d16(dev, addr+0x6058, &use_gg);
            if (res!=2) {
                complain("madc32_gate: read use_gg: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        } else {
            use_gg=0; /* no old value needed */
        }

        /* calculate new use_gg */
        if (p[0]<3) { /* disable gate(s) */
            use_gg&=!bankpattern;
        } else {      /* enable gate(s) */
            use_gg|=bankpattern;
            if (bankpattern&1) {
                res=dev->write_a32d16(dev, addr+0x6050, p[3]);
                if (res!=2) {
                    complain("madc32_gate: res=%d errno=%s",
                            res, strerror(errno));
                    return plErr_System;
                }
                res=dev->write_a32d16(dev, addr+0x6054, p[4]);
                if (res!=2) {
                    complain("madc32_gate: res=%d errno=%s",
                            res, strerror(errno));
                    return plErr_System;
                }
            }
            if (bankpattern&2) {
                res=dev->write_a32d16(dev, addr+0x6052, p[3]);
                if (res!=2) {
                    complain("madc32_gate: res=%d errno=%s",
                            res, strerror(errno));
                    return plErr_System;
                }
                res=dev->write_a32d16(dev, addr+0x6056, p[4]);
                if (res!=2) {
                    complain("madc32_gate: res=%d errno=%s",
                            res, strerror(errno));
                    return plErr_System;
                }
            }
        }

        res=dev->write_a32d16(dev, addr+0x6058, use_gg);
        if (res!=2) {
            complain("madc32_gate: write use_gg: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_madc32_gate(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=2 && p[0]!=4) {
        complain("madc32_gate: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_gate: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_gate: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_gate[] = "madc32_gate";
int ver_proc_madc32_gate = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==2
 * p[1]: module idx (or -1 for all madc32 of this IS)
 * p[2]: val
 */

plerrcode
proc_madc32_nimbusy(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_nimbusy);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        res=dev->write_a32d16(dev, addr+0x606e, p[2]);
        if (res!=2) {
            complain("madc32_nimbusy: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_madc32_nimbusy(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=2) {
        complain("madc32_nimbusy: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_nimbusy: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_nimbusy: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_nimbusy[] = "madc32_nimbusy";
int ver_proc_madc32_nimbusy = 1;
/*****************************************************************************/
/*
 * madc32_init_cblt enables (or disables) multicast and chained block readout
 * It operates over all madc32 modules of the instrumentation system.
 * It requires that all modules are in the same crate.
 * 
 * p[0]: argcount (0..2)
 * [p[1]: mcst_module   if not given: multicast disabled
 * [p[2]: cblt_module]] if not given: cblt disabled
 */
plerrcode
proc_madc32_init_cblt(ems_u32* p)
{
    ml_entry *mcst_mod=0;
    ml_entry *cblt_mod=0;
    ml_entry *module_first=0;
    ml_entry *module_last=0;
    int idx_start, idx_end, i;
    ems_u16 cblt_mcst_control=0;

    if (p[0]>0) { /* enable mcst */
        cblt_mcst_control|=1<<7;
        mcst_mod=ModulEnt(p[1]);
    } else {
        cblt_mcst_control|=1<<6;
    }

    if (p[0]>1) { /* enable cblt */
        cblt_mcst_control|=1<<1;
        cblt_mod=ModulEnt(p[2]);
    } else {
        cblt_mcst_control|=1<<0;
    }

    if (memberlist) {
        idx_start=1;
        idx_end=memberlist[0];
    } else {
        idx_start=0;
        idx_end=modullist->modnum-1;
    }

    printf("idx_start=%d idx_end=%d\n", idx_start, idx_end);
    for (i=idx_start; i<=idx_end; i++) {
        ml_entry *module=ModulEnt(i);
        struct vme_dev *dev;
        ems_u32 addr;
        int res;

        if (module->modulclass!=modul_vme ||
                    module->modultype!=mesytec_madc32) {
            printf("modules %d skipped\n", i);
            continue;
        }

        dev=module->address.vme.dev;
        addr=module->address.vme.addr;

        if (module_first==0 || addr<module_first->address.vme.addr)
            module_first=module;
        if (module_last==0 || addr>module_last->address.vme.addr)
            module_last=module;
        printf("module_first=%p module_last=%p\n", module_first, module_last);

        /* write multicast address */
        if (mcst_mod) {
            res=dev->write_a32d16(dev, addr+0x6024,
                    mcst_mod->address.vme.addr>>24);
            if (res!=2) {
                complain("madc32_cblt: mcst_address: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* write cblt address */
        if (cblt_mod) {
            res=dev->write_a32d16(dev, addr+0x6022,
                    cblt_mod->address.vme.addr>>24);
            if (res!=2) {
                complain("madc32_cblt: cblt_address: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* disable all mcst and cblt features */
        res=dev->write_a32d16(dev, addr+0x6020, 0x55);
        if (res!=2) {
            complain("madc32_cblt: cblt_control: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
        /* enable requested features */
        res=dev->write_a32d16(dev, addr+0x6020, cblt_mcst_control);
        if (res!=2) {
            complain("madc32_cblt: cblt_control: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    /* set first and last cblt module */
    if (p[0]>1) {
        /* sanity check */
        if (!module_first || !module_last) {
            complain("madc32_cblt: no modules");
            return plErr_BadISModList;
        }
        /* no error check here, because we already wrote to the register */
        module_first->address.vme.dev->write_a32d16(
                module_first->address.vme.dev,
                module_first->address.vme.addr+0x6020, 1<<5);
        module_last->address.vme.dev->write_a32d16(
                module_last->address.vme.dev,
                module_last->address.vme.addr+0x6020, 1<<3);
    }

    return plOK;
}

plerrcode
test_proc_madc32_init_cblt(ems_u32* p)
{
    ml_entry *module;
    if (p[0]>2) {
        complain(": illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (p[0]>0) { /* mcst address given */
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_init_cblt: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=vme_mcst) {
            complain("madc32_init_cblt: p[1](==%u): no mcst module", p[1]);
            return plErr_BadModTyp;
        }
    }

    if (p[0]>1) { /* cblt address given */
        if (!valid_module(p[2], modul_vme, 0)) {
            complain("madc32_init_cblt: p[2](==%u): no valid VME module", p[2]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[2]);
        if (module->modultype!=vme_cblt) {
            complain("madc32_init_cblt: p[2](==%u): no cblt module", p[2]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_init_cblt[] = "madc32_init_cblt";
int ver_proc_madc32_init_cblt = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3
 * p[1]: module idx
 * p[2]: multievent
 * p[3]: max_transfer_data
 */
plerrcode
proc_madc32_start_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;

    /* yes, this will be called multiple times, this is not harmfull */
    madc32_clear_statist(0);

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_start_simple);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        int res;

        pres=plOK;
        madc32_clear_statist(module);

        /* stop acq (should not be necessary) */
        res=dev->write_a32d16(dev, addr+0x603a, 0);
        if (res!=2) {
            complain("madc32_start_simple: stop acq: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set multievent */
        res=dev->write_a32d16(dev, addr+0x6036, p[2]);
        if (res!=2) {
            complain("madc32_start_simple: set multievent: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set max_transfer_data (956 is maximum) */
        res=dev->write_a32d16(dev, addr+0x601a, p[3]);
        if (res!=2) {
            complain("madc32_start_simple: set multievent: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* start acq */
        res=dev->write_a32d16(dev, addr+0x603a, 1);
        if (res!=2) {
            complain("madc32_start_simple: start acq: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* readout reset */
        res=dev->write_a32d16(dev, addr+0x6034, ANY);
        if (res!=2) {
            complain("madc32_start_simple: reset FIFO: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_madc32_start_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=3) {
        complain("madc32_start_simple: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_start_simple: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_start_simple: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_start_simple[] = "madc32_start_simple";
int ver_proc_madc32_start_simple = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3
 * p[1]: mcst_module
 * p[2]: multievent (0: single, 3: multi (2 not usable))
 * p[3]: max_transfer_data
 */
plerrcode
proc_madc32_start_cblt(ems_u32* p)
{
    ml_entry* mcst_module=ModulEnt(p[1]);
    struct vme_dev* dev=mcst_module->address.vme.dev;
    ems_u32 maddr=mcst_module->address.vme.addr;
    int res;

    madc32_clear_statist(0);
    for_each_module(mesytec_madc32, madc32_clear_statist);

    /* stop acq (should not be necessary) */
    res=dev->write_a32d16(dev, maddr+0x603a, 0);
    if (res!=2) {
        complain("madc32_start_cblt: stop acq: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /* set multievent */
    res=dev->write_a32d16(dev, maddr+0x6036, p[2]);
    if (res!=2) {
        complain("madc32_start_cblt: set multievent: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /*
     * set max_transfer_data (956 is maximum)
     * 0 (infinite) is also possible, but dangerous (transfer may never end)
     */
    res=dev->write_a32d16(dev, maddr+0x601a, p[3]);
    if (res!=2) {
        complain("madc32_start_cblt: set max_transfer_data: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /* start acq */
    res=dev->write_a32d16(dev, maddr+0x603a, 1);
    if (res!=2) {
        complain("madc32_start_cblt: start acq: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /* readout reset */
    res=dev->write_a32d16(dev, maddr+0x6034, ANY);
    if (res!=2) {
        complain("madc32_start_cblt: reset FIFO: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    return plOK;
}

plerrcode test_proc_madc32_start_cblt(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=3) {
        complain("madc32_start_cblt: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme, 0)) {
        complain("madc32_start_cblt: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }

    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("madc32_start_cblt: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_start_cblt[] = "madc32_start_cblt";
int ver_proc_madc32_start_cblt = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==2
 * p[1]: mcst_module
 */
plerrcode
proc_madc32_stop_cblt(ems_u32* p)
{
    ml_entry* mcst_module=ModulEnt(p[1]);
    struct vme_dev* dev=mcst_module->address.vme.dev;
    ems_u32 maddr=mcst_module->address.vme.addr;
    int res;

    /* stop acq */
    res=dev->write_a32d16(dev, maddr+0x603a, 0);
    if (res!=2) {
        complain("madc32_stop_cblt: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    return plOK;
}

plerrcode test_proc_madc32_stop_cblt(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=1) {
        complain("madc32_stop_cblt: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme, 0)) {
        complain("madc32_stop_cblt: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }

    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("madc32_stop_cblt: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_stop_cblt[] = "madc32_stop_cblt";
int ver_proc_madc32_stop_cblt = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: module idx
 */
plerrcode
proc_madc32_stop_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_stop_simple);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        int res;

        pres=plOK;

        res=dev->write_a32d16(dev, addr+0x603a, 0);
        if (res!=2) {
            complain("stop_simple: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_madc32_stop_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=1) {
        complain("stop_simple: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_stop_simple: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_stop_simple: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_stop_simple[] = "madc32_stop_simple";
int ver_proc_madc32_stop_simple = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: module idx
 * p[2]: max_words (per module)
 */
plerrcode
proc_madc32_read_simple(ems_u32* p)
{
    ems_u32 *oldoutptr=outptr;
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_madc32, proc_madc32_read_simple);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        /*ems_u16 val;*/
        int res;

        pres=plOK;

#if 0
        /* read data size */
        res=dev->read_a32d16(dev, addr+0x6030, &val);
        if (res!=2) {
            complain("madc32_read_simple: read length: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
#endif

        /* read data */
        oldoutptr=outptr;
        res=dev->read(dev, addr+0, 0xb, outptr, /*4*val*/4*p[2], 4, &outptr);
        if (res<0) {
            complain("madc32_read_simple: read: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* clear busy */
        res=dev->write_a32d16(dev, addr+0x6034, 0);
        if (res!=2) {
            complain("madc32_read_simple: clear busy: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
        madc32_scan_events(oldoutptr, outptr-oldoutptr);
    }

    return pres;
}

plerrcode test_proc_madc32_read_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=2) {
        complain("madc32_read_simple: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme, 0)) {
            complain("madc32_read_simple: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_madc32) {
            complain("madc32_read_simple: p[1](==%u): no mesytec madc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=(ip[1]>=0?1:nr_modules(mesytec_madc32))*p[2];
    return plOK;
}

char name_proc_madc32_read_simple[] = "madc32_read_simple";
int ver_proc_madc32_read_simple = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: mcst_module
 * p[2]: max_words (per module)
 */
plerrcode
proc_madc32_read_mcst(ems_u32* p)
{
    ml_entry *module, *mcst_module=ModulEnt(p[1]);
    struct vme_dev *dev, *mdev=mcst_module->address.vme.dev;
    ems_u32 addr, maddr=mcst_module->address.vme.addr;
    ems_u32 *oldoutptr=outptr;
    int res, i;

    if (memberlist) { /* iterate over memberlist */
        for (i=1; i<=memberlist[0]; i++) {
            module=&modullist->entry[memberlist[i]];
            if (module->modulclass==modul_vme &&
                    module->modultype==mesytec_madc32) {
                dev=module->address.vme.dev;
                addr=module->address.vme.addr;
                res=dev->read(dev, addr, 0xb, outptr, 4*p[2], 4, &outptr);
                if (res<0) {
                    complain("madc32_read_mcst: %s", strerror(errno));
                    return plErr_System;
                }
            }
        }
    } else {          /* iterate over modullist */
        for (i=0; i<modullist->modnum; i++) {
            module=&modullist->entry[i];
            if (module->modulclass==modul_vme &&
                    module->modultype==mesytec_madc32) {
                dev=module->address.vme.dev;
                addr=module->address.vme.addr;
                res=dev->read(dev, addr, 0xb, outptr, 4*p[2], 4, &outptr);
                if (res<0) {
                    complain("madc32_read_mcst: %s", strerror(errno));
                    return plErr_System;
                }
            }
        }
    }

    /* clear busy */
    res=mdev->write_a32d16(dev, maddr+0x6034, 0);
    if (res!=2) {
        complain("madc32_read_mcst: clear busy: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    madc32_scan_events(oldoutptr, outptr-oldoutptr);

    return plOK;
}

plerrcode test_proc_madc32_read_mcst(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=2) {
        complain("madc32_read_mcst: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme, 0)) {
        complain("madc32_read_mcst: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }
    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("madc32_read_mcst: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    wirbrauchen=nr_modules(mesytec_madc32)*p[2];
    return plOK;
}

char name_proc_madc32_read_mcst[] = "madc32_read_mcst";
int ver_proc_madc32_read_mcst = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3
 * p[1]: mcst_module
 * p[2]: cblt_module
 * p[3]: max words (should be equal to max_transfer_data in
 *       madc32_start_cblt times number of modules)
 */

plerrcode
proc_madc32_read_cblt(ems_u32* p)
{
    ml_entry* mcst_module=ModulEnt(p[1]);
    ml_entry* cblt_module=ModulEnt(p[2]);
    /* dev of mcst and cblt should be identical */
    struct vme_dev* dev=cblt_module->address.vme.dev;
    ems_u32 maddr=mcst_module->address.vme.addr;
    ems_u32 caddr=cblt_module->address.vme.addr;
    ems_u32 *oldoutptr=outptr;
    int res;

    /* read data */
    res=dev->read(dev, caddr+0, 0xb, outptr, 4*p[3], 4, &outptr);
    //printf("data read: %d\n", res/4);

    /* clear busy */
    res=dev->write_a32d16(dev, maddr+0x6034, 0);
    if (res!=2) {
        complain("madc32_read_cblt: clear busy: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    madc32_scan_events(oldoutptr, outptr-oldoutptr);

    return plOK;
}

plerrcode test_proc_madc32_read_cblt(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=3) {
        complain("madc32_read_cblt: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme, 0)) {
        complain("madc32_read_cblt: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }
    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("madc32_read_cblt: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    if (!valid_module(p[2], modul_vme, 0)) {
        complain("madc32_read_cblt: p[2](==%u): no valid VME module", p[2]);
        return plErr_ArgRange;
    }
    module=ModulEnt(p[2]);
    if (module->modultype!=vme_cblt) {
        complain("madc32_read_cblt: p[2](==%u): no cblt module", p[2]);
        return plErr_BadModTyp;
    }

    wirbrauchen=p[3];
    return plOK;
}

char name_proc_madc32_read_cblt[] = "madc32_read_cblt";
int ver_proc_madc32_read_cblt = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: module idx
 */

plerrcode
proc_madc32_status(ems_u32* p)
{
    ml_entry* module=ModulEnt(p[1]);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    ems_u16 val;
    int res;

    /* rfirmware revision */
    res=dev->read_a32d16(dev, addr+0x600e, &val);
    if (res!=2) {
        complain("madc32_status: firmware revision: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }
    printf("madc32(%08x): fw=%04x\n", addr, val);

    dev->read_a32d16(dev, addr+0x6000, &val);
    printf("6000 addr source   : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6002, &val);
    printf("6002 addr reg      : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6010, &val);
    printf("6010 irq level     : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6012, &val);
    printf("6012 irq vector    : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6018, &val);
    printf("6018 irq threshold : %04x\n", val);

    dev->read_a32d16(dev, addr+0x601a, &val);
    printf("601a max trans data: %04x\n", val);

    dev->read_a32d16(dev, addr+0x601c, &val);
    printf("601c withdraw irq  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6020, &val);
    printf("6020 cblt control  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6022, &val);
    printf("6022 cblt address  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6024, &val);
    printf("6024 mcst address  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6030, &val);
    printf("6030 buff data len : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6032, &val);
    printf("6032 data len form : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6036, &val);
    printf("6036 multievent    : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6038, &val);
    printf("6038 marking type  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x603a, &val);
    printf("603a start acq     : %04x\n", val);

    dev->read_a32d16(dev, addr+0x603e, &val);
    printf("603e data ready    : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6040, &val);
    printf("6040 bank operation: %04x\n", val);

    dev->read_a32d16(dev, addr+0x6042, &val);
    printf("6042 resolution    : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6044, &val);
    printf("6044 output format : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6046, &val);
    printf("6046 override      : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6048, &val);
    printf("6048 slc off       : %04x\n", val);

    dev->read_a32d16(dev, addr+0x604a, &val);
    printf("604a skip oorange  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6050, &val);
    printf("6050 hold delay0   : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6052, &val);
    printf("6052 hold delay1   : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6054, &val);
    printf("6054 hold width0   : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6056, &val);
    printf("6056 hold width1   : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6058, &val);
    printf("6058 use gg        : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6060, &val);
    printf("6060 input range   : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6062, &val);
    printf("6062 ecl term      : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6064, &val);
    printf("6064 ecl gate1 osc : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6066, &val);
    printf("6066 ecl fc res    : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6068, &val);
    printf("6068 ecl busy      : %04x\n", val);

    dev->read_a32d16(dev, addr+0x606a, &val);
    printf("606a nim gat1 osc  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x606c, &val);
    printf("606c nim fc reset  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x606e, &val);
    printf("606e nim busy      : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6070, &val);
    printf("6070, pulser status : %04x\n", val);

    /* counters A */
    dev->read_a32d16(dev, addr+0x6090, &val);
    printf("6090 reset ctr ab  : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6092, &val);
    printf("6092 evctr lo      : %04x\n", val);
    dev->read_a32d16(dev, addr+0x6094, &val);
    printf("6094 evctr hi      : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6096, &val);
    printf("6096 ts sources    : %04x\n", val);

    dev->read_a32d16(dev, addr+0x6098, &val);
    printf("6098 ts divisor    : %04x\n", val);

    dev->read_a32d16(dev, addr+0x609c, &val);
    printf("609c ts counter lo : %04x\n", val);
    dev->read_a32d16(dev, addr+0x609e, &val);
    printf("609e ts counter hi : %04x\n", val);

    /* counters B */
    dev->read_a32d16(dev, addr+0x60a0, &val);
    printf("60a0 adc bysy time lo: %04x\n", val);
    dev->read_a32d16(dev, addr+0x60a2, &val);
    printf("60a2 adc bysy time hi: %04x\n", val);

    dev->read_a32d16(dev, addr+0x60a4, &val);
    printf("60a4 gate1 time lo : %04x\n", val);
    dev->read_a32d16(dev, addr+0x60a6, &val);
    printf("60a6 gate1 time hi : %04x\n", val);

    dev->read_a32d16(dev, addr+0x60a8, &val);
    printf("60a8 time 0        : %04x\n", val);
    dev->read_a32d16(dev, addr+0x60aa, &val);
    printf("60aa time 1        : %04x\n", val);
    dev->read_a32d16(dev, addr+0x60ac, &val);
    printf("60ac time 2        : %04x\n", val);

    dev->read_a32d16(dev, addr+0x60ae, &val);
    printf("60ae stop ctr: %04x\n", val);


    return plOK;
}

plerrcode test_proc_madc32_status(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=1) {
        complain("madc32_status: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme, 0)) {
        complain("madc32_status: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }

    module=ModulEnt(p[1]);
    if (module->modultype!=mesytec_madc32) {
        complain("madc32_status: p[1](==%u): no mesytec madc32", p[1]);
        return plErr_BadModTyp;
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_madc32_status[] = "madc32_status";
int ver_proc_madc32_status = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: module idx (0: global statistics only
 *                   1: global and all modules)
 * 
 * outptr (repeated for all modules)
 *   [0]: number of following words for this module or global statistics
 *   [1[: module ID (-1 for global statistics)
 *   [2]: updated: tv.tv_sec
 *   [3]: updated: tv.tv_usec
 *   [4]: events
 *   [5]: words
 *   [6]: timestamp: high
 *   [7]: timestamp: low
 *   [8]: triggers
 */

static plerrcode
madc32_put_statist(ml_entry* module) {
    struct madc32_statist *statist=0;
    ems_u32 *help, *x;
    int id;

    if (module) {
        struct madc32_private *private;
        private=(struct madc32_private*)module->private_data;
        if (private && module->private_length==sizeof(struct madc32_private)) {
            statist=&private->statist;
            id=private->module_id;
        }
    } else {
        statist=&madc32_statist;
        id=-1;
    }

    if (!statist) {
        complain("madc32: no statistics for module %08x found",
                module->address.vme.addr);
        return plErr_Program;
    }

    help=outptr++;
    *outptr++=id;
    *outptr++=statist->updated.tv_sec;
    *outptr++=statist->updated.tv_usec;
    *outptr++=statist->scanned>>32;
    *outptr++=statist->scanned&0xffffffff;
    *outptr++=statist->events>>32;
    *outptr++=statist->events&0xffffffff;
    *outptr++=statist->words>>32;
    *outptr++=statist->words&0xffffffff;
    *outptr++=statist->timestamp_high;
    *outptr++=statist->timestamp_low;
    *outptr++=statist->triggers;
    *help=outptr-help-1;

    for (x=help; x<outptr; x++) {
        printf("[%2ld] %08x\n", x-help, *x);
    }

    return plOK;
}

plerrcode
proc_madc32_statist(ems_u32* p)
{
    madc32_put_statist(0);
    if (p[1]>0) {
        for_each_module(mesytec_madc32, madc32_put_statist);
    }
    return plOK;
}

plerrcode test_proc_madc32_statist(ems_u32* p)
{
    if (p[0]!=1) {
        complain("madc32_evnr: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    wirbrauchen=-1;
    return plOK;
}

char name_proc_madc32_statist[] = "madc32_statist";
int ver_proc_madc32_statist = 1;
/*****************************************************************************/
/*****************************************************************************/
