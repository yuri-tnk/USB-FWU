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

#include "fwu_utils.h"
#include "fwu_usb_hw.h"
#include "fwu_usb.h"

 //-- Entry size is 64 bytes

//----------------------------------------------------------------------------
void fwu_queue_create(FWU_DQ * flq, int n_entries, unsigned char * data_arr)
{
   flq->data_fifo = data_arr;
   flq->num_entries = n_entries;
   flq->tail_cnt   = 0;
   flq->header_cnt = 0;
}

//----------------------------------------------------------------------------
//--  ptr to free element
unsigned char * fwu_queue_tst(FWU_DQ *flq)
{
   FWU_INTSAVE_DATA
   int flag;

   fwu_disable_interrupt();

   flag = ((flq->tail_cnt == 0 && flq->header_cnt == flq->num_entries - 1)
             || flq->header_cnt == flq->tail_cnt-1);

   fwu_enable_interrupt();

   if(flag)
      return  NULL;  //--  full

   return &flq->data_fifo[(flq->header_cnt)<<6]; //-- * 64
}

//----------------------------------------------------------------------------
//--  ptr to free element
unsigned char * fwu_queue_tsti(FWU_DQ *flq)
{
   int flag;
   flag = ((flq->tail_cnt == 0 && flq->header_cnt == flq->num_entries - 1)
             || flq->header_cnt == flq->tail_cnt-1);
   if(flag)
      return  NULL;  //--  full

   return &flq->data_fifo[(flq->header_cnt)<<6]; //-- * 64
}

//----------------------------------------------------------------------------
void fwu_queue_put(FWU_DQ * flq)
{
   FWU_INTSAVE_DATA
 //   flq->data_fifo[flq->header_cnt<<6] = data_ptr;
   fwu_disable_interrupt();

   flq->header_cnt++;
   if(flq->header_cnt >= flq->num_entries)
      flq->header_cnt = 0;

   fwu_enable_interrupt();
}

//----------------------------------------------------------------------------
void fwu_queue_puti(FWU_DQ * flq)
{
 //   flq->data_fifo[flq->header_cnt<<6] = data_ptr;

   flq->header_cnt++;
   if(flq->header_cnt >= flq->num_entries)
      flq->header_cnt = 0;
}

//----------------------------------------------------------------------------
unsigned char * fwu_queue_get(FWU_DQ * flq)
{
   FWU_INTSAVE_DATA
   unsigned char * ptr;

   fwu_disable_interrupt();

   if(flq->tail_cnt == flq->header_cnt)
   {
      fwu_enable_interrupt();
      return NULL; //-- empty
   }
   //-- rd data
   ptr  =  &flq->data_fifo[(flq->tail_cnt)<<6];  //-- * 64
   flq->tail_cnt++;
   if(flq->tail_cnt >= flq->num_entries)
      flq->tail_cnt = 0;

   fwu_enable_interrupt();

   return ptr;
}

//----------------------------------------------------------------------------
unsigned char * fwu_queue_geti(FWU_DQ * flq)
{
   unsigned char * ptr;

   if(flq->tail_cnt == flq->header_cnt)
      return NULL; //-- empty

   //-- rd data
   ptr  =  &flq->data_fifo[(flq->tail_cnt)<<6];  //-- * 64
   flq->tail_cnt++;
   if(flq->tail_cnt >= flq->num_entries)
      flq->tail_cnt = 0;

   return ptr;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

