/*
 * procs/unixvme/caen/mtdc32.c
 * created 2015-09-04 PeWue
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: mtdc32.c,v 1.1 2015/09/05 00:18:12 wuestner Exp $";

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
 * mtdc 32-channel ADC
 * A32D16 (registers) A32D32/BLT32 (data buffer)
 * reserves 65536 Byte
 */
/*
 // Address registers
 *0x0000: data fifo (65464 words)
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
 *0x6042: tdc_resolution
 *0x6044: output_format
 // Trigger
 *0x6050: bank0_win_start
 *0x6052: bank1_win_start
 *0x6054: bank0_win_width
 *0x6056: bank1_win_width
 *0x6058: bank0_trig_source
 *0x605a: bank1_trig_source
 *0x605c: first_hit
 // Inputs, outputs
 *0x6060: Negative_edge
 *0x6062: ECL_term
 *0x6064: ECL_trig1_osc
 *0x6068: Trig_select
 *0x606a: NIM_trig1_osc
 *0x606e: NIM_busy
 // Test pulser / Threshold
 *0x6070: pulser_status;
 *0x6070: pulser_pattern
 *0x6070: bank0_input_thr
 *0x6070: bank1_input_thr
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
 *0x60A8: time_0
 *0x60AA: time_1
 *0x60AC: time_2
 *0x60AE: stop_ctr
 // Multiplicity filter
 *0x60b0: high_limit0
 *0x60b0: low_limit0
 *0x60b0: high-limit1
 *0x60b0: low-limit1
 */

#define ANY 0x0815

struct mtdc32_statist {
    struct timeval updated;
    u_int64_t scanned;  /* how often scan_events was called */
    u_int64_t events;   /* number of event headers found */
    u_int64_t words;    /* number of data words */
    u_int32_t timestamp_high;
    u_int32_t timestamp_low;
    u_int32_t triggers; /* trigger counter in footer */
};

struct mtdc32_private {
    ems_u32 module_id;
    ems_u32 marking_type;
    ems_u32 output_format;
    struct mtdc32_statist statist;
};

static struct mtdc32_statist mtdc32_statist;

/*****************************************************************************/
static struct mtdc32_private*
find_private_by_id(int id)
{
    struct mtdc32_private *private;
    ml_entry *module;
    int i;

    if (memberlist) { /* iterate over memberlist */
        for (i=1; i<=memberlist[0]; i++) {
            module=&modullist->entry[memberlist[i]];
            if (module->modulclass==modul_vme &&
                module->modultype==mesytec_mtdc32 &&
                module->private_data &&
                module->private_length==sizeof(struct mtdc32_private)) {
                    private=(struct mtdc32_private*)module->private_data;
                    if (private->module_id==id)
                        return private;
            }
        }
    } else {          /* iterate over modullist */
        for (i=0; i<modullist->modnum; i++) {
            module=&modullist->entry[i];
            if (module->modulclass==modul_vme &&
                module->modultype==mesytec_mtdc32 &&
                module->private_data &&
                module->private_length==sizeof(struct mtdc32_private)) {
                    private=(struct mtdc32_private*)module->private_data;
                    if (private->module_id==id)
                        return private;
            }
        }
    }
    return 0; /* modules not found */
}

static plerrcode
mtdc32_clear_statist(ml_entry *module)
{
    memset(&mtdc32_statist, 0, sizeof(struct mtdc32_statist));
    if (module) {
        struct mtdc32_private *private;
        private=(struct mtdc32_private*)module->private_data;
        if (private && module->private_length==sizeof(struct mtdc32_private))
            memset(&private->statist, 0, sizeof(struct mtdc32_statist));
    }
    return plOK;
}

