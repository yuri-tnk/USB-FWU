/*
TNKernel USB Firmware Upgrader Demo

Copyright © 2006,2008 Yuri Tiomkin
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

#ifndef  _FWU_LIST_H_
#define  _FWU_LIST_H_


#define ERR_BAD_INPUT_VALUE   (-2)
#define ERR_ENTRY_DUP         (-3)


#define s32 __int32
typedef struct FWU_LISTINFO_tag
{
   s32 curr_capacity;
   s32 num_elem;
   void ** pList;
}FWU_LISTINFO;

typedef s32 (*fwu_list_compare)(void * Item1, void * Item2);

#define   SRCH_EQUATE     1
#define   SRCH_MORE       2
#define   SRCH_LESS       3
#define   SRCH_ERROR      4

#define   INSERT_DUP      1
#define   NOT_INSERT_DUP  2

s32   fwu_list_add(FWU_LISTINFO * lst_i,void * ptr);
s32   fwu_list_delete(FWU_LISTINFO * lst_i,s32 pos);
s32   fwu_list_insert(FWU_LISTINFO * lst_i,s32 pos,void * ptr);
void * fwu_list_get(FWU_LISTINFO * lst_i,s32 pos);
s32   fwu_list_put(FWU_LISTINFO * lst_i,s32 pos,void * ptr);
s32   fwu_list_insert_by_order(FWU_LISTINFO * lst_i,void * item_to_insert,
                   fwu_list_compare compare,s32 * insert_mode);

s32   fwu_list_exchange(FWU_LISTINFO * lst_i,s32 pos1,s32 pos2);
s32   fwu_list_sort(FWU_LISTINFO * lst_i,fwu_list_compare compare);
s32   fwu_list_search(FWU_LISTINFO * lst_i,void * item_to_search,
                             s32 * res_index,fwu_list_compare compare);


s32   fwu_list_init(FWU_LISTINFO * lst_i);
void  fwu_list_empty(FWU_LISTINFO * lst_i);
s32   fwu_list_records(FWU_LISTINFO * lst_i);

s32   fwu_list_grow(FWU_LISTINFO * lst_i);
s32   fwu_list_set_capacity(FWU_LISTINFO * lst_i,s32 new_capacity);


#endif
