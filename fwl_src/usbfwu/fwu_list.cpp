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

#include "stdafx.h"

#include "fwu_list.h"

//===========================================================================
//
//===========================================================================
s32  fwu_list_init(FWU_LISTINFO * lst_i)
{
   if(lst_i == NULL)
      return ERR_BAD_INPUT_VALUE;
   lst_i->curr_capacity = 0;
   lst_i->num_elem = 0;
   lst_i->pList = 0;
   return 0;
}

//===========================================================================
//
//===========================================================================
s32   fwu_list_add(FWU_LISTINFO * lst_i,void * ptr)
{
  s32 rc,tmp;
  tmp = lst_i->num_elem;
  if(tmp == lst_i->curr_capacity)
  {
     rc = fwu_list_grow(lst_i);
     if(rc != 0)
        return rc;
  }
  lst_i->pList[tmp] = ptr;
  lst_i->num_elem++;
  return 0;
}

//===========================================================================
//
//===========================================================================
s32  fwu_list_records(FWU_LISTINFO * lst_i)
{
    return        lst_i->num_elem;
}
//===========================================================================
//
//===========================================================================
s32  fwu_list_grow(FWU_LISTINFO * lst_i)
{
  s32 delta;

  if(lst_i->curr_capacity > 64)
     delta = lst_i->curr_capacity >> 2; //-- div 4
  else
  {
     if(lst_i->curr_capacity > 8)
        delta = 16;
     else
        delta = 4;
  }
  return fwu_list_set_capacity(lst_i,lst_i->curr_capacity + delta);
}

//===========================================================================
//
//===========================================================================
s32  fwu_list_set_capacity(FWU_LISTINFO * lst_i,s32 new_capacity)
{
   void ** pNewData;

   if(new_capacity == 0)
   {
      if(lst_i->pList)
         ::GlobalFree(lst_i->pList);
      lst_i->pList = NULL;
      lst_i->curr_capacity = 0;
   }
   else if(new_capacity < lst_i->curr_capacity && new_capacity > 0)
   {
      pNewData = (void **)::GlobalAlloc(GMEM_FIXED,new_capacity * sizeof(void*));
      if(lst_i->pList)
      {
         memcpy(pNewData,lst_i->pList,new_capacity * sizeof(void*));
         ::GlobalFree(lst_i->pList);
      }
      lst_i->pList = pNewData;
      lst_i->curr_capacity = new_capacity;
   }
   else if(new_capacity > lst_i->curr_capacity)
   {
      pNewData = (void **)::GlobalAlloc(GMEM_FIXED,new_capacity * sizeof(void*));
      if(lst_i->pList)
          memcpy(pNewData,lst_i->pList,lst_i->curr_capacity * sizeof(void*));
      memset(&pNewData[lst_i->curr_capacity], 0,
               (new_capacity - lst_i->curr_capacity) * sizeof(void*));
      if(lst_i->pList)
         ::GlobalFree(lst_i->pList);
      lst_i->pList = pNewData;
      lst_i->curr_capacity = new_capacity;
   }
   else
      return -1; //-- ????
   return 0; //-- O.K.
}

//===========================================================================
//
//===========================================================================
s32  fwu_list_insert(FWU_LISTINFO * lst_i,s32 pos,void * ptr)
{
   s32 rc;
   if(lst_i == NULL)
      return ERR_BAD_INPUT_VALUE;
   if(pos < 0 || pos > lst_i->num_elem)
      return ERR_BAD_INPUT_VALUE;
   if(lst_i->num_elem == lst_i->curr_capacity)
   {
      rc = fwu_list_grow(lst_i);
      if(rc != 0)
         return rc;
   }
   if(pos < lst_i->num_elem)
     memmove(&lst_i->pList[pos+1],&lst_i->pList[pos],
                          (lst_i->num_elem - pos) * sizeof(void*));
   lst_i->pList[pos] = ptr;
   lst_i->num_elem++;
   return 0; //-- O.K.
}

