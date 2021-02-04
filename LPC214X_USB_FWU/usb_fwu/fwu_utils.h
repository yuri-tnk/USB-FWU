/*
   USB Firmware Upgrader

Copyright © 2006 Yuri Tiomkin
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
//    VALID ONLY FOR 'LPC2146 USB FIRMWARE UPGRADER' PROJECT
//----------------------------------------------------------------------------


#ifndef  _FWU_UTILS_H_
#define  _FWU_UTILS_H_

#define IRQ_RAM_ADDR        0x40000018
#define FIQ_RAM_ADDR        0x4000001C
#define IRQ_RAM_FUNC_ADDR   0x40000038
#define FIQ_RAM_FUNC_ADDR   0x4000003C

   //-- Op states
#define STATE_DEFAULT      0

#define USER_ABORT      0x0D
   //-- Host --

#define  GET_FW_INFO      0x05
#define  GET_FW           0x07
#define  FW_RX_RDY        0x09   //-- internal
#define  END_OK           0x0B
#define  END_ABORT        0x0D
#define  PUT_FW_INFO      0x10
#define  FW_WR            0x12
#define  GET_ISFWU        0x16
#define  SWITCH_TO_FWU    0x18


   //-- Device  --

#define  FW_INFO_ASK       0x70
#define  END_ASK           0x71
#define  ABORT_ASK         0x72
#define  PUT_FW_INFO_ASK   0x73
#define  FW_WR_RDY         0x78
#define  FW_WR_ERR         0x75

#define  GET_ISFWU_ASK     0x1234AA55


  //--  FW
#define  FW_PRODUCT_ID_OFFSET     4
#define  FW_VERSION_OFFSET        8
#define  FW_LENGTH_OFFSET        12
#define  FW_CRC_OFFSET           16

#define  FW_ROM_PRODUCT_ID_OFFSET   0
#define  FW_ROM_VERSION_OFFSET      4
#define  FW_ROM_LENGTH_OFFSET       8
#define  FW_ROM_CRC_OFFSET         12


#define  CTRL_BUF_CRC_OFF         60
#define  CTRL_BUF_SIZE            64

#define  STREAM_BUF_SIZE        4096

#define  NOW_CHK_BREAK      (4096/64)

#define  RAM_START_ADDR   0x40000000
#define  FLASH_SIZE        (256*1024)


#define  ERR_NO_ERR               0
#define  ERR_WRONG_PARAM          1
#define  ERR_WSTATE               2

//------------------------------------------------------

#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#define __min(a,b)  (((a) < (b)) ? (a) : (b))

#define LCR_DISABLE_LATCH_ACCESS    0x00000000
#define LCR_ENABLE_LATCH_ACCESS     0x00000080


  //-- Flash IAP

#define  IAP_CMD_PREPARE_SECTORS               50
#define  IAP_CMD_COPY_RAM_TO_FLASH             51
#define  IAP_CMD_ERASE_SECTORS                 52

#define  IAP_CMD_SUCCESS                        0
#define  IAP_BUSY                              11

#define  IAP_LOCATION                  0x7ffffff1


#define  PRC_CLK        60000 // 12000  //-- in KHz
//#define  FW_START_ADDR   (0x00008000)

#define  FW_LOAD_ADDR      0x00002000
#define  FW_START_OFFSET         0x20


typedef struct _SECTORGEOMETRY
{
   int size;
   int qty;
}SECTORGEOMETRY;


typedef struct _FWU_DQ
{
   unsigned char * data_fifo;
   int num_entries;
   int tail_cnt;
   int header_cnt;
}FWU_DQ;


#define  DATA_ENTRY_SIZE      64


#define  FWU_INTSAVE_DATA         int save_status_reg = 0;
#define  fwu_disable_interrupt()  save_status_reg = cpu_save_sr()
#define  fwu_enable_interrupt()   cpu_restore_sr(save_status_reg)


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

//--- Flash
int flash_write(unsigned int flash_addr,
                unsigned int ram_addr,
                int prc_clk, //-- in KHz
                int len);     //-- 256,512, etc up to min sector size(4096)

int flash_erase_sectors(unsigned int start_addr,
                        int len,      //-- any
                        int prc_clk);  //-- in KHz

void start_firmware(void);
void switch_to_firmware(void);
void do_switch_to_firmware(void);
void set_state(int state);

    //--- fwu_queue.h
void fwu_queue_create(FWU_DQ * flq,int n_entries,unsigned char * data_arr);
unsigned char * fwu_queue_tst(FWU_DQ *flq);
unsigned char * fwu_queue_tsti(FWU_DQ *flq);
void fwu_queue_put(FWU_DQ * flq);
void fwu_queue_puti(FWU_DQ * flq);
unsigned char * fwu_queue_get(FWU_DQ * flq);
unsigned char * fwu_queue_geti(FWU_DQ * flq);

void Init_CRC32_Table(void);

void cpu_irq_handler(void);

  //-- Assembler
int   cpu_save_sr(void);
void  cpu_restore_sr(int sr);
void  cpu_irq_isr(void);
void  cpu_fiq_isr(void);
void  start_firmware(void);

unsigned int iap_command(unsigned int cmd,
                                unsigned int p0,
                                unsigned int p1,
                                unsigned int p2,
                                unsigned int p3,
                                unsigned int *r0);

#endif


