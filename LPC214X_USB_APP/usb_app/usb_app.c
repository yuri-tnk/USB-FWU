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

/*============================================================================
  usb_app.c


    Remark: In this example OS time tick period is 10 mS.
*===========================================================================*/


#include "LPC214x.h"
#include "../../tnkernel/tn.h"
#include "../../tnkernel/tn_port.h"
#include "utils.h"
#include "tn_usb_hw.h"
#include "tn_usb.h"
#include "tn_usb_ep.h"

//----------- Tasks ----------------------------------------------------------


#define  TASK_EP2_RX_PRIORITY        9
#define  TASK_EP2_TX_PRIORITY        8
#define  TASK_IO_PRIORITY           20

#define  TASK_PROCESSING_STK_SIZE   128
#define  TASK_EP2_RX_STK_SIZE       128
#define  TASK_EP2_TX_STK_SIZE       128
#define  TASK_IO_STK_SIZE           128

TN_TCB  task_ep2_rx;
TN_TCB  task_ep2_tx;
TN_TCB  task_io;

unsigned int task_ep2_rx_stack[TASK_EP2_RX_STK_SIZE];
unsigned int task_ep2_tx_stack[TASK_EP2_TX_STK_SIZE];
unsigned int task_io_stack[TASK_IO_STK_SIZE];

void task_ep2_rx_func(void * par);
void task_ep2_tx_func(void * par);
void task_io_func(void * par);

//------- Queues ----------------------------

#define  QUEUE_EP2RX_SIZE        8
#define  QUEUE_EP2TX_SIZE        8

   //--- EP2 RX queue
TN_DQUE  queueEP2RX;
void     * queueEP2RXMem[QUEUE_EP2RX_SIZE];

   //--- EP2 TX queue
TN_DQUE  queueEP2TX;
void     * queueEP2TXMem[QUEUE_EP2TX_SIZE];


//------ Fixed-Sized Memory Pool ----------------


//-- Force item size align  to 4 - not needs in this case
#define USBBulkBufItemSize  USB_MAX_PACKET0  // MAKE_ALIG(USB_MAX_PACKET0)
//-- item size - in sizeof(int) units

TN_FMP EP2RXMemPool;
unsigned int memEP2RXMemPool[QUEUE_EP2RX_SIZE * (USBBulkBufItemSize / sizeof(int))];

TN_FMP EP2TXMemPool;
unsigned int memEP2TXMemPool[QUEUE_EP2TX_SIZE * (USBBulkBufItemSize / sizeof(int))];

//--- Semaphores

TN_SEM semEP2TX;


//--- Non OS globals --------------------

   //-- USB

USB_DEVICE_INFO  gUSBInfo;
BYTE             gEP0Buf[USB_MAX_PACKET0];

extern const BYTE abDescriptors[];

EP_INFO  gEP2TX_EI;
EP_INFO  gEP2RX_EI;

unsigned char gStreamRXBuf[STREAM_BUF_SIZE];
volatile int  gUserAbort = FALSE;

//char buf[64];

extern const unsigned int crc32_ref_table[];

//-- To create big flash only - imitation big code size firmware
extern const unsigned int big_flashsize_arr[];


int processing_protocol(unsigned char * data_in);

