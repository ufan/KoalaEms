/*
 * procs/unixvme/caen/v792.c
 * created 30.Sep.2002 p.kulessa
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: v785_792.c,v 1.10 2017/10/09 21:09:24 wuestner Exp $";

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
extern int *memberlist;

RCS_REGISTER(cvsid, "procs/unixvme/caen")

/*
 * V775 32-channel MULTIEVENT TDC
 * V785 32-channel PEAK SENSING ADC
 * V792 32-channel QDC
 * A32D16 (registers) A32D32/BLT32 (data buffer)
 * reserves 65536 Byte
 */
/*
 * 0x00..0x7FC: output buffer
 * 0x1000: firmware revision
 * 0x1002: geo address
 * 0x1004: MCST/CBLT address
 * 0x1006: bit set 1
 * 0x1008: bit clear 1
 * 0x100A: interrupt level
 * 0x100C: interrupt vector
 * 0x100E: status register 1
 * 0x1010: control register 1
 * 0x1012: ADER high
 * 0x1014: ADER low
 * 0x1016: single shot reset
 * 0x101A: MCST/CBLT ctrl
 * 0x1020: event trigger register
 * 0x1022: status register 2
 * 0x1024: event counter_l
 * 0x1026: event counter_h
 * 0x1028: increment event
 * 0x102A: increment offset
 * 0x102C: load test register
 * 0x102E: fast clear window
 * 0x1032: bit set 2
 * 0x1034: bit clear 2
 * 0x1036: memory write test address
 * 0x1038: memory test word high
 * 0x103A: memory test word low
 * 0x103C: crate select
 * 0x103E: test write event
 * 0x1040: event counter reset
 * 0x1060: pedestal current (common for all channels)
 * 0x1064: memory test read address
 * 0x1068: sw comm
 * 0x106A: slide constant
 * 0x1070: AAD
 * 0x1072: BAD
 * 0x1080..0x10BF: thresholds
 * 0x8000..ffff: ROM 
 */

#undef USE_ECOUNTS

#ifdef USE_ECOUNTS /* wozu zum Teufel soll das gut sein? */
struct v792_private {
    ems_i32 event;
};

#define ecount(m) (((struct v792_private*)(m)->private_data)->event)
#endif

static ems_u32 mtypes[]={CAEN_V775, CAEN_V785, CAEN_V792, 0};

/*****************************************************************************/
#ifdef USE_ECOUNTS
static int
read_ecount(struct vme_dev* dev, ems_u32 addr, ems_u32* ecount)
{
    ems_u32 count;
    ems_u16 d;

    dev->read_a32d16(dev, addr+0x1026, &d);
    count=d&0xff; count<<=16;
    dev->read_a32d16(dev, addr+0x1024, &d);
    count|=d;
    *ecount=count;
    return 0;
}

static int
read_ecounts(ems_u32 ecounts[32])
{
#if 1
    struct vme_dev* dev;
    ems_u32 addrs[64];
    ems_u32 counts[64];
    int i, j, res;

    dev=ModulEnt(1)->address.vme.dev;
    for (i=1, j=0; i<=memberlist[0]; i++) {
        ems_u32 addr=ModulEnt(i)->address.vme.addr;
        addrs[j++]=addr+0x1024;
        addrs[j++]=addr+0x1026;
    }
    res=dev->read_pipe(dev, 0x09, memberlist[0]*2, 2, addrs, counts);
    if (res!=memberlist[0]*2) {
        printf("read ecounts failed\n");
        return 0;
    }
    for (i=1, j=0; i<=memberlist[0]; i++, j+=2) {
        ecounts[i]=(counts[j]&0xffff)|((counts[j+1]&0xff)<<16);
    }
#else
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        read_ecount(dev, addr, ecounts+i-1);
    }
#endif
    return 0;
}
#endif

static plerrcode
v792flush(void)
{
#ifdef USE_ECOUNTS
    ems_u32 eventcounts[32];
#endif
    int i, res;
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

        res=dev->read_a32_fifo(dev, addr, outptr, 0x800*4, 4, 0);
        printf("v792flush(0x%08x, evt %d): res=%d\n", addr, trigger.eventcnt, res);
#ifdef USE_ECOUNTS
        ecount(module)=read_ecount(dev, addr, eventcounts);
#endif
    }
    return plOK;
}
/*****************************************************************************/
/*
 * p[0]: argcount==0/1
 * [p[1]: index of module; -1: all modules of intrumentation system]
 */
