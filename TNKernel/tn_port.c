/*
TNKernel real-time kernel

Copyright � 2004,2005 Yuri Tiomkin
All rights reserved.

Permission to use, copy, modify, and distribute this software in source
and binary forms and its documentation for any purpose and without fee
is hereby granted, provided that the above copyright notice appear
in all copies and that both that copyright notice and this permission
notice appear in supporting documentation.

THIS SOFTWARE IS PROVIDED BY THE YURI TIOMKIN AND CONTRIBUTORS "AS IS" AND
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

/* ver 2.0  */

#include "tn.h"

//----------------------------------------------------------------------------
// Processor specific routine - here for ARM7
// sizeof(void*) = sizeof(int)
//----------------------------------------------------------------------------
unsigned int * tn_stack_init(void * task_func,void * stack_start, void * param)
{
   unsigned int * stk;

 //-- filling register's position in stack(except R0,CPSR,SPSR positions) -
 //-- for debugging only

   stk  = (unsigned int *)stack_start; // Load stack pointer
   *stk = (unsigned int)task_func;   // Entry Point
   stk--;
   *stk = (unsigned int)tn_task_exit;    // LR  //0x14141414L
   stk--;
   *stk = 0x12121212L;    // R12
   stk--;
   *stk = 0x11111111L;    // R11
   stk--;
   *stk = 0x10101010L;    // R10
   stk--;
   *stk = 0x09090909L;    // R9
   stk--;
   *stk = 0x08080808L;    // R8
   stk--;
   *stk = 0x07070707L;    // R7
   stk--;
   *stk = 0x06060606L;    // R6
   stk--;
   *stk = 0x05050505L;    // R5
   stk--;
   *stk = 0x04040404L;    // R4
   stk--;
   *stk = 0x03030303L;    // R3
   stk--;
   *stk = 0x02020202L;    // R2
   stk--;
   *stk = 0x01010101L;    // R1
   stk--;
   *stk = (unsigned int)param;      // R0 : task's function argument
   stk--;
   *stk = (unsigned int)(0x13uL);   // CPSR  (Enable both IRQ and FIQ ints)
   stk--;
   *stk = (unsigned int)(0x13uL);   // SPSR

   return stk;
}

//----------------------------------------------------------------------------
// This functions will works for all processors where
// sizeof(void*) = sizeof(int) (include ARM7)
//----------------------------------------------------------------------------
TN_TCB * get_task_by_timer_queque(CDLL_QUEUE * que)
{
   unsigned  int * ptr;
   if(que == NULL)
      return NULL;

   ptr = (unsigned int*)que;
   ptr --;
   ptr -= (sizeof(CDLL_QUEUE)/sizeof(void*));
   return (TN_TCB *)ptr;
}
//----------------------------------------------------------------------------
TN_TCB * get_task_by_tsk_queue(CDLL_QUEUE * que)
{
   unsigned  int * ptr;
   if(que == NULL)
      return NULL;
   ptr = (unsigned int*)que;
   ptr --;
   return (TN_TCB *)ptr;
}
//----------------------------------------------------------------------------
TN_TCB * get_task_by_block_queque(CDLL_QUEUE * que)
{
   unsigned  int * ptr;
   if(que == NULL)
      return NULL;

   ptr = (unsigned int*)que;
   ptr --;
   ptr -= (sizeof(CDLL_QUEUE)/sizeof(void*)) << 1;
   return (TN_TCB *)ptr;
}

//----------------------------------------------------------------------------
TN_MUTEX * get_mutex_by_mutex_queque(CDLL_QUEUE * que)
{
   unsigned  int * ptr;
   if(que == NULL)
      return NULL;

   ptr = (unsigned int*)que;
   ptr -= (sizeof(CDLL_QUEUE)/sizeof(void*));

   return (TN_MUTEX *)ptr;
}

//----------------------------------------------------------------------------
TN_MUTEX * get_mutex_by_lock_mutex_queque(CDLL_QUEUE * que)
{
   unsigned  int * ptr;
   if(que == NULL)
      return NULL;

   ptr = (unsigned int*)que;
   ptr -= (sizeof(CDLL_QUEUE)/sizeof(void*)) << 1; //-- * 2

   return (TN_MUTEX *)ptr;
}

//----------------------------------------------------------------------------
TN_MUTEX * get_mutex_by_wait_queque(CDLL_QUEUE * que)
{
   unsigned  int * ptr;
   if(que == NULL)
      return NULL;

   ptr = (unsigned int*)que;

   return (TN_MUTEX *)ptr;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