//----------------------------------------------------------------------------
int main()
{
   unsigned int * ptr;

   tn_arm_disable_interrupts();

//-- Push  linker to include in the output file
   ptr = (unsigned int *)big_flashsize_arr[0];

   //------ Clear FW loader marks
   ptr =(unsigned int *)RAM_START_ADDR;
   *ptr = 0;
    ptr++;
   *ptr = 0;

   //------ Set interrupt vectors
    ptr = (unsigned int *)IRQ_RAM_ADDR;
   *ptr = 0xE59FF018;                      //-- ldr pc, [pc, #24]
    ptr = (unsigned int *)FIQ_RAM_ADDR;
   *ptr = 0xE59FF018;                      //-- ldr pc, [pc, #24]

   //--- Put IRQ & FIQ vectors in RAM
    ptr = (unsigned int *)IRQ_RAM_FUNC_ADDR;
   *ptr = (unsigned int)&tn_cpu_irq_isr;
    ptr = (unsigned int *)FIQ_RAM_FUNC_ADDR;
   *ptr = (unsigned int)&tn_cpu_fiq_isr;

   HardwareInit();

   //--- USB  data structs init ---

   gUSBInfo.Descriptors = (BYTE*)&abDescriptors[0];   //-- Descriptors
   gUSBInfo.EP0Status.pbuf   = &gEP0Buf[0];           //-- EP0 buffer
   tn_usb_reset_data(&gUSBInfo);

   gEP2RX_EI.queue       = &queueEP2RX;
   gEP2RX_EI.mem_pool    = &EP2RXMemPool;
   gEP2RX_EI.ep_num_phys = EP2_RX;

   gEP2TX_EI.queue       = &queueEP2TX;
   gEP2TX_EI.mem_pool    = &EP2TXMemPool;
   gEP2TX_EI.ep_num_phys = EP2_TX;

   //-----------------------------

   tn_start_system(); //-- Never returns

   return 1;
}

//----------------------------------------------------------------------------
void  tn_app_init()
{

   //--- Task IO
   tn_task_create(&task_io,                      //-- task TCB
                 task_io_func,                   //-- task function
                 TASK_IO_PRIORITY,               //-- task priority
                 &(task_io_stack                 //-- task stack first addr in memory
                    [TASK_IO_STK_SIZE-1]),
                 TASK_IO_STK_SIZE,               //-- task stack size (in int,not bytes)
                 NULL,                           //-- task function parameter
                 TN_TASK_START_ON_CREATION       //-- Creation option
                 );

   //-- Task USB EP2 rx
   tn_task_create(&task_ep2_rx,                  //-- task TCB
                 task_ep2_rx_func,               //-- task function
                 TASK_EP2_RX_PRIORITY,           //-- task priority
                 &(task_ep2_rx_stack             //-- task stack first addr in memory
                    [TASK_EP2_RX_STK_SIZE-1]),
                 TASK_EP2_RX_STK_SIZE,           //-- task stack size (in int,not bytes)
                 NULL,                           //-- task function parameter
                 TN_TASK_START_ON_CREATION       //-- Creation option
                 );

   //-- Task USB EP2 tx
   tn_task_create(&task_ep2_tx,                  //-- task TCB
                 task_ep2_tx_func,               //-- task function
                 TASK_EP2_TX_PRIORITY,           //-- task priority
                 &(task_ep2_tx_stack             //-- task stack first addr in memory
                    [TASK_EP2_TX_STK_SIZE-1]),
                 TASK_EP2_TX_STK_SIZE,           //-- task stack size (in int,not bytes)
                 NULL,                           //-- task function parameter
                 TN_TASK_START_ON_CREATION       //-- Creation option
                 );

//------------------------------------

  //--- Queues

   tn_queue_create(&queueEP2RX,        //-- Ptr to already existing TN_DQUE
                   &queueEP2RXMem[0],  //-- Ptr to already existing array of void * to store data queue entries.Can be NULL
                   QUEUE_EP2RX_SIZE    //-- Capacity of data queue(num entries).Can be 0
                   );

   tn_queue_create(&queueEP2TX,        //-- Ptr to already existing TN_DQUE
                   &queueEP2TXMem[0],  //-- Ptr to already existing array of void * to store data queue entries.Can be NULL
                   QUEUE_EP2TX_SIZE    //-- Capacity of data queue(num entries).Can be 0
                   );

  //--- Fixed-sized memory pool

   tn_fmem_create(&EP2RXMemPool,
                     (void *)&memEP2RXMemPool[0], // start_addr
                     USBBulkBufItemSize,
                     QUEUE_EP2RX_SIZE
                    );

   tn_fmem_create(&EP2TXMemPool,
                     (void *)&memEP2TXMemPool[0], // start_addr
                     USBBulkBufItemSize,
                     QUEUE_EP2TX_SIZE
                    );

  //--- Semaphores

   semEP2TX.id_sem = 0;
   tn_sem_create(&semEP2TX,0,   //-- Start value
                           1);  //-- Max value

}