static plerrcode
mtdc32_init_private(ml_entry *module, int module_id, int marking_type,
        int output_format)
{
    struct mtdc32_private *private;

    free(module->private_data);
    module->private_data=calloc(1, sizeof(struct mtdc32_private));
    module->private_length=sizeof(struct mtdc32_private);

    private=(struct mtdc32_private*)module->private_data;
    private->module_id=module_id;
    private->marking_type=marking_type;
    private->output_format=output_format;
    return plOK;
}

static void
mtdc32_scan_events(ems_u32 *data, int words)
{
    struct mtdc32_private *private=0;
    struct timeval now;
    int i;

    gettimeofday(&now, 0);
    mtdc32_statist.updated=now;
    mtdc32_statist.scanned++;

    for (i=0; i<words; i++) {
        ems_u32 d=data[i];
        switch (d&0xc0000000) {
        case 0x40000000: { /* header */
                int id=(d>>16)&0xff;
                int len=d&0xfff;
                private=find_private_by_id(id);

                mtdc32_statist.events++;
                mtdc32_statist.words+=len;

                if (private) {
                    private->statist.updated=now;
                    private->statist.events++;
                    private->statist.words+=len;
                }
            }
            break;
        case 0xc0000000: /* footer */
#if 0
            if (private) {
                int stamp=d&0x3fffffff;
                if (private->marking_type==0) { /* event counter */
                    private->statist.triggers=stamp;
                } else {                        /* time stamp */
                    private->statist.timestamp_low=stamp;
                }
            }
#endif
            break;
        case 0x80000000: /* does not exist */
            printf("mtdc32: illegal data word %08x\n", d);
            break;
#if 0
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
                printf("mtdc32: illegal data word %08x\n", d);
            }
#endif
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
 *
 * Warning: because of the static variable (for_each_member_idx) this
 * function ist not reenrant. But ems server does not use threads...
 */

static int for_each_member_idx=-1;

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
        complain("mtdc32:for_each: %s", strerror(errno));
        return plErr_NoMem;
    }
    memcpy(pc, p, psize);

    if (memberlist) { /* iterate over memberlist */
        for (i=1; i<=memberlist[0]; i++) {
            module=&modullist->entry[memberlist[i]];
            if (module->modulclass==modul_vme &&
                    module->modultype==modtype) {
                pc[1]=i;
                for_each_member_idx++;
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
                for_each_member_idx++;
                pres=proc(pc);
                if (pres!=plOK)
                    break;
            }
        }
    }

    for_each_member_idx=-1;
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
 * p[0]: argcount==8
 * p[1]: module idx (or -1 for all mtdc32 of this IS)
 * p[2]: module ID
 *         -1: the default should be used
 *         !!! use mtdc32_id to set usefull IDs
 *         >=0: ID is used for the first module and incremented for each
 *              following module
 * p[3]: resolution (2..9) (-1: don't change the default)
 * p[4]: negative_edge (2 bits) (-1: don't change the default)
 * p[5]: marking_type (0: event counter  1: time stamp  3: extended time stamp)
 * p[6]: output format (0: time difference 1: single hit full time stamp)
 * p[7]: bank_operation (0..2)
 * p[8]: first hit only
 */

plerrcode
proc_mtdc32_init(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ems_u16 val;
    plerrcode pres;
    int res;

printf("proc_mtdc32_init: p[1]=%d, idx=%d\n", ip[1], for_each_member_idx);

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_init);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u16 module_id, marking_type, output_format;

        pres=plOK;

        /* read firmware revision */
        res=dev->read_a32d16(dev, addr+0x600e, &val);
        if (res!=2) {
            complain("mtdc32_init: firmware revision: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
        printf("mtdc32(%08x): fw=%02x.%02x\n", addr, (val>>8)&0xff, val&0xff);

        /* soft reset (there is no hard reset) */
        res=dev->write_a32d16(dev, addr+0x6008, 1);
        if (res!=2) {
            complain("mtdc32_init: soft reset: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set module ID */
        if (ip[2]<0 || ip[2]==0xff) {
            res=dev->write_a32d16(dev, addr+0x6004, 0xff);
            if (res!=2) {
                complain("mtdc32_init: set module ID: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
            module_id=(addr>>24)&0xff;
        } else {
            module_id=ip[2];
            if (for_each_member_idx>=0)
                module_id+=for_each_member_idx;
            res=dev->write_a32d16(dev, addr+0x6004, module_id);
            if (res!=2) {
                complain("mtdc32_init: set module ID: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* set resolution */
        if (ip[3]>=0) {
            res=dev->write_a32d16(dev, addr+0x6042, p[3]);
            if (res!=2) {
                complain("mtdc32_init: set resolution: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* set negative edge */
        if (ip[4]>=0) {
            res=dev->write_a32d16(dev, addr+0x6060, p[4]);
            if (res!=2) {
                complain("mtdc32_init: set negative edge: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* set marking type */
        marking_type=p[5];
        res=dev->write_a32d16(dev, addr+0x6038, marking_type);
        if (res!=2) {
            complain("mtdc32_init: set marking type: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set output format */
        output_format=p[6];
        res=dev->write_a32d16(dev, addr+0x6044, p[6]);
        if (res!=2) {
            complain("mtdc32_init: set output format: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set bank operation */
        res=dev->write_a32d16(dev, addr+0x6040, p[7]);
        if (res!=2) {
            complain("mtdc32_init: set bank operation: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set first_hit */
        res=dev->write_a32d16(dev, addr+0x605c, p[8]);
        if (res!=2) {
            complain("mtdc32_init: set first_hit: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        mtdc32_init_private(module, module_id, marking_type, output_format);
    }

    return pres;
}

plerrcode test_proc_mtdc32_init(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=8) {
        complain("mtdc32_init: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_init: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_init: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_init[] = "mtdc32_init";
int ver_proc_mtdc32_init = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==4
 * p[1]: module idx (or -1 for all mtdc32 of this IS)
 * p[2]: level
 * p[3]: vector
 *         <0: -vector is used for all modules
 *         >=0: vector is used for the first module and incremented for each
 *              following module
 * p[4]: irq threshold (maximimum 956)
 */

plerrcode
proc_mtdc32_irq(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res, vector;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_irq);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        /* level */
        res=dev->write_a32d16(dev, addr+0x6010, p[2]);
        if (res!=2) {
            complain("mtdc32_irq: set level: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* vector */
        if (ip[3]<0) {
            vector=-ip[3];
        } else {
            vector=ip[3];
            if (for_each_member_idx>=0)
                vector+=for_each_member_idx;
        }
        res=dev->write_a32d16(dev, addr+0x6012, vector);
        if (res!=2) {
            complain("mtdc32_irq: set vector: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* irq threshold */
        res=dev->write_a32d16(dev, addr+0x6018, p[4]);
        if (res!=2) {
            complain("mtdc32_irq: set irq threshold: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_mtdc32_irq(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=4) {
        complain("mtdc32_irq: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_irq: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_irq: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_irq[] = "mtdc32_irq";
int ver_proc_mtdc32_irq = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==2
 * p[1]: module idx (or -1 for all mtdc32 of this IS)
 * p[2]: val
 */

plerrcode
proc_mtdc32_nimbusy(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_nimbusy);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        res=dev->write_a32d16(dev, addr+0x606e, p[2]);
        if (res!=2) {
            complain("mtdc32_nimbusy: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_mtdc32_nimbusy(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=2) {
        complain("mtdc32_nimbusy: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_nimbusy: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_nimbusy: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_nimbusy[] = "mtdc32_nimbusy";
int ver_proc_mtdc32_nimbusy = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==2
 * p[1]: module idx (or -1 for all mtdc32 of this IS)
 * p[2]: selection   (0x6068)
 * p[3]: termination (0x6062)
 */

plerrcode
proc_mtdc32_trigger(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_trigger);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        res=dev->write_a32d16(dev, addr+0x6068, p[2]);
        if (res!=2) {
            complain("mtdc32_trigger: set selction: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        res=dev->write_a32d16(dev, addr+0x6062, p[2]);
        if (res!=2) {
            complain("mtdc32_trigger: set ecl_term: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_mtdc32_trigger(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=3) {
        complain("mtdc32_trigger: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_trigger: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_trigger: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_trigger[] = "mtdc32_trigger";
int ver_proc_mtdc32_trigger = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3
 * p[1]: module idx (or -1 for all mtdc32 of this IS)
 * p[2]: bank (-1 for both)
 * p[3]: value
 */
plerrcode
proc_mtdc32_threshold(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_threshold);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        if (ip[2]<0 || ip[2]==0) {
            res=dev->write_a32d16(dev, addr+0x6078, p[3]);
            if (res!=2) {
                complain("mtdc32_threshold: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        if (ip[2]<0 || ip[2]==1) {
            res=dev->write_a32d16(dev, addr+0x607a, p[3]);
            if (res!=2) {
                complain("mtdc32_threshold: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }
    }

    return pres;
}

plerrcode test_proc_mtdc32_threshold(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=3) {
        complain("mtdc32_threshold: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_threshold: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_threshold: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_threshold[] = "mtdc32_threshold";
int ver_proc_mtdc32_threshold = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==5
 * p[1]: module idx (or -1 for all mtdc32 of this IS)
 * p[2]: bank (-1 for both)
 * p[3]: start
 * p[4]: width
 * p[5]: source
 */
plerrcode
proc_mtdc32_window(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;
    int res;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_window);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        pres=plOK;

        if (ip[2]<0 || ip[2]==0) {
            res=dev->write_a32d16(dev, addr+0x6050, p[3]);
            if (res!=2) {
                complain("mtdc32_window: start: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
            res=dev->write_a32d16(dev, addr+0x6054, p[4]);
            if (res!=2) {
                complain("mtdc32_window: width: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
            res=dev->write_a32d16(dev, addr+0x6058, p[5]);
            if (res!=2) {
                complain("mtdc32_window: source: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        if (ip[2]<0 || ip[2]==1) {
            res=dev->write_a32d16(dev, addr+0x6052, p[3]);
            if (res!=2) {
                complain("mtdc32_window: start: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
            res=dev->write_a32d16(dev, addr+0x6056, p[4]);
            if (res!=2) {
                complain("mtdc32_window: width: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
            res=dev->write_a32d16(dev, addr+0x605a, p[5]);
            if (res!=2) {
                complain("mtdc32_window: source: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }
    }

    return pres;
}

plerrcode test_proc_mtdc32_window(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=5) {
        complain("mtdc32_window: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_window: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_window: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_window[] = "mtdc32_window";
int ver_proc_mtdc32_window = 1;
/*****************************************************************************/
/*
 * mtdc32_init_cblt enables (or disables) multicast and chained block readout
 * It operates over all mtdc32 modules of the instrumentation system.
 * It requires that all modules are in the same crate.
 * 
 * p[0]: argcount (0..2)
 * [p[1]: mcst_module   if not given: multicast disabled
 * [p[2]: cblt_module]] if not given: cblt disabled
 */
plerrcode
proc_mtdc32_init_cblt(ems_u32* p)
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
                    module->modultype!=mesytec_mtdc32) {
            printf("module %d skipped\n", i);
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
                complain("mtdc32_cblt: mcst_address: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* write cblt address */
        if (cblt_mod) {
            res=dev->write_a32d16(dev, addr+0x6022,
                    cblt_mod->address.vme.addr>>24);
            if (res!=2) {
                complain("mtdc32_cblt: cblt_address: res=%d errno=%s",
                        res, strerror(errno));
                return plErr_System;
            }
        }

        /* disable all mcst and cblt features */
        res=dev->write_a32d16(dev, addr+0x6020, 0x55);
        if (res!=2) {
            complain("mtdc32_cblt: cblt_control: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
        /* enable requested features */
        res=dev->write_a32d16(dev, addr+0x6020, cblt_mcst_control);
        if (res!=2) {
            complain("mtdc32_cblt: cblt_control: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    /* set first and last cblt module */
    if (p[0]>1) {
        /* sanity check */
        if (!module_first || !module_last) {
            complain("mtdc32_cblt: no modules");
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
test_proc_mtdc32_init_cblt(ems_u32* p)
{
    ml_entry *module;
    if (p[0]>2) {
        complain(": illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (p[0]>0) { /* mcst address given */
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_init_cblt: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=vme_mcst) {
            complain("mtdc32_init_cblt: p[1](==%u): no mcst module", p[1]);
            return plErr_BadModTyp;
        }
    }

    if (p[0]>1) { /* cblt address given */
        if (!valid_module(p[2], modul_vme)) {
            complain("mtdc32_init_cblt: p[2](==%u): no valid VME module", p[2]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[2]);
        if (module->modultype!=vme_cblt) {
            complain("mtdc32_init_cblt: p[2](==%u): no cblt module", p[2]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_init_cblt[] = "mtdc32_init_cblt";
int ver_proc_mtdc32_init_cblt = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3
 * p[1]: module idx
 * p[2]: multievent
 * p[3]: max_transfer_data
 */
plerrcode
proc_mtdc32_start_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;

    /* yes, this will be called multiple times, this is not harmfull */
    mtdc32_clear_statist(0);

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_start_simple);
    } else {
        ml_entry* module=ModulEnt(p[1]);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        int res;

        pres=plOK;
        mtdc32_clear_statist(module);

        /* stop acq (should not be necessary) */
        res=dev->write_a32d16(dev, addr+0x603a, 0);
        if (res!=2) {
            complain("mtdc32_start_simple: stop acq: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set multievent */
        res=dev->write_a32d16(dev, addr+0x6036, p[2]);
        if (res!=2) {
            complain("mtdc32_start_simple: set multievent: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* set max_transfer_data (956 is maximum) */
        res=dev->write_a32d16(dev, addr+0x601a, p[3]);
        if (res!=2) {
            complain("mtdc32_start_simple: set multievent: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* start acq */
        res=dev->write_a32d16(dev, addr+0x603a, 1);
        if (res!=2) {
            complain("mtdc32_start_simple: start acq: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* readout reset */
        res=dev->write_a32d16(dev, addr+0x6034, ANY);
        if (res!=2) {
            complain("mtdc32_start_simple: reset FIFO: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
    }

    return pres;
}

plerrcode test_proc_mtdc32_start_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=3) {
        complain("mtdc32_start_simple: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_start_simple: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_start_simple: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_start_simple[] = "mtdc32_start_simple";
int ver_proc_mtdc32_start_simple = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3
 * p[1]: mcst_module
 * p[2]: multievent (0: single, 3: multi (2 not usable))
 * p[3]: max_transfer_data
 */
plerrcode
proc_mtdc32_start_cblt(ems_u32* p)
{
    ml_entry* mcst_module=ModulEnt(p[1]);
    struct vme_dev* dev=mcst_module->address.vme.dev;
    ems_u32 maddr=mcst_module->address.vme.addr;
    int res;

    mtdc32_clear_statist(0);
    for_each_module(mesytec_mtdc32, mtdc32_clear_statist);

    /* stop acq (should not be necessary) */
    res=dev->write_a32d16(dev, maddr+0x603a, 0);
    if (res!=2) {
        complain("mtdc32_start_cblt: stop acq: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /* set multievent */
    res=dev->write_a32d16(dev, maddr+0x6036, p[2]);
    if (res!=2) {
        complain("mtdc32_start_cblt: set multievent: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /*
     * set max_transfer_data (956 is maximum)
     * 0 (infinite) is also possible, but dangerous (transfer may never end)
     */
    res=dev->write_a32d16(dev, maddr+0x601a, p[3]);
    if (res!=2) {
        complain("mtdc32_start_cblt: set max_transfer_data: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /* start acq */
    res=dev->write_a32d16(dev, maddr+0x603a, 1);
    if (res!=2) {
        complain("mtdc32_start_cblt: start acq: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    /* readout reset */
    res=dev->write_a32d16(dev, maddr+0x6034, ANY);
    if (res!=2) {
        complain("mtdc32_start_cblt: reset FIFO: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    return plOK;
}

plerrcode test_proc_mtdc32_start_cblt(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=3) {
        complain("mtdc32_start_cblt: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme)) {
        complain("mtdc32_start_cblt: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }

    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("mtdc32_start_cblt: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_start_cblt[] = "mtdc32_start_cblt";
int ver_proc_mtdc32_start_cblt = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==2
 * p[1]: mcst_module
 */
plerrcode
proc_mtdc32_stop_cblt(ems_u32* p)
{
    ml_entry* mcst_module=ModulEnt(p[1]);
    struct vme_dev* dev=mcst_module->address.vme.dev;
    ems_u32 maddr=mcst_module->address.vme.addr;
    int res;

    /* stop acq */
    res=dev->write_a32d16(dev, maddr+0x603a, 0);
    if (res!=2) {
        complain("mtdc32_stop_cblt: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    return plOK;
}

plerrcode test_proc_mtdc32_stop_cblt(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=1) {
        complain("mtdc32_stop_cblt: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme)) {
        complain("mtdc32_stop_cblt: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }

    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("mtdc32_stop_cblt: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_stop_cblt[] = "mtdc32_stop_cblt";
int ver_proc_mtdc32_stop_cblt = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: module idx
 */
plerrcode
proc_mtdc32_stop_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_stop_simple);
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

plerrcode test_proc_mtdc32_stop_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=1) {
        complain("stop_simple: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_stop_simple: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_stop_simple: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=0;
    return plOK;
}

char name_proc_mtdc32_stop_simple[] = "mtdc32_stop_simple";
int ver_proc_mtdc32_stop_simple = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: module idx
 * p[2]: max_words (per module)
 */
plerrcode
proc_mtdc32_read_simple(ems_u32* p)
{
    ems_u32 *oldoutptr=outptr;
    ems_i32 *ip=(ems_i32*)p;
    plerrcode pres;

    if (ip[1]<0) {
        pres=for_each_member(p, mesytec_mtdc32, proc_mtdc32_read_simple);
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
            complain("mtdc32_read_simple: read length: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
#endif

        /* read data */
        oldoutptr=outptr;
        res=dev->read(dev, addr+0, 0xb, outptr, /*4*val*/4*p[2], 4, &outptr);
        if (res<0) {
            complain("mtdc32_read_simple: read: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }

        /* clear busy */
        res=dev->write_a32d16(dev, addr+0x6034, 0);
        if (res!=2) {
            complain("mtdc32_read_simple: clear busy: res=%d errno=%s",
                    res, strerror(errno));
            return plErr_System;
        }
        mtdc32_scan_events(oldoutptr, outptr-oldoutptr);
    }

    return pres;
}

plerrcode test_proc_mtdc32_read_simple(ems_u32* p)
{
    ems_i32 *ip=(ems_i32*)p;
    ml_entry* module;

    if (p[0]!=2) {
        complain("mtdc32_read_simple: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (ip[1]>=0) {
        if (!valid_module(p[1], modul_vme)) {
            complain("mtdc32_read_simple: p[1](==%u): no valid VME module", p[1]);
            return plErr_ArgRange;
        }
        module=ModulEnt(p[1]);
        if (module->modultype!=mesytec_mtdc32) {
            complain("mtdc32_read_simple: p[1](==%u): no mesytec mtdc32", p[1]);
            return plErr_BadModTyp;
        }
    }

    wirbrauchen=(ip[1]>=0?1:nr_modules(mesytec_mtdc32))*p[2];
    return plOK;
}

char name_proc_mtdc32_read_simple[] = "mtdc32_read_simple";
int ver_proc_mtdc32_read_simple = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: mcst_module
 * p[2]: max_words (per module)
 */
plerrcode
proc_mtdc32_read_mcst(ems_u32* p)
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
                    module->modultype==mesytec_mtdc32) {
                dev=module->address.vme.dev;
                addr=module->address.vme.addr;
                res=dev->read(dev, addr, 0xb, outptr, 4*p[2], 4, &outptr);
                if (res<0) {
                    complain("mtdc32_read_mcst: %s", strerror(errno));
                    return plErr_System;
                }
            }
        }
    } else {          /* iterate over modullist */
        for (i=0; i<modullist->modnum; i++) {
            module=&modullist->entry[i];
            if (module->modulclass==modul_vme &&
                    module->modultype==mesytec_mtdc32) {
                dev=module->address.vme.dev;
                addr=module->address.vme.addr;
                res=dev->read(dev, addr, 0xb, outptr, 4*p[2], 4, &outptr);
                if (res<0) {
                    complain("mtdc32_read_mcst: %s", strerror(errno));
                    return plErr_System;
                }
            }
        }
    }

    /* clear busy */
    res=mdev->write_a32d16(dev, maddr+0x6034, 0);
    if (res!=2) {
        complain("mtdc32_read_mcst: clear busy: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    mtdc32_scan_events(oldoutptr, outptr-oldoutptr);

    return plOK;
}

plerrcode test_proc_mtdc32_read_mcst(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=2) {
        complain("mtdc32_read_mcst: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme)) {
        complain("mtdc32_read_mcst: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }
    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("mtdc32_read_mcst: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    wirbrauchen=nr_modules(mesytec_mtdc32)*p[2];
    return plOK;
}

char name_proc_mtdc32_read_mcst[] = "mtdc32_read_mcst";
int ver_proc_mtdc32_read_mcst = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==3
 * p[1]: mcst_module
 * p[2]: cblt_module
 * p[3]: max words (should be equal to max_transfer_data in
 *       mtdc32_start_cblt times number of modules)
 */

plerrcode
proc_mtdc32_read_cblt(ems_u32* p)
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
        complain("mtdc32_read_cblt: clear busy: res=%d errno=%s",
                res, strerror(errno));
        return plErr_System;
    }

    mtdc32_scan_events(oldoutptr, outptr-oldoutptr);

    return plOK;
}

plerrcode test_proc_mtdc32_read_cblt(ems_u32* p)
{
    ml_entry* module;

    if (p[0]!=3) {
        complain("mtdc32_read_cblt: illegal # of arguments: %d", p[0]);
        return plErr_ArgNum;
    }

    if (!valid_module(p[1], modul_vme)) {
        complain("mtdc32_read_cblt: p[1](==%u): no valid VME module", p[1]);
        return plErr_ArgRange;
    }
    module=ModulEnt(p[1]);
    if (module->modultype!=vme_mcst) {
        complain("mtdc32_read_cblt: p[1](==%u): no mcst module", p[1]);
        return plErr_BadModTyp;
    }

    if (!valid_module(p[2], modul_vme)) {
        complain("mtdc32_read_cblt: p[2](==%u): no valid VME module", p[2]);
        return plErr_ArgRange;
    }
    module=ModulEnt(p[2]);
    if (module->modultype!=vme_cblt) {
        complain("mtdc32_read_cblt: p[2](==%u): no cblt module", p[2]);
        return plErr_BadModTyp;
    }

    wirbrauchen=p[3];
    return plOK;
}

char name_proc_mtdc32_read_cblt[] = "mtdc32_read_cblt";
int ver_proc_mtdc32_read_cblt = 1;
/*****************************************************************************/