//===========================================================================
//
//===========================================================================
s32  fwu_list_delete(FWU_LISTINFO * lst_i,s32 pos)
{
   s32 nm;

   if(lst_i == NULL)
      return ERR_BAD_INPUT_VALUE;
   if(pos < 0 || pos >= lst_i->num_elem)
      return ERR_BAD_INPUT_VALUE;

   nm = lst_i->num_elem - (pos + 1);
   if(nm)
      memmove(&lst_i->pList[pos], &lst_i->pList[pos+1],nm * sizeof(void*));
   lst_i->num_elem--;
   return 0;
}
//===========================================================================
//
//===========================================================================
void * fwu_list_get(FWU_LISTINFO * lst_i,s32 pos)
{
   if(lst_i == NULL)
      return NULL;

   if(lst_i->pList == NULL || pos < 0 || pos >= lst_i->num_elem)
      return NULL; //-- ????

   return lst_i->pList[pos];
}

//===========================================================================
//
//===========================================================================
s32  fwu_list_exchange(FWU_LISTINFO * lst_i,s32 pos1,s32 pos2)
{
   void * pTmp;
   if(lst_i == NULL)
      return ERR_BAD_INPUT_VALUE;
   if(lst_i->pList == NULL || pos1 < 0 || pos2 < 0 ||
                 pos1 >= lst_i->num_elem || pos1 >= lst_i->num_elem)
      return ERR_BAD_INPUT_VALUE;

   pTmp = lst_i->pList[pos1];
   lst_i->pList[pos1] = lst_i->pList[pos2];
   lst_i->pList[pos2] = pTmp;
   return 0; //-- O.K.
}

//===========================================================================
//
//===========================================================================
s32   fwu_list_sort(FWU_LISTINFO * lst_i,fwu_list_compare compare)
{
  s32  base,limit;
  s32  stack[40], *sp;
  s32  i, j;
  s32  thresh;
  s32  rc;

   if(lst_i == NULL)
      return ERR_BAD_INPUT_VALUE;
   if(lst_i->num_elem < 2)
      return 0; //-- O.K.

   thresh = 7;
   sp = stack;
   limit = lst_i->num_elem;

   base = 0;

   for(;;)                              //-- repeat until done then return
   {
      while(limit - base  > thresh)     //-- if more than thresh elements
      {
         rc = fwu_list_exchange(lst_i,((limit - base)>>1) + base , base);
         if(rc != 0)
            return rc;
         i = base + 1;                    //-- i scans from left to right
         j = limit - 1;                   //-- j scans from right to left

         if(compare(lst_i->pList[i], lst_i->pList[j]) > 0 )       //-- Sedgewick's
         {
            rc = fwu_list_exchange(lst_i,i, j);     //-- three-element sort
            if(rc != 0)
               return rc;
         }
         if(compare(lst_i->pList[base],lst_i->pList[j]) > 0 )    //-- sets things up
         {
            rc = fwu_list_exchange(lst_i,base, j);    //--   so that
            if(rc != 0)
               return rc;
         }
         if(compare(lst_i->pList[i],lst_i->pList[base]) > 0 )    //--  i <= base <= j
         {
            rc = fwu_list_exchange(lst_i,i, base);   //-- base is the pivot element
            if(rc != 0)
               return rc;
         }
         for(;;)
         {
            do{                           //-- move i right until i >= pivot
              i ++;
            }
            while(compare(lst_i->pList[i],lst_i->pList[base]) < 0);

            do{                           //-- move j left until j <= pivot
              j --;
            }
            while(compare(lst_i->pList[j],lst_i->pList[base]) > 0);

            if (i > j)                    //-- break loop if indexes crossed
               break;
            rc = fwu_list_exchange(lst_i,i,j);    //-- else swap elements, keep scanning
            if(rc != 0)
               return rc;
         }
         rc = fwu_list_exchange(lst_i,base,j);    //-- move pivot into correct place
         if(rc != 0)
            return rc;
         if(j-base > limit - i)          //-- if left subfile is larger...
         {
            sp[0] = base;                 //-- stack left subfile base
            sp[1] = j;                    //-- and limit
            base = i;                     //-- sort the right subfile
         }
         else                             //-- else right subfile is larger
         {
            sp[0] = i;                     //-- stack right subfile base
            sp[1] = limit;                 //-- and limit
            limit = j;                     //-- sort the left subfile
         }
         sp += 2;                         //-- increment stack pointer
      } // for(;;)

      //-- Insertion sort on remaining subfile.
      i = base + 1;
      while(i < limit)
      {
         j = i;
         while(j > base && compare(lst_i->pList[j - 1],lst_i->pList[j]) > 0)
         {
            rc = fwu_list_exchange(lst_i,j-1,j);
            if(rc != 0)
               return rc;
            j --;
         }
         i ++;
      }
      if(sp > stack)       //-- if any entries on stack...
      {
         sp -= 2;           //-- pop the base and limit
         base = sp[0];
         limit = sp[1];
      }
      else                  //-- else stack empty, all done
        break;              //-- Return.
   }//for(;;)

   return 0; //-- O.K.
}