//----------------------------------------------------------------------------
void task_ep2_rx_func(void * par)
{
   int rc;
   void * ptr;

   for(;;)
   {
      rc = tn_queue_receive(&queueEP2RX,(void **)&ptr,TN_WAIT_INFINITE);
      if(rc == TERR_NO_ERR)
      {
         //--- Do nothing in this example
         //------------------------------
         tn_fmem_release(&EP2RXMemPool,(void*)ptr);
      }
   }
}

//----------------------------------------------------------------------------
void task_ep2_tx_func(void * par)  //-- Stream  tx
{
   char * ptr;
   int rc;
   int chkbreak_cnt;
   int cnt;

   tn_usb_connect(TRUE);  //-- Connect USB - here

   for(;;)
   {
      rc = tn_sem_acquire(&semEP2TX,TN_WAIT_INFINITE);
      if(rc == TERR_NO_ERR)
      {
         chkbreak_cnt = 0;
         cnt = 0;

         for(;;)
         {
           //--- Payload -----------------------------------------
            rc = tn_fmem_get(&EP2TXMemPool,(void**)&ptr,TN_WAIT_INFINITE);
            if(rc == TERR_NO_ERR)
            {
        //-- Fill buf by user's data --
               s_memset(ptr,'0',12);
               s_itoa(ptr,cnt);
               s_memset(&((unsigned char *)ptr)[12],' ',50);
               ((unsigned char *)ptr)[62] = '\r';
               ((unsigned char *)ptr)[63] = '\n';
               cnt++;
               //-----------------------------
                tn_queue_send(&queueEP2TX,(void*)ptr,TN_WAIT_INFINITE);
            }
           //-----------------------------------------------------

            chkbreak_cnt++;
            if(chkbreak_cnt >= NOW_CHK_BREAK)
            {
               chkbreak_cnt = 0;
               if(gUserAbort == TRUE)
               {
                  gUserAbort = FALSE;
                  break;
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------
void task_io_func(void * par)
{
   int led_blink_state = 0;
   int led_blink_cnt   = 0;
   int led_blink_dly   = 2;
   int led_conf_cnt    = 0;
   int led_conf_state  = 0;

   rIO0SET |= LED_APP_MASK;  //-- Led - on
   for(;;)
   {
      tn_task_sleep(5);  //-- 50 mS

      //-- if no configuration then  Blink LED
      if(gUSBInfo.Configuration == 0)
      {
         led_conf_cnt++;
         if(led_conf_cnt >= 2) //-- 100 ms
         {
            led_conf_cnt = 0;
            if(led_conf_state == 0)
            {
               led_conf_state = 1;
               rIO0SET |= (1<<14);
            }
            else if(led_conf_state == 1)
            {
               led_conf_state = 0;
               rIO0CLR |= (1<<14);
            }
         }
      }
      else
         rIO0CLR |= (1<<14); //-- Conf Led - on

   //---- Application LED

      led_blink_cnt++;
      if(led_blink_state == 0)
      {
         if(led_blink_cnt >= led_blink_dly)
         {
            led_blink_cnt    =  0;
            led_blink_state  =  1;
            led_blink_dly    =  2;
            rIO0CLR |= LED_APP_MASK;  //-- Led - Off
         }
      }
      else if(led_blink_state == 1)
      {
         if(led_blink_cnt >= led_blink_dly)
         {
            led_blink_cnt   =  0;
            led_blink_state =  2;
            led_blink_dly   =  2;
            rIO0SET |= LED_APP_MASK;  //-- Led - on
         }
      }
      else if(led_blink_state == 2)
      {
         if(led_blink_cnt >= led_blink_dly)
         {
            led_blink_cnt   =  0;
            led_blink_state =  3;
            led_blink_dly   = 15;   //-- 750 msec
            rIO0CLR |= LED_APP_MASK;  //-- Led - off
         }
      }
      else if(led_blink_state == 3)
      {
         if(led_blink_cnt >= led_blink_dly)
         {
            led_blink_cnt   =  0;
            led_blink_state =  0;
            led_blink_dly   = 2;
            rIO0SET |= LED_APP_MASK;  //-- Led - on
         }
      }
   }
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------