static plerrcode
v792reset_module(int idx)
{
    ml_entry* module=ModulEnt(idx);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    ems_u16 stat;
    int res, i;

    /* reset module */
    /* res=dev->write_a32d16(dev, addr+0x1016, 0); should not be used */
    res=dev->write_a32d16(dev, addr+0x1006, 0x80);
    if (res!=2) return plErr_System;
    res=dev->write_a32d16(dev, addr+0x1008, 0x80);
    if (res!=2) return plErr_System;

    /* check amnesia and write GEO address if necessary*/
printf("read addr 0x%x offs 0x100e\n", addr);
    res=dev->read_a32d16(dev, addr+0x100e, &stat);
    if (res!=2) return plErr_System;
    if (stat&0x10) {
        res=dev->write_a32d16(dev, addr+0x1002, idx);
        if (res!=2) return plErr_System;
        /* reset module again to use this address */
        res=dev->write_a32d16(dev, addr+0x1006, 0x80);
        if (res!=2) return plErr_System;
        res=dev->write_a32d16(dev, addr+0x1008, 0x80);
        if (res!=2) return plErr_System;
    }

    /* clear threshold memory */
    for (i=0; i<32; i++) {
        res=dev->write_a32d16(dev, addr+0x1080+2*i, 0);
        if (res!=2) return plErr_System;
    }

    return plOK;
}

plerrcode proc_v792reset(ems_u32* p)
{
    int i, pres=plOK;

    if (p[0] && (p[1]>-1)) {
        pres=v792reset_module(p[1]);
    } else {
        for (i=memberlist[0]; i>0; i--) {
            pres=v792reset_module(i);
            if (pres!=plOK)
                break;
        }
    }
    return pres;
}

