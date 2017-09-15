/*
 * lowlevel/jtag_low.c
 * created 12.Aug.2005 PW
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL: jtag_lowinit.c,v 1.2 2011/04/06 20:30:24 wuestner Exp $";

#include <sconf.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <rcs_ids.h>

/*#include "jtag_low.h"*/
#include "jtag_int.h"

#define __CONCAT(x,y)	x ## y
#define __STRING(x)	#x
#define __SS(s) __STRING(s)
#define JTAG_DEV(vendor, name, ir_len, mem_size, write_size) { \
    __CONCAT(VID_, vendor),                                    \
    __CONCAT(DID_, name),                                      \
    ir_len,                                                    \
    mem_size,                                                  \
    write_size,                                                \
    __SS(name)                                                 \
}
#define JTAG_DEV_END(name) {                                   \
    0, 0, -1, -1, -1, __SS(name)                               \
}

static const struct jtag_chipdata jtag_chipdata[]={
    JTAG_DEV(XILINX, XC18V256,      8,  0x8000, -1),
    JTAG_DEV(XILINX, XC18V512a,     8, 0x10000, -1),
    JTAG_DEV(XILINX, XC18V512b,     8, 0x10000, -1),
    JTAG_DEV(XILINX, XC18V01a,      8, 0x20000, 0x100),
    JTAG_DEV(XILINX, XC18V01b,      8, 0x20000, 0x100),
    JTAG_DEV(XILINX, XC18V02a,      8, 0x40000, 0x200),
    JTAG_DEV(XILINX, XC18V02b,      8, 0x40000, 0x200),
    JTAG_DEV(XILINX, XC18V04a,      8, 0x80000, 0x200),
    JTAG_DEV(XILINX, XC18V04b,      8, 0x80000, 0x200),
    JTAG_DEV(XILINX, XCF01S,        8, 0x20000, 0x100/*unknown*/),
    JTAG_DEV(XILINX, XCF02S,        8, 0x40000, 0x100/*unknown*/),
    JTAG_DEV(XILINX, XCF04S,        8, 0x80000, 0x100/*unknown*/),
    JTAG_DEV(XILINX, XCF08P,       16, 0x100000, 0x100/*unknown*/),
    JTAG_DEV(XILINX, XCF16P,       16, 0x200000, 0x100/*unknown*/),
    JTAG_DEV(XILINX, XCF32P,       16, 0x400000, 0x100/*unknown*/),
    JTAG_DEV(XILINX, XC2S100_FG256, 5, -1, -1),
    JTAG_DEV(XILINX, XC2S150,       5, -1, -1),/*XCV150*/
    JTAG_DEV(XILINX, XC2S200_BG256, 5, -1, -1),
    JTAG_DEV(XILINX, XCV300,        5, -1, -1),
    JTAG_DEV(XILINX, XCV400,        5, -1, -1),
    JTAG_DEV(XILINX, XC2V2000,      6, -1, -1),
    JTAG_DEV(XILINX, XC3S200,       6, -1, -1),
    JTAG_DEV(XILINX, XC3S400,       6, -1, -1),
    JTAG_DEV(XILINX, XC3S1000,      6, -1, -1),
    JTAG_DEV_END(unknown device),
};

static enum jtag_states jtag_transitions[][2]={
    /*jtag_TLR*/ {jtag_RTI, jtag_TLR},
    /*jtag_RTI*/ {jtag_RTI, jtag_SDS},
    /*jtag_SDS*/ {jtag_CD , jtag_SIS},
    /*jtag_CD */ {jtag_SD , jtag_E1D},
    /*jtag_SD */ {jtag_SD , jtag_E1D},
    /*jtag_E1D*/ {jtag_PD , jtag_UD},
    /*jtag_PD */ {jtag_PD , jtag_E2D},
    /*jtag_E2D*/ {jtag_SD , jtag_UD},
    /*jtag_UD */ {jtag_RTI, jtag_SDS},
    /*jtag_SIS*/ {jtag_CI , jtag_TLR},
    /*jtag_CI */ {jtag_SI , jtag_E1I},
    /*jtag_SI */ {jtag_SI , jtag_E1I},
    /*jtag_E1I*/ {jtag_PI , jtag_UI},
    /*jtag_PI */ {jtag_PI , jtag_E2I},
    /*jtag_E2I*/ {jtag_SI , jtag_UI},
    /*jtag_UI */ {jtag_RTI, jtag_SDS},
};