//===========================================================================
//
//===========================================================================
s32   fwu_list_search(FWU_LISTINFO * lst_i,void * item_to_search,
                        s32 * res_index,fwu_list_compare compare)
{
  s32 low,mid,high;
  s32 cond;
  s32 nel;

  nel = lst_i->num_elem;

  if(nel != 0)
  {
     low = 0;
     high = nel - 1;
     while(low <= high)
     {
        mid = (low + high) >> 1;
        cond = compare(item_to_search,lst_i->pList[mid]);
        if(cond < 0)
        {
           if(mid == 0)
              break;
           high = mid - 1;
        }
        else if(cond > 0)
        {
           low = mid + 1;
           if(low == 0)
              break;
        }
        else
        {
           *res_index = mid;
           return SRCH_EQUATE;
        }
     }
  }
 //-----------------
  if(cond < 0)
  {
     *res_index = mid;
     return SRCH_MORE;
  }
  else if(cond > 0)
  {
     *res_index = mid;
     return SRCH_LESS;
  }
  //-- here -  cond == 0 or internal err
  *res_index = -1;
  return SRCH_ERROR;
}

//===========================================================================
//
//===========================================================================
s32   fwu_list_insert_by_order(FWU_LISTINFO * lst_i,void * item_to_insert,
                                    fwu_list_compare compare,s32 * insert_mode)
{
   s32 rc;
   s32 res_ind;
   if(lst_i == NULL)
       return ERR_BAD_INPUT_VALUE;

   if(lst_i->num_elem >0)
   {
      rc = fwu_list_search(lst_i,item_to_insert,&res_ind,compare);

      switch(rc)
      {
         case SRCH_EQUATE:

            if(*insert_mode == INSERT_DUP)
            {
               rc = fwu_list_insert(lst_i,res_ind,item_to_insert);
               if(rc !=0)
                  return rc;
            }
            else if(*insert_mode == NOT_INSERT_DUP)
            {
               *insert_mode = res_ind;
               return ERR_ENTRY_DUP;
            }
            else
               return ERR_BAD_INPUT_VALUE;
            break;

         case SRCH_MORE:

            rc = fwu_list_insert(lst_i,res_ind,item_to_insert);
            if(rc !=0)
                return rc;
            break;

         case SRCH_LESS:

            if(res_ind + 1 > lst_i->num_elem)
            {
               rc = fwu_list_add(lst_i,item_to_insert);
               if(rc !=0)
                  return rc;
            }
            else
            {
               rc = fwu_list_insert(lst_i,res_ind+1,item_to_insert);
               if(rc !=0)
                  return rc;
            }
            break;

         case SRCH_ERROR:
            return -1;
         default:
            return -1;
      }
   }
   else
   {
      rc = fwu_list_add(lst_i,item_to_insert);
      return rc;
   }
   return 0; //-- O.K.
}

//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================
//===========================================================================