plerrcode test_proc_v792reset(ems_u32* p)
{
    plerrcode res;

    if ((p[0]!=0) && (p[0]!=1))
        return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792reset[] = "v792reset";
int ver_proc_v792reset = 1;
/*****************************************************************************/
/*
 * p[0]: argcount=3+memberlist[0]
 * p[1]: CBLT addr
 * p[2]: sparse
 * p[3]: step_th
 * p[4...]: iped
 */
static plerrcode
v792init_module(int idx, ems_u32 cblt, int cblt_ctrl, int sparse,
    int step_th, int iped)
{
    ml_entry* module=ModulEnt(idx);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr;
    ems_u32 val;
    plerrcode pres;

printf("v792init T1\n");
    pres=v792reset_module(idx);
    if (pres!=plOK)
        return pres;

printf("v792init T2\n");
    /* write CBLT address */
    if (dev->write_a32d16(dev, addr+0x1004, cblt)!=2)
        return plErr_System;

printf("v792init T3\n");
    /* set control register 1 (enable bus error) */
    if (dev->write_a32d16(dev, addr+0x1010, 0x20)!=2)
        return plErr_System;

printf("v792init T4\n");
    /* set cblt control */
    if (dev->write_a32d16(dev, addr+0x101a, cblt_ctrl)!=2)
        return plErr_System;

printf("v792init T5\n");
    /* set bit register 2 (enable headers for empty events */
    val=0;
    val|=0x8;       /* disable overrange suppression */
    if (!sparse)
        val|=0x10;  /* disable zero suppression */
    if (step_th)
        val|=0x100; /* change threshold resolution */
    val|=0x800;     /* enable auto increment */
    val|=0x1000;    /* enable empty events */
    val|=0x4000;    /* increment event counter for all events */
    if (dev->write_a32d16(dev, addr+0x1034, 0xffff)!=2)
        return plErr_System;
    if (dev->write_a32d16(dev, addr+0x1032, val)!=2)
        return plErr_System;

printf("v792init T6\n");
    /* programm Iped register */
    if (iped>=0) {
        if (dev->write_a32d16(dev, addr+0x1060, iped)!=2)
            return plErr_System;
    }

printf("v792init T7\n");
    /* set sliding scale DAQ */
    if (dev->write_a32d16(dev, addr+0x106a, 0)!=2)
        return plErr_System;

    return plOK;
}

plerrcode proc_v792init(ems_u32* p)
{
    plerrcode pres=plOK;
    int i;

    for (i=1; i<=memberlist[0]; i++) {
#if 0
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        int res;
#endif

        /* wrong: memberlist[...] has nothing to do with position in crate */
        /* we could use the geographical address here */
        int cblt_ctrl=3;
        if (i==1) {
            cblt_ctrl=2;
        } else if (i==memberlist[0]) {
            cblt_ctrl=1;
        }
        pres=v792init_module(i, p[1], cblt_ctrl, p[2], p[3], p[3+i]);
        if (pres!=plOK)
            break;
    }
    return pres;
}

plerrcode test_proc_v792init(ems_u32* p)
{
    plerrcode res;

    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;
printf("v792init: after test\n");
    if (p[0]!=memberlist[0]+3)
        return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792init[] = "v792init";
int ver_proc_v792init = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_v792start(ems_u32* p)
{
    int i, res;

    for (i=memberlist[0]; i>0; i--) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;

#if 0
        /* clear event counter */
        res=dev->write_a32d16(dev, addr+0x1040, 0);
        if (res!=2) return plErr_System;
#else
        /* clear data and event counter */
        res=dev->write_a32d16(dev, addr+0x1032, 4);
        if (res!=2) return plErr_System;
        res=dev->write_a32d16(dev, addr+0x1034, 4);
        if (res!=2) return plErr_System;
#endif
    }
    return plOK;
}

plerrcode test_proc_v792start(ems_u32* p)
{
    plerrcode res;

    if (p[0])
        return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792start[] = "v792start";
int ver_proc_v792start = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_v792convert(ems_u32* p)
{
    int i, res;

    for (i=memberlist[0]; i>0; i--) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;

        /* write conversion pulse */
        res=dev->write_a32d16(dev, module->address.vme.addr+0x1068, 0);
        if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode test_proc_v792convert(ems_u32* p)
{
    plerrcode res;

    if (p[0])
        return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792convert[] = "v792convert";
int ver_proc_v792convert = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: bit set register 2
 */
/*
 *    0: mem test
 *    2: offline
 *    4: clear data
 *    8: over range prog
 *   10: low threshold prog
 *   40: test acq
 *   80: slide enable
 *  100: step threshold
 *  800: auto increment
 * 1000: empty prog
 * 2000: slide-sub enable
 * 4000: all trig
 */
plerrcode proc_v792bitset2(ems_u32* p)
{
    int i, res;

    for (i=memberlist[0]; i>0; i--) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;

        /* write bit set register 2 */
        res=dev->write_a32d16(dev, module->address.vme.addr+0x1032, p[1]);
        if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode test_proc_v792bitset2(ems_u32* p)
{
    plerrcode res;

    if ((unsigned)p[0]>1) return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792bitset2[] = "v792bitset2";
int ver_proc_v792bitset2 = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: bit res register 2
 */
plerrcode proc_v792bitres2(ems_u32* p)
{
    int i, res;

    for (i=memberlist[0]; i>0; i--) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;

        /* write bit res register 2 */
        res=dev->write_a32d16(dev, module->address.vme.addr+0x1034, p[1]);
        if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode test_proc_v792bitres2(ems_u32* p)
{
    plerrcode res;

    if ((unsigned)p[0]>1) return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792bitres2[] = "v792bitres2";
int ver_proc_v792bitres2 = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_v792read(ems_u32* p)
{
    int i, res;

    *outptr++=memberlist[0];
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data, *help;
        int code, count, j;

        help=outptr++;

        /* read header */
        res=dev->read_a32d32(dev, addr, &data);
        if (res!=4) return plErr_System;
        code=(data>>24)&7;
        if (code==6) { /* buffer empty */
            /*
            *help=0;
            return plOK;
            */
            printf("v792read: buffer empty\n");
            *outptr++=0;
            *help=1;
            return plErr_HW;
        }
        if (code!=2) { /* no header */
            printf("v792read: not a header: 0x%08x\n", data);
            *outptr++=1;
            *outptr++=data;
            *help=2;
            return plErr_HW;
        }
        *outptr++=data;
        count=(data>>8)&0x3f;

        /* read data */
        for (j=0; j<count; j++) {
            res=dev->read_a32d32(dev, addr, &data);
            if (res!=4) return plErr_System;
            if (data&0x3c00000) {
                printf("v792read: not a data word: 0x%08x\n", data);
                *outptr++=2;
                *outptr++=data;
                *help=outptr-help-1;
                return plErr_HW;
            }
            *outptr++=data;
        }

        /* read EOB */
        res=dev->read_a32d32(dev, addr, &data);
        if (res!=4) return plErr_System;
        code=(data>>24)&7;
        if (code!=4) { /* no EOB */
            printf("v792read: not a EOB: 0x%08x\n", data);
            *outptr++=3;
            *outptr++=data;
            *help=outptr-help-1;
            return plErr_HW;
        }
        *outptr++=data;

        /* read invalid last word */
        res=dev->read_a32d32(dev, addr, &data);
        if (res!=4) return plErr_System;
        code=(data>>24)&7;
        if (code!=0x6) { /* no EOB */
            printf("v792read: not a invalid word: 0x%08x\n", data);
            *outptr++=4;
            *outptr++=data;
            *help=outptr-help-1;
            return plErr_HW;
        }
        *help=outptr-help-1;
    }

    return plOK;
}

plerrcode test_proc_v792read(ems_u32* p)
{
    plerrcode res;

    if (p[0]) return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;

    wirbrauchen=memberlist[0]*34*32;

    return plOK;
}

char name_proc_v792read[] = "v792read";
int ver_proc_v792read = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_v792read_all(ems_u32* p)
{
    int i, res;

    *outptr++=memberlist[0];
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data, *help;

        help=outptr++;

        /* read until code==6 (invalid word) */
        while (1) {
            res=dev->read_a32d32(dev, addr, &data);
            if (res!=4) {
                *help=outptr-help-1;
                return plErr_System;
            }
            if ((data&0x7000000)!=0x6000000) { /* valid word */
                *outptr++=data;
            } else {
                break;
            }
        }
        *help=outptr-help-1;

        /* check for complete event */
        if (*help<2) {
            printf("v792read_all: not enough data\n");
        } else {
            if ((help[1]&0x7000000)!=0x2000000)
                printf("v792read_all: first word is not a header\n");
            if ((outptr[-1]&0x7000000)!=0x4000000)
                printf("v792read_all: last word is not a trailer\n");
        }
    }
    return plOK;
}

plerrcode test_proc_v792read_all(ems_u32* p)
{
    plerrcode res;

    if (p[0])
        return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;

    wirbrauchen=memberlist[0]*34*32;

    return plOK;
}

char name_proc_v792read_all[] = "v792read_all";
int ver_proc_v792read_all = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_v792read_one(ems_u32* p)
{
    int i, res;

    *outptr++=memberlist[0];
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data, *help;

        help=outptr++;

        /* read until code==4 (trailer) */
        do {
            res=dev->read_a32d32(dev, addr, &data);
            if (res!=4) {
                *help=outptr-help-1;
                return plErr_System;
            }
            *outptr++=data;
        } while ((data&0x7000000)!=0x4000000);
        *help=outptr-help-1;

        /* read until code==6 (invalid word) and discard data */
        do {
            res=dev->read_a32d32(dev, addr, &data);
            if (res!=4) {
                return plErr_System;
            }
        } while ((data&0x7000000)!=0x6000000);

    }
    return plOK;
}

plerrcode test_proc_v792read_one(ems_u32* p)
{
    plerrcode res;

    if (p[0])
        return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;

    wirbrauchen=memberlist[0]*34*32;

    return plOK;
}

char name_proc_v792read_one[] = "v792read_one";
int ver_proc_v792read_one = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==1
 * p[1]: 0: simple loop 1: read_fifo
 */
plerrcode proc_v792flush(ems_u32* p)
{
    int i, res;

    if (p[1]) {
        return v792flush();
    }

    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data;

        /* read until code==6 (invalid word) */
        do {
            res=dev->read_a32d32(dev, addr, &data);
            if (res!=4) {
                return plErr_System;
            }
        } while ((data&0x7000000)!=0x6000000);
    }
    return plOK;
}

plerrcode test_proc_v792flush(ems_u32* p)
{
    plerrcode res;

    if (p[0]!=1)
        return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;

    wirbrauchen=0;

    return plOK;
}

char name_proc_v792flush[] = "v792flush";
int ver_proc_v792flush = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==2
 * p[1]: ecounts reader: 0:single 1: pipe
 */
plerrcode proc_v792readblock(ems_u32* p)
{
    int i, res;
    ems_u32* outp=outptr;

    *outptr++=memberlist[0];

#ifdef USE_ECOUNTS
    { /* event number check */
        ems_u32 ecounts[32];

        read_ecounts(ecounts);
        for (i=1; i<=memberlist[0]; i++) {
            ml_entry* module=ModulEnt(i);
            struct vme_dev* dev=module->address.vme.dev;
            ems_u32 addr=module->address.vme.addr;
            ems_u32 ecount;

            ecount(module)++;
            read_ecount(dev, addr, &ecount);
            if (ecount!=ecount(module)) {
                printf("v792readblock: eventcount %d --> %d; module %d "
                        "addr 0x%x; evt=%d\n",
                        ecount(module), ecount, i, addr, trigger.eventcnt);
                send_unsol_warning(11, 3, trigger.eventcnt, addr,
                        ecount-ecount(module));
                /*goto error_hw;*/
            }
        }
    }    
#endif


    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr;
        ems_u32 data, *help;
        int code, count;

        /*perfspect_record_time("proc_v792readblock");*/

        help=outptr++;

        /* read header */
        res=dev->read_a32d32(dev, addr+0, &data);
        if (res!=4) {
            printf("v792readblock: cannot read header\n");
            return plErr_System;
        }
        code=(data>>24)&7;
        if (code==6) { /* buffer empty */
            /*
            *help=0;
            return plOK;
            */
            printf("v792read(0x%08x): buffer empty\n", addr);
            *outptr++=0;
            *help=1;
            goto error_hw;
        }
        if (code!=2) { /* no header */
            printf("v792read(0x%08x): not a header: 0x%08x\n", addr, data);
            *outptr++=1;
            *outptr++=data;
            *help=2;
            goto error_hw;
        }
        *outptr++=data;
        count=(data>>8)&0x3f;

        /* read data, EOB and invalid last word *//* now: all data */
      /*res=dev->read_a32_fifo(dev, addr, outptr, (count+2)*4, 4, 0);*/
        res=dev->read_a32_fifo(dev, addr, outptr,     0x800*4, 4, 0);
      /*if (res!=(count+2)*4) {*/
        if (res<(count+2)*4) {
            printf("v792read(0x%08x): read_a32_fifo(%d): res=%d\n",
                    addr, (count+2)*4, res);
            goto error_hw;
        }
        if (res>(count+2)*4) {
            printf("v792read(0x%08x): count=%d (not %d)\n", addr, res/4, count+2);
            goto error_hw;
        }
        outptr+=count+1;
        code=(outptr[-1]>>24)&7;
        if (code!=4) { /* no EOB */
            printf("v792read(0x%08x): not a EOB: 0x%08x\n", addr, data);
            *outptr++=3;
            *outptr++=data;
            *help=outptr-help-1;
            goto error_hw;
        }
        code=(outptr[0]>>24)&7;
        if (code!=0x6) { /* no EOB */
            printf("v792read(0x%08x): not a invalid word: 0x%08x\n", addr, data);
            *outptr++=4;
            *outptr++=data;
            *help=outptr-help-1;
            goto error_hw;
        }
        *help=outptr-help-1;
    }

    return plOK;

error_hw:
    outptr=outp;
    return v792flush();
}

plerrcode test_proc_v792readblock(ems_u32* p)
{
    plerrcode res;

    if (p[0]!=1)
        return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK)
        return res;
    if ((memberlist[0]>32) || (memberlist[0]<1))
        return plErr_BadISModList;

#ifdef USE_ECOUNTS
    {
        int i;
        for (i=1; i<=memberlist[0]; i++) {
            ml_entry* module=ModulEnt(i);
            if (!module->private_data) {
                module->private_data=(ems_u32*)malloc(sizeof(struct v792_private));
                if (!module->private_data)
                    return plErr_NoMem; /* XXX memory leak */
            }
            ecount(module)=-1;
        }
    }
#endif    

    wirbrauchen=memberlist[0]*34*32;

#if PERFSPECT
    perfbedarf=memberlist[0];
    perfnames[0]="Module 1";
    perfnames[1]="Module 2";
    perfnames[2]="Module 3";
    perfnames[3]="Module 4";
    perfnames[4]="Module 5";
    perfnames[5]="Module 6";
    perfnames[6]="Module 7";
    perfnames[7]="Module 8";
#endif

    return plOK;
}

char name_proc_v792readblock[] = "v792readblock";
int ver_proc_v792readblock = 2;
/*****************************************************************************/
/*
 * p[0]: argcount==memberlist[0]*32
 * p[1..32]: thresholds for module 1
 * p[n*32+1..(n+1)*32]: thresholds for module n
 * ...
 */
plerrcode proc_v792writethreshold(ems_u32* p)
{
    int i, res;

    p++;
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr+0x1080;
        int j;

        /* write threshold values */
        for (j=32; j; j--, addr+=2) {
            res=dev->write_a32d16(dev, addr, *p++);
            if (res!=2) return plErr_System;
        }
    }
    return plOK;
}

plerrcode test_proc_v792writethreshold(ems_u32* p)
{
    plerrcode res;

    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;
    if (p[0]!=memberlist[0]*32) return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792writethreshold[] = "v792writethreshold";
int ver_proc_v792writethreshold = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==33
 * p[1]: module
 * p[2..33]: thresholds for module 1
 * ...
 */
plerrcode proc_v792writethreshold2(ems_u32* p)
{
    ml_entry* module=ModulEnt(p[1]);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr+0x1080;
    int j;

    /* write threshold values */
    p+=2;
    for (j=32; j; j--, addr+=2) {
        if (dev->write_a32d16(dev, addr, *p++)!=2)
            return plErr_System;
    }

    return plOK;
}

plerrcode test_proc_v792writethreshold2(ems_u32* p)
{
    ml_entry* module;
    plerrcode pres;

    if (p[0]!=33)
        return plErr_ArgNum;

    if (!valid_module(p[1], modul_vme))
        return plErr_ArgRange;
    module=ModulEnt(p[1]);
    if ((pres=test_proc_vmemodule(module, mtypes))!=plOK)
        return pres;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792writethreshold2[] = "v792writethreshold";
int ver_proc_v792writethreshold2 = 2;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 * ...
 */
plerrcode proc_v792readthreshold(ems_u32* p)
{
    int i, res;

    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr+0x1080;
        int j;
        ems_u16 data;

        /* write threshold values */
        for (j=32; j; j--, addr+=2) {
            res=dev->read_a32d16(dev, addr, &data);
            if (res!=2) return plErr_System;
 
           *outptr++=data;

        }
    }
    return plOK;
}

plerrcode test_proc_v792readthreshold(ems_u32* p)
{
    plerrcode res;

    if (p[0]) return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;

    wirbrauchen=memberlist[0]*32;
    return plOK;

}

char name_proc_v792readthreshold[] = "v792readthreshold";
int ver_proc_v792readthreshold = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==memberlist[0]
 * p[1,..,n]: common pedestal for module n
 * ...
 */
plerrcode proc_v792writepedestal(ems_u32* p)
{
    int i, res;

    p++;
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr+0x1060;

        /* write common pedestal  */
            res=dev->write_a32d16(dev, addr, *p++);
            if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode test_proc_v792writepedestal(ems_u32* p)
{
    plerrcode res;

    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;
    if (p[0]!=memberlist[0]) return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792writepedestal[] = "v792writepedestal";
int ver_proc_v792writepedestal = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 * ...
 */
plerrcode proc_v792readpedestal(ems_u32* p)
{
    int i, res;

    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr+0x1060;
        ems_u16 data;

        /* read common pedestal  */
            res=dev->read_a32d16(dev, addr, &data);
            if (res!=2) return plErr_System;
          *outptr++=data;
    }
    return plOK;
}

plerrcode test_proc_v792readpedestal(ems_u32* p)
{
    plerrcode res;

    if (p[0]) return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;

    wirbrauchen=memberlist[0];
    return plOK;
}

char name_proc_v792readpedestal[] = "v792readpedestal";
int ver_proc_v792readpedestal = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==memberlist[0]
 * p[1,..,n]: slide const  for module n
 * ...
 */
plerrcode proc_v792writeslide(ems_u32* p)
{
    int i, res;

    p++;
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr+0x106A;

        /* write slide values */
            res=dev->write_a32d16(dev, addr, *p++);
            if (res!=2) return plErr_System;
    }
    return plOK;
}

plerrcode test_proc_v792writeslide(ems_u32* p)
{
    plerrcode res;

    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;
    if (p[0]!=memberlist[0]) return plErr_ArgNum;

    wirbrauchen=0;
    return plOK;
}

char name_proc_v792writeslide[] = "v792writeslide";
int ver_proc_v792writeslide = 1;
/*****************************************************************************/
/*
 * p[0]: argcount==0
 */
plerrcode proc_v792readslide(ems_u32* p)
{
    int i, res;

    p++;
    for (i=1; i<=memberlist[0]; i++) {
        ml_entry* module=ModulEnt(i);
        struct vme_dev* dev=module->address.vme.dev;
        ems_u32 addr=module->address.vme.addr+0x106A;
        ems_u16 data;

        /*read  slide values */
            res=dev->read_a32d16(dev, addr, &data);
            if (res!=2) return plErr_System;

          *outptr++=data;

    }
    return plOK;
}

plerrcode test_proc_v792readslide(ems_u32* p)
{
    plerrcode res;

    if (p[0]) return plErr_ArgNum;
    if ((res=test_proc_vme(memberlist, mtypes))!=plOK) return res;

    wirbrauchen=memberlist[0];
    return plOK;
}

char name_proc_v792readslide[] = "v792readslide";
int ver_proc_v792readslide = 1;
/*****************************************************************************/
/*
 * read ALL data (from all modules and all events)
 * p[0]: number of args == 1
 * p[1]: cblt_addr
 */
plerrcode proc_v792cblt(ems_u32* p)
{
    ml_entry* module=ModulEnt(1);
    struct vme_dev* dev=module->address.vme.dev;
    ems_u32 addr=module->address.vme.addr+0x100E;
    ems_u32* help=outptr++;
    ems_u32 status, num=0;
    int loop=0, res;
    ems_u32 v=0;

    if (p[0]>1) var_read_int(p[2], &v);

    if (v>0) {
        var_write_int(p[2], v-1);
        return plOK;
    }
    v=!!v;

do {
    do {
        /* read data */
        res=dev->read(dev, p[1], 0x0b, outptr, memberlist[0]*32*34*4, 4,
                &outptr);
        /*res=dev->read_a32_fifo(dev, p[1], outptr, memberlist[0]*32*34*4, 4, &outptr);*/
        if (res>=0) {
            num+=res/4;
            if (!res) {
                printf("v792cblt: no Data, evt=%d loop=%d\n", trigger.eventcnt, loop);
            }
        } else {
            *help=-res;
            printf("v792cblt: error reading data: %s\n", strerror(-res));
            return plErr_System;
        }

        /* check global dready */
        res=dev->read(dev, addr, 0x2f, &status, 2, 2, 0);
        if (res<0) {
            *help=-res;
            printf("v792cblt: error reading status: %s\n", strerror(-res));
            return plErr_System;
        }
        loop++;
    } while ((loop<128) && (status&2));
    if (loop>1) {
        printf("v792cblt: multiple buffered events found (%d); evt=%d\n",
                loop, trigger.eventcnt);
        send_unsol_warning(11, 3, trigger.eventcnt, p[1], loop);
        if (loop>=128) {
            printf("v792cblt: DREADY still active after 32 events\n");
            return plErr_HW;
        }
    }
} while (v--);

    if (p[0]>1) var_write_int(p[2], 0);

    *help=num;
    return plOK;
}
plerrcode test_proc_v792cblt(ems_u32* p)
{
    /*plerrcode pres;*/

    if (p[0]<1)
        return plErr_ArgNum;
    if (!memberlist)
        return plErr_NoISModulList;
    if (!memberlist[0])
        return plErr_BadISModList;

    if (p[0]>1) {
        unsigned int v=p[2];
        unsigned int size;
        plerrcode pres;
        if ((var_attrib(v, &size)!=plOK) || (size!=1)) {
            var_delete(v);
            if ((pres=var_create(v, 1))!=plOK) return pres;
            var_write_int(v, 0);
        }
    }

    /*if ((pres=test_proc_vme(memberlist, mtypes))!=plOK) return pres;*/

    wirbrauchen=memberlist[0]*32*34+1;
    return plOK;
}
char name_proc_v792cblt[] = "v792cblt";
int ver_proc_v792cblt = 1;
/*****************************************************************************/
/*****************************************************************************/