static const char* jtag_state_names[]={
    "test-logic-reset",
    "run-test/idle",
    "select-dr-scan",
    "capture-dr",
    "shift-dr",
    "exit1-dr",
    "pause-dr",
    "exit2-dr",
    "update-dr",
    "select-ir-scan",
    "capture-ir",
    "shift-ir",
    "exit1-ir",
    "pause-ir",
    "exit2-ir",
    "update-ir",
};

RCS_REGISTER(cvsid, "lowlevel/jtag")

/****************************************************************************/
#ifdef NIE
static void
printbits64(ems_u64 d, int skip, const char* as, const char* es)
{
    int i;

    printf("%s", as);
    for (i=0; i<64; i++, skip--) {
        int bit=!!(d&0x8000000000000000ULL);
        if (skip>0)
            printf(" ");
        else
            printf("%d", bit);
        d<<=1;
    }
    printf("%s", es);
}
#endif
/****************************************************************************/
#ifdef JTAG_TRACE
static void
printbits32(ems_u32 d, const char* as, const char* es)
{
    int i;

    printf("%s", as);
    for (i=0; i<32; i++) {
        int bit=!!(d&0x80000000UL);
        printf("%d", bit);
        if (i%4==3)
            printf(" ");
        d<<=1;
    }
    printf("%s", es);
}
#endif
/****************************************************************************/
static void
find_jtag_chipdata(struct jtag_chip* jtag_chip)
{
    const struct jtag_chipdata* d=jtag_chipdata;

/*printf("find_jtag_chipdata 0x%x\n", jtag_chip->id);*/

    jtag_chip->chipdata=0;
    while (d->vendor_id) {
        ems_u32 id=(d->part_id<<12)|(d->vendor_id<<1)|0x1;
        if ((jtag_chip->id&0x0fffffff)==id) {
            jtag_chip->chipdata=d;
            return;
        }
        d++;
    }
    printf("Vendor 0x%x Device 0x%x", (jtag_chip->id>>1)&0x7ff,
            (jtag_chip->id>>12)&0xffff);
}
/****************************************************************************/
static void
changestate(struct jtag_chain* chain, int tms)
{
    enum jtag_states oldstate;

    if ((tms<0) || (tms>1)) {
        printf("changestate: invalid ms: %d\n", tms);
        return;
    }
    oldstate=chain->jstate;
    chain->jstate=jtag_transitions[oldstate][tms];
}
/****************************************************************************/
static int
jtag_action(struct jtag_chain* chain, int tms, int tdi, int* tdo, int loop)
{
#ifdef JTAG_TRACE
    static ems_u32 ichain=0;
    static ems_u32 ochain=0;
    static char dchain[64];

#endif
    struct jtag_dev* jtag_dev=&chain->jtag_dev;
    enum jtag_states old_state;
    int tdo_, res;

    old_state=chain->jstate;
    res=chain->jtag_dev.jt_action(jtag_dev, tms, tdi, &tdo_);
    if (tdo)
        *tdo=tdo_;
    changestate(chain, tms);

    if (old_state==jtag_SI) {
        int bit=tdi, i;
        for (i=0; i<chain->num_chips; i++) {
            int nextbit=chain->chips[i].ir&1;
            chain->chips[i].ir>>=1;
            chain->chips[i].ir|=(bit<<(chain->chips[i].chipdata->ir_len-1));
            bit=nextbit;
        }
        if (chain->jstate!=jtag_SI) {
            int i;
            for (i=0; i<chain->num_chips; i++) {
                printf("IR[%d]: 0x%x\n", i, chain->chips[i].ir);
            }
        }
    }

    if (old_state==jtag_SD) {
        int i;
        for (i=63; i>0; i--)
            dchain[i]=dchain[i-1];
        dchain[0]=tdi?'1':'0';
#if 0
        if (chain->jstate!=jtag_SD) {
            printf("DR: ");
            for (i=0; i<64; i++) {
                printf("%c", dchain[i]);
            }
            printf("\n");
        }
#endif
    } else if (chain->jstate==jtag_SD) {
        int i;
        for (i=0; i<64; i++)
            dchain[i]='_';
    }

#ifdef JTAG_TRACE
    ichain>>=1;
    ochain>>=1;
    if (tdi)  ichain|=0x80000000UL;
    if (tdo_) ochain|=0x80000000UL;

    if (jtag_traced) {
        if (jtag_traced>1) {
            ems_u32 data;

            chain->jtag_dev.jt_data(jtag_dev, &data);

            printf("%16s tms %d tdi %d %16s tdo %d loop %d\n",
                jtag_state_names[old_state],
                tms, tdi,
                jtag_state_names[chain->jstate],
                tdo_,
                loop);

            printbits32(ichain, "ichain ", " "); printf("%08x\n", ichain);
            printbits32(ochain, "ochain ", " "); printf("%08x\n", ochain);
            printbits32(data  , "data   ", " "); printf("%08x\n", data);
        }

        if (chain->jstate!=old_state) {
            printf("(%2d) %16s --> %16s\n", chain->jstate_count,
                    jtag_state_names[old_state],
                    jtag_state_names[chain->jstate]);
            chain->jstate_count=1;
        } else {
            chain->jstate_count++;
        }
    }
#endif
    return res;
}
/****************************************************************************/
static int
jtag_data(struct jtag_chain* chain, ems_u32* data)
{
    int res;
    struct jtag_dev* jtag_dev=&chain->jtag_dev;
    res=chain->jtag_dev.jt_data(jtag_dev, data);
    return res;
}
/****************************************************************************/
static int
jtag_force_TLR(struct jtag_chain* chain)
{
    int i;

    /*printf("force_TLR\n");*/
    chain->jstate=jtag_TLR; /* not known yet, but forced by the following loop*/
    for (i=0; i<5; i++) {                                /* force TLR state */
        if (jtag_action(chain, 1, 0, 0, i)) return -1;
    }
    return 0;
}
/****************************************************************************/
#if 0
/*
 * state before: RTI or UI or SDS
 * state after:  RTI
 * reads 'bits' bits of data from 'chip'
 */
static int
jtag_rd_data32(struct jtag_chip* chip, int bits, ems_u32* data)
{
    struct jtag_chain* chain=chip->chain;
    int i;

    if (bits<1) {
        printf("jtag_wr_data32: bits=%d (illegal)\n", bits);
        return -1;
    }

    switch (chain->jstate) {
    case jtag_RTI:
    case jtag_UI:
        if (jtag_action(chain, 1, 0, 0, -1)) return -1; /* SDS */
        /* no break */
    case jtag_SDS:
        if (jtag_action(chain, 0, 0, 0, -1)) return -1; /* CD */
        break;
    default:
        printf("jtag_rd_data32: unexpected state %s\n",
                jtag_state_names[chain->jstate]);
        return -1;
    }

    for (i=chip->pre_d_bits; i>0; i--) {
        /* SD shift data from trailing chips away */
        if (jtag_action(chain, 0, 0, 0, i)) return -1;
    }
    for (i=bits-1; i>0; i--) {
        if (jtag_action(chain, 0, 0, 0, i)) return -1; /* SD shift data */
    }
    if (jtag_action(chain, 1, 0, 0, -1)) return -1;     /* E1D */
    if (jtag_data(chain, data)) return -1;
    (*data)>>=32-bits;

    if (jtag_action(chain, 1, 0, 0, -1)) return -1;     /* UD */
    if (jtag_action(chain, 0, 0, 0, -1)) return -1;     /* RTI */

    return 0;
}
#endif
/****************************************************************************/
#if 0
/*
 * state before: RTI
 * state after:  UI
 * fills 'chip' with 'icode', all other chips with 'BYPASS'
 */
