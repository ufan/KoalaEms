/*
 * spect_decode.cc
 * 
 * created 26.Oct.2003 
 * $ZEL: spect_decode.cc,v 1.1 2004/02/11 12:37:38 wuestner Exp $
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <iostream.h>

#ifndef __CINT__
#include <TH1.h>
#include <TF1.h>
#endif

#include "spect_decode.hxx"

/******************************************************************************/
static int
decode_caen_v265(ems_u32* ptr, int size, TH1F* h1f, int sel_chan)
{
    int nr_mod, i, j;

    nr_mod=*ptr++; size--;

    if (size!=16) {
        printf("\ncaen v265: size=%d\n", size);
        return -1;
    }

    for (i=0; i<16; i++) {
        ems_u32 v=ptr[i];
        int r15=!!(v&(1<<12));
        int chan=(v>>13)&7;
        int val=v&0xfff;
        if (v==0xffff) {
            printf("caen_v265: v=0x%x\n", v);
            return 0;
        }
        if (r15) chan+=8;
        if (chan==sel_chan) {
            h1f->Fill(val);
        }
    }
    return 0;
}
/******************************************************************************/
static int
decode_caen_v556(ems_u32* ptr, int size, TH1F* h1f, int sel_chan)
{
    int nr_mod, i;

    nr_mod=*ptr++;
    for (i=0; i<nr_mod; i++) {
        ems_u32 head;
        int nr_data, j;

        nr_data=*ptr++;
        head=*ptr++; nr_data--;

        for (j=0; j<nr_data; j++) {
            ems_u32 v;
            v=*ptr++;
            int chan=(v>>12)&7;
            int val=v&0xfff;
            if (chan==sel_chan) {
                h1f->Fill(val);
            }
        }
    }

    return 0;
}
/******************************************************************************/
static int
decode_caen_v556_2diff(ems_u32* ptr, int size, TH1F** h1f, int sel_chan[2])
{
    int nr_mod, i, ci;
    int c[2];
    int cv[2]={0, 0};

    nr_mod=*ptr++;
    for (i=0; i<nr_mod; i++) {
        ems_u32 head;
        int nr_data, j;

        nr_data=*ptr++;
        head=*ptr++; nr_data--;

        for (j=0; j<nr_data; j++) {
            ems_u32 v;
            v=*ptr++;
            int chan=(v>>12)&7;
            int val=v&0xfff;

            for (ci=0; ci<2; ci++) {
                if (chan==sel_chan[ci]) {
                    c[ci]=val;
                    cv[ci]=1;
                }
            }
        }
        printf("c0=%d c1=%d c2=%d\n", c[0], c[1], c[0]-c[1]);
        if (cv[0] && cv[1]) {
            h1f[0]->Fill(c[0]);
            h1f[1]->Fill(c[1]);
            h1f[2]->Fill(c[0]-c[1]);
        } else {
            printf("Da fehlt was.\n");
        }
    }

    return 0;
}
/******************************************************************************/
static int
decode_sis3400(ems_u32* ptr, int size, TH1F* h1f, int sel_chan)
{
    ems_u32 v, count;
    int i;

    /* printf("\nsis3400: size=%d\n", size); */
    v=*ptr++; size--;
    if (v!=1) {
        printf("\nsis3400: %d modules\n", v);
        return -1;
    }
    count=*ptr++; size--;
    if (count!=size) {
        printf("\nsis3400: size=%d count=%d\n", size, count);
        return -1;
    }

    for (i=0; i<count; i++) {
        v=*ptr++; size--;
        int chan=(v>>20)&0x3f;
        int val=v&0xfffff;
        int trailing=v&0x80000000;
        if (!trailing && (chan==sel_chan)) {
            h1f->Fill(val);
        }
    }    
    return 0;
}
/******************************************************************************/
int
proceed_event(struct event_buffer* eb, TH1F* h1f, int sel_sev, int sel_chan)
{
    ems_u32* sptr=eb->data;
    int i;

/*
    printf("event %d trig %d sub %d size %d\n",
            eb->evno, eb->trigno, eb->subevents, eb->size);
    for (i=0; i<eb->size; i++) printf("%3d 0x%08x\n", i, eb->data[i]);
*/
    for (i=0; i<eb->subevents; i++) {
        int size;
        eb->subeventptr[i]=sptr;
        size=sptr[1];
        sptr+=size+2;
    }

    for (i=0; i<eb->subevents; i++) {
        ems_u32* ptr=eb->subeventptr[i];
/*
        printf("subevent %d size=%d\n", ptr[0], ptr[1]);
*/

        if (ptr[0]==sel_sev) {
            switch (ptr[0]) {
            case 2: decode_caen_v265(ptr+2, ptr[1], h1f, sel_chan); break;
            case 3: decode_caen_v556(ptr+2, ptr[1], h1f, sel_chan); break;
            case 4: decode_sis3400(ptr+2, ptr[1], h1f, sel_chan); break;
            }
        }
    }
    return 0;
}
/******************************************************************************/
int
proceed_event2diff(struct event_buffer* eb, TH1F* h1f[3],
        int sel_sev, int* sel_chan)
{
    ems_u32* sptr=eb->data;
    int i;

    for (i=0; i<eb->subevents; i++) {
        int size;
        eb->subeventptr[i]=sptr;
        size=sptr[1];
        sptr+=size+2;
    }

    for (i=0; i<eb->subevents; i++) {
        ems_u32* ptr=eb->subeventptr[i];

        if (ptr[0]==sel_sev) {
            switch (ptr[0]) {
            case 2:
                printf("v265 not supported with more than 1 channel\n");
                return -1;
            case 3:
                decode_caen_v556_2diff(ptr+2, ptr[1], h1f, sel_chan);
                break;
            case 4:
                printf("sis3400 not supported with more than 1 channel\n");
                return -1;
            }
        }
    }
    return 0;
}
/******************************************************************************/
/******************************************************************************/