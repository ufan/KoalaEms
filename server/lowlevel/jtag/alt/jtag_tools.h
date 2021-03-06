/*
 * lowlevel/jtag_tools.h
 * created 12.Aug.2005 PW
 * $ZEL: jtag_tools.h,v 1.1 2006/09/15 17:54:02 wuestner Exp $
 */

#ifndef _jtag_tools_h_
#define _jtag_tools_h_

#include <sconf.h>
#include <stdio.h>
#include <errorcodes.h>
#include <emsctypes.h>
#include "../devices.h"

plerrcode jtag_list(struct generic_dev* dev, int ci, int addr);
plerrcode jtag_check(struct generic_dev* dev, int ci, int addr);
#ifdef NEVER
plerrcode jtag_read_id(struct generic_dev* dev, int ci, int addr,
    unsigned int chip, ems_u32* id);
#endif
plerrcode jtag_read(struct generic_dev* dev, int ci, int addr,
    unsigned int chip_idx, const char* name);
plerrcode jtag_write(struct generic_dev* dev, int ci, int addr,
    unsigned int chip_idx, const char* name);
plerrcode jtag_init_chain(struct jtag_chain* chain);

#endif