static int
jtag_instruction(struct jtag_chip* chip, ems_u32 icode)
{
    struct jtag_chain* chain=chip->chain;
    int i;

    switch (chain->jstate) {
    case jtag_TLR:
        if (jtag_action(chain, 0, 0, 0, -1)) return -1; /* RTI */
    case jtag_RTI:
    case jtag_UD:
        if (jtag_action(chain, 1, 0, 0, -1)) return -1; /* SDS */
        if (jtag_action(chain, 1, 0, 0, -1)) return -1; /* SIS */
        if (jtag_action(chain, 0, 0, 0, -1)) return -1; /* CI */
        if (jtag_action(chain, 0, 1, 0, -1)) return -1; /* SI */
        break;
    default:
        printf("jtag_instruction: unexpected state %s\n",
                jtag_state_names[chain->jstate]);
        return -1;
    }

    for (i=chip->pre_c_bits; i>0; i--) {
        if (jtag_action(chain, 0, 1, 0, i)) return -1;                 /* SI */
    }
    for (i=chip->chipdata->ir_len-1; i>=0; i--) {
        if (jtag_action(chain, i?0:!chip->after_c_bits, icode&1, 0, i))
            return -1; /* SI/E1I */
        icode>>=1;
    }
    for (i=chip->after_c_bits-1; i>=0; i--) {
        if (jtag_action(chain, i?0:1, 1, 0, i)) return -1;         /* SI/E1I */
    }

    if (jtag_action(chain, 1, 1, 0, -1)) return -1;                    /* UI */

    return 0;
}
#endif
/****************************************************************************/
static int
jtag_read_ids(struct jtag_chain* chain, ems_u32** IDs, int* IDnum)
{
    ems_u32* ids=0;
    ems_u32 id;
    int i, num=0;

    *IDs=0;
    *IDnum=0;

    if (jtag_force_TLR(chain))
        return -1;

    jtag_action(chain, 0, 0, 0, -1);                                  /* RTI */
    jtag_action(chain, 1, 0, 0, -1);                                  /* SDS */
    jtag_action(chain, 0, 0, 0, -1);                                  /* CD */

    do {
        for (i=0; i<32; i++) {  /*  shift DR */
            jtag_action(chain, 0, 0, 0, i);
        }
        jtag_data(chain, &id);
        if (id) {
            /*printf("jtag ID 0x%08x\n", id);*/
            ids=realloc(ids, sizeof(ems_u32)*(num+1));
            if (ids==0) {
                printf("jtag_read_ids: malloc: %s\n", strerror(errno));
                return -1;
            }
            for (i=num; i>0; i--)
                ids[i]=ids[i-1];
            ids[0]=id;
            num++;
        }
    } while (id && (num<8));

    /* return to RTI state */
    jtag_action(chain, 1, 0, 0, -1);                                  /* E1D */
    jtag_action(chain, 1, 0, 0, -1);                                  /* UD */
    jtag_action(chain, 0, 0, 0, -1);                                  /* RTI */

    *IDs=ids;
    *IDnum=num;
    return 0;
}
/****************************************************************************/
void
jtag_free_chain(struct jtag_chain* chain)
{
    free(chain->chips);
    chain->chips=0;
}
/****************************************************************************/
plerrcode
jtag_init_chain(struct jtag_chain* chain)
{
    int i, bits;
    ems_u32* ids;

    if (jtag_read_ids(chain, &ids, &chain->num_chips))
        return plErr_System;

    chain->chips=malloc(sizeof(struct jtag_chip)*chain->num_chips);
    chain->valid=1;
    chain->jstate_count=0;

    for (i=0; i<chain->num_chips; i++) {
        struct jtag_chip* chip=chain->chips+i;
        chip->chain=chain;
        chip->id=ids[i];
        chip->version=(ids[i]>>28)&0xf;
        find_jtag_chipdata(chip);
        if (chip->chipdata==0)
            chain->valid=0;
    }

    if (!chain->valid) {
        free(ids);
        return plErr_BadModTyp;
    }

    for (i=0, bits=0; i<chain->num_chips; i++) {
        struct jtag_chip* chip=chain->chips+i;
        chip->pre_d_bits=chain->num_chips-i-1;
        chip->after_d_bits=i;
        chip->after_c_bits=bits;
        bits+=chip->chipdata->ir_len;
    }
    for (i=chain->num_chips-1, bits=0; i>=0; i--) {
        struct jtag_chip* chip=chain->chips+i;
        chip->pre_c_bits=bits;
        bits+=chip->chipdata->ir_len;
    }

    free(ids);
    chain->state=0;
    return plOK;
}
/****************************************************************************/
void
jtag_dump_chain(struct jtag_chain* chain)
{
    int i;

    for (i=0; i<chain->num_chips; i++) {
        struct jtag_chip* chip=chain->chips+i;
        printf("jtag ID 0x%08x ", chip->id);
        if (chip->chipdata!=0) {
            printf("  %s ir_len=%d ver=%d\n", chip->chipdata->name,
                chip->chipdata->ir_len, chip->version);
#if 0
            printf("pre_d=%d after_d=%d pre_c=%d after_c=%d ir=%d %s\n",
                chip->pre_d_bits, chip->after_d_bits,
                chip->pre_c_bits, chip->after_c_bits,
                chip->chipdata->ir_len,
                chip->chipdata->name);
#endif
        } else {
            printf("  chip not known\n");
        }
    }
}
/****************************************************************************/
int
jtag_irlen(struct jtag_chain* chain)
{
    int maxlen=(chain->num_chips+1)*32, n=0, i;

    if (jtag_force_TLR(chain))
        return -1;

    jtag_action(chain, 0, 0, 0, -1);                                  /* RTI */
    jtag_action(chain, 1, 0, 0, -1);                                  /* SDS */
    jtag_action(chain, 1, 0, 0, -1);                                  /* SIS */
    jtag_action(chain, 0, 0, 0, -1);                                  /* CI */
    for (i=0; i<maxlen; i++) {
        jtag_action(chain, 0, 0, 0, i);                               /* SI */
    }
    for (i=0; i<maxlen; i++) {
        int d;
        jtag_action(chain, 0, 1, &d, i);                              /* SI */
        if (!d)
            n=i;
    }
    /*printf("n1: %d\n", n+2);*/

    /* return to RTI state */
    /* !!! all instruction registers MUST contain BYPASS when we enter E1I */
    jtag_action(chain, 1, 0, 0, -1);                                  /* E1I */
    jtag_action(chain, 1, 0, 0, -1);                                  /* UI */
    jtag_action(chain, 0, 0, 0, -1);                                  /* RTI */

    return n+2;
}
/****************************************************************************/
int
jtag_datalen(struct jtag_chain* chain)
{
    int maxlen=(chain->num_chips+1)*32, n=0, i;

    if (jtag_force_TLR(chain))
        return -1;

    jtag_action(chain, 0, 0, 0, -1);                                  /* RTI */
    jtag_action(chain, 1, 0, 0, -1);                                  /* SDS */
    jtag_action(chain, 1, 0, 0, -1);                                  /* SIS */
    jtag_action(chain, 0, 0, 0, -1);                                  /* CI */
    /* set all chips to 'bypass' */
    for (i=0; i<maxlen; i++) {
        jtag_action(chain, 0, 1, 0, i);                               /* SI */
    }
    jtag_action(chain, 1, 0, 0, -1);                                  /* E1I */
    jtag_action(chain, 1, 0, 0, -1);                                  /* UI */
    jtag_action(chain, 1, 0, 0, -1);                                  /* SDS */
    jtag_action(chain, 0, 0, 0, -1);                                  /* CD */
    for (i=0; i<maxlen; i++) {
        jtag_action(chain, 0, 1, 0, i);                               /* SD */
    }
    for (i=0; i<maxlen; i++) {
        int d;
        jtag_action(chain, 0, 0, &d, i);                              /* SD */
        if (d)
            n=i;
    }
    printf("n1: %d\n", n+2);

    /* return to RTI state */
    jtag_action(chain, 1, 0, 0, -1);                                  /* E1D */
    jtag_action(chain, 1, 0, 0, -1);                                  /* UD */
    jtag_action(chain, 0, 0, 0, -1);                                  /* RTI */

    return n+2;
}
/****************************************************************************/
#ifdef NEVER
int
jtag_id(struct jtag_chip* chip, ems_u32* id)
{
    struct jtag_chain* chain=chip->chain;

    if (jtag_force_TLR(chain))
        return -1;

    if (jtag_instruction(chip, IDCODE))
        return -1;

    if (jtag_rd_data32(chip, 32, id))
        return -1;
    printf("ID=0x%08x\n", *id);

    return 0;
}
#endif
/****************************************************************************/
/****************************************************************************/
