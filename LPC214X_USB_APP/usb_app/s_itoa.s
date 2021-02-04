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

/*; Simple itoa()
  ; Calculation '/10' and '%10' in assembler
  ; dramatically increases performance
  ;
*/

/*
; char * s_itoa(char * buffer, int i)
; {
;   char * ptr;
;   int rem;
;
;   ptr = buffer + 12;
;   *ptr = 0;
;   do
;   {
;      ptr--;
;      *ptr = (i % 10) + '0';
;      i /= 10;
;   }while( i > 0);
;
;   return ptr;
; }
*/
   .text
   .code 32
   .align 0
   .global s_itoa
/*
  Register usage:

  r0       - tmp
  r1,r2,r3 - for fast div10
  r4       - char * ptr;
  r5       - i  (for future extensions)
*/

s_itoa:

    stmdb    sp!,{r5,lr}

    mov     r5, r1          /*  i -> r5 */
    mov     r4, r0          /*  ptr = buffer + 12; */
    add     r4, r4, #12

    mov     r0, #0          /*   *ptr = 0; */
    strb    r0, [r4]

/* do { */
label_1:

    sub     r4, r4, #1      /*    ptr--;  */

    mov     r1,r5

/*-------------------------------------*/
/* From ARM(c) assembler code examples */

/* Fast unsigned divide by 10: dividend in a1, divisor in a2 */
/* Returns quotient in a1, remainder in a2. */

    MOV     r2, r1
    MOV     r1, r1, LSR #1
    ADD     r1, r1, r1, LSR #1
    ADD     r1, r1, r1, LSR #4
    ADD     r1, r1, r1, LSR #8
    ADD     r1, r1, r1, LSR #16
    MOV     r1, r1, LSR #3
    ADD     r3, r1, r1, ASL #2
    SUB     r2, r2, r3, ASL #1
    CMP     r2, #10
    ADDGE   r1, r1, #1
    SUBGE   r2, r2, #10
/*-------------------------------------*/

    mov     r0, r2       /*  *ptr = (i % 10) + '0'; */
    add     r0, r0, #0x30
    strb    r0, [r4]

    mov     r5, r1       /*   i /= 10; */

    subs    r5, r5, #0   /*  }while(i>0) */
    bgt     label_1

    mov     r0, r4        /* return ptr */

    ldmia   sp!,{r5,lr}
    bx lr



