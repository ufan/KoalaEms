/*
 * procs/unixvme/sis3316/sis3316.c
 * created 2015-Mar-11 PW
 */
static const char* cvsid __attribute__((unused))=
    "$ZEL$";

#include <sconf.h>
#include <debug.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <errorcodes.h>
#include <modultypes.h>
#include <rcs_ids.h>
#include "../../procs.h"
#include "../../../objects/domain/dom_ml.h"
#include "../../../lowlevel/unixvme/vme.h"
#include "../../../lowlevel/devices.h"
#include "../vme_verify.h"

extern ems_u32* outptr;
extern int wirbrauchen;
extern int *memberlist;

RCS_REGISTER(cvsid, "procs/unixvme/sis3316")

/*
 * SIS3316 125/250 MHz 16 channel FADC
 * A32D32/BLT32/MBLT64
 * occupies 16777216 Byte of address space
 */
/*
 *
0x000000..0x00001C R/W VME FPGA interface registers
0x000020..0x0000FC R/W VME FPGA registers
0x000400..0x00043C   W VME FPGA key addresses (with Broadcast functionality)

0x001000..0x001FFC R/W ADC FPGA 1: ch1-ch4 registers
0x002000..0x002FFC R/W ADC FPGA 2: ch5-ch8 registers
0x003000..0x003FFC R/W ADC FPGA 3: ch9-ch12 registers
0x004000..0x004FFC R/W ADC FPGA 4: ch13-ch16 registers

0x100000..0x1FFFFC R/W ADC FPGA 1: ch1-ch4 Memory Data FIFO
0x200000..0x2FFFFC R/W ADC FPGA 2: ch5-ch8 Memory Data FIFO
0x300000..0x3FFFFC R/W ADC FPGA 3: ch9-ch12 Memory Data FIFO
0x400000..0x4FFFFC R/W ADC FPGA 4: ch13-ch16 Memory Data FIFO

VME FPGA interface registers
0x00000000 R/W Control/Status Register (J-K register)
0x00000004 R   Module Id. and Firmware Revision register
0x00000008 R/W Interrupt configuration register
0x0000000C R/W Interrupt control register

0x00000010 R/W Interface Access Arbitration control/status register
0x00000014 R/W CBLT/Broadcast Setup register
0x00000018 R/W Internal test
0x0000001C R/W Hardware Version register

VME FPGA registers
0x00000020 R   Temperature register
0x00000024 R/W Onewire control/status register (EEPROM DS2430)
0x00000028 R   Serial number register
0x0000002C R/W Internal data transfer speed setting register

0x00000030
0x00000034
0x00000038
0x0000003C

0x00000040
0x00000044
0x00000048
0x0000004C

0x00000050
0x00000054
0x00000058
0x0000005C

0x00000060
0x00000064
0x00000068
0x0000006C

0x00000070
0x00000074
0x00000078
0x0000007C

0x00000080
0x00000084
0x00000088
0x0000008C

0x00000090
0x00000094
0x00000098
0x0000009C

0x000000A0
0x000000A4
0x000000A8
0x000000AC

0x000000B0
0x000000B4
0x000000B8
0x000000BC







 */


#if 0

After Power Up or after any Sample Clock changes require the following 
sequence:
1. Issue a Key Register Reset command (disarms the sample logic, too)
2. Setup the ADC Sample Clock distribution logic via the ADC Sample Clock 
   distribution control register
3. In case of use
     Internal CLK: program internal CLK oscillator and wait until the clock
                   is stable (min. 10ms)
     ...
4. Issue a Key ADC Clock DCM/PLL Reset command.
5. Calibrate and configure the ADC FPGA input logic of the ADC data inputs 
via the ADC Input tap delay registers. 
6. Setup sample logic

#endif