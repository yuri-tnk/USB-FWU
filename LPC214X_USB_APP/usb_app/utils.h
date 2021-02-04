/*
TNKernel real-time kernel - USB examples

Copyright © 2004,2006 Yuri Tiomkin
All rights reserved.

Permission to use, copy, modify, and distribute this software in source
and binary forms and its documentation for any purpose and without fee
is hereby granted, provided that the above copyright notice appear
in all copies and that both that copyright notice and this permission
notice appear in supporting documentation.

THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL YURI TIOMKIN OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

//----------------------------------------------------------------------------
//    VALID ONLY FOR 'LPC2146 USB APPLICATION' PROJECT
//----------------------------------------------------------------------------

#ifndef  _UTILS_H_
#define  _UTILS_H_

#define  LED_APP_MASK        (1<<29)

#define  IRQ_RAM_ADDR        0x40000018
#define  FIQ_RAM_ADDR        0x4000001C
#define  IRQ_RAM_FUNC_ADDR   0x40000038
#define  FIQ_RAM_FUNC_ADDR   0x4000003C

#define  RAM_START_ADDR      0x40000000

   //-- Host --

#define  GET_FW_INFO       0x05
#define  GET_FW            0x07
#define  FW_RX_RDY         0x09   //-- internal
#define  END_OK            0x0B
#define  END_ABORT         0x0D
#define  PUT_FW_INFO       0x10
#define  FW_WR             0x12
#define  APP_SEND_DATA     0x14
#define  GET_ISFWU         0x16
#define  SWITCH_TO_FWU     0x18
#define  USER_ABORT        0x0D

#define  APP_CMD           0x50

  //-- Application specific
#define  SEND_DEMO_DATA  0x1111

   //-- Device  --

#define  FW_INFO_ASK       0x70
#define  END_ASK           0x71
#define  ABORT_ASK         0x72
#define  PUT_FW_INFO_ASK   0x73
#define  FW_WR_RDY         0x78
#define  FW_WR_ERR         0x75
#define  APP_SEND_ASK      0x7A



  //--  FW
#define  FW_PRODUCT_ID_OFFSET       4
#define  FW_VERSION_OFFSET          8
#define  FW_LENGTH_OFFSET          12
#define  FW_CRC_OFFSET             16

#define  FW_ROM_PRODUCT_ID_OFFSET   0
#define  FW_ROM_VERSION_OFFSET      4
#define  FW_ROM_LENGTH_OFFSET       8
#define  FW_ROM_CRC_OFFSET         12

#define  CTRL_BUF_CRC_OFF          60
#define  CTRL_BUF_SIZE             64

#define  STREAM_BUF_SIZE         4096

#define  NOW_CHK_BREAK       (4096/64)

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))

#define LCR_DISABLE_LATCH_ACCESS    0x00000000
#define LCR_ENABLE_LATCH_ACCESS     0x00000080


//---- Prototypes -----

void  HardwareInit(void);
char * s_itoa(char * buffer, int i);
void * s_memset(void * dst,  int ch, int length);
void * s_memcpy(void * s1, const void *s2, int n);
unsigned int calc_crc(unsigned char * buf,int nbytes);
void make_cmd(int cmd,unsigned char * buf);

//-- Interrupt functions prototypes ---

void tn_int_default_func(void);
void tn_timer0_int_func(void);
void tn_uart0_int_func(void);

//--- TNKernel core functions

void tn_arm_enable_interrupts(void);
void tn_arm_disable_interrupts(void);

#endif


