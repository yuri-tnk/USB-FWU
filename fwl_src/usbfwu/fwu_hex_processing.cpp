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
#include "fwu_hex_processing.h"

#pragma warning(disable: 4996)

#define LINE_BUF_SIZE         1024

#define ERR_OPEN_INPUT_FILE    (-2)
#define ERR_CRC_ERR            (-3)

typedef struct _CHUNKITEM
{
   unsigned __int32 start_addr;
   int len;
   char file_name[24];
   FILE * hFile;
}CHUNKITEM;

//-- The File's Globals

static FWU_LISTINFO gFileList;
static char line_buf[LINE_BUF_SIZE];
static unsigned char line_data[256];
static int gByteWidth = 8;
static CString gErrStr;

//--- Prototypes

__int32 addr_compare_func(void * Item1, void * Item2);
static void clear_list();

// Allocate resources(file handles, memory)  - inside proc_hex_file()
// Free resources                            - inside make_output_bin_file()

//----------------------------------------------------------------------------
__int32 addr_compare_func(void * Item1, void * Item2)
{
   CHUNKITEM * p1 = (CHUNKITEM *)Item1;
   CHUNKITEM * p2 = (CHUNKITEM *)Item2;

   if(p1->start_addr < p2->start_addr)
      return -1;
   else if(p1->start_addr > p2->start_addr)
      return 1;
   return 0;
}

//----------------------------------------------------------------------------
int hexchar_to_bin(int ch)
{
   int ret_val;

   if(ch >= _T('0') && ch <= _T('9'))
      ret_val = ch - _T('0');
   else if(ch >= _T('a') && ch <= _T('f'))
      ret_val = 10 + (ch - _T('a'));
   else if(ch >= _T('A') && ch <= _T('F'))
      ret_val = 10 + (ch - _T('A'));
   else  //-- Err
      ret_val = -1;

   return ret_val;
}

//----------------------------------------------------------------------------
int hexbyte_to_bin(char * ptr)
{
   int i;
   int rc;
   int val;

   for(i=0;i<2;i++)
   {
      rc = hexchar_to_bin(*ptr);
      if(rc<0)
         return -1; //-- Error

      if(i==0)
         val = (rc<<4) & 0xF0;
      else if(i==1)
         val |= rc & 0x0F;
      ptr++;
   }
   return val;
}

//----------------------------------------------------------------------------
int proc_hex_file(char * in_file_name)
{
   FILE * hInFile;
   FILE * curr_file = NULL;
   int * pCurrChunkLen = NULL;
   char * ptr;
   int num_bytes;
   int line_crc_val;
   int rec_type;
   int i,rc,tmp_byte;
   unsigned __int32 line_addr;
   unsigned __int32 mem_addr;
   unsigned __int32 prev_mem_addr    = 0xFFFFFFFF;
   unsigned __int32 linear_base_addr = 0;
   unsigned __int32 segment_addr     = 0;
   unsigned __int32 seg_start_to_run_addr    = 0xFFFFFFFF;
   unsigned __int32 linear_start_to_run_addr = 0xFFFFFFFF;
   int file_line_num = 0;

 //  CString s1;

   gErrStr = _T("");
   clear_list();

   hInFile = fopen(in_file_name,"rt");
   if(hInFile == NULL)
   {
      gErrStr.Format("Error: could not open input file %s.\n",in_file_name);
      return -1; //-- Err
   }

   while(fgets(line_buf, LINE_BUF_SIZE, hInFile) != NULL)
   {
      file_line_num++;

      ptr = line_buf;

//-- Skip whitespaces
      for(;;)
      {
         if(*ptr != _T(' ') && *ptr != _T('\t'))
            break;
         ptr++;
      }

  //-- Colon

      if(*ptr != _T(':'))
      {
         gErrStr.Format("Error: no colon char. Line: %d.\n",file_line_num);
         return -1; //-- Err
      }
      else
         ptr++;

  //-- Num bytes

      num_bytes = hexbyte_to_bin(ptr);
      if(num_bytes < 0) //-- Error
      {
         gErrStr.Format("Error: non-hex char. Line: %d.\n",file_line_num);
         return -1; //-- Err
      }
      ptr += 2;
      line_crc_val = num_bytes;

  //-- Address

      line_addr = hexbyte_to_bin(ptr);
      if(line_addr < 0) //-- Error
      {
         gErrStr.Format("Error: non-hex char. Line: %d.\n",file_line_num);
         return -1; //-- Err
      }
      ptr += 2;
      line_crc_val += line_addr;

      tmp_byte = hexbyte_to_bin(ptr);
      if(tmp_byte < 0) //-- Error
      {
         gErrStr.Format("Error: non-hex char. Line: %d.\n",file_line_num);
         return -1; //-- Err
      }
      ptr += 2;
      line_crc_val += tmp_byte;

      line_addr <<= 8;
      line_addr |= (tmp_byte) & 0xFF;

  //-- Record type

      rec_type = hexbyte_to_bin(ptr);
      if(rec_type < 0) //-- Error
      {
         gErrStr.Format("Error: non-hex char. Line: %d.\n",file_line_num);
         return -1; //-- Err
      }
      ptr += 2;
      line_crc_val += rec_type;

  //-- data

      for(i=0;i<num_bytes;i++)
      {
         tmp_byte = hexbyte_to_bin(ptr);
         if(tmp_byte < 0) //-- Error
         {
            gErrStr.Format("Error: non-hex char. Line: %d.\n",file_line_num);
            return -1; //-- Err
         }
         line_data[i] = tmp_byte;
         line_crc_val += tmp_byte;
         ptr += 2;
      }

  //-- crc

      tmp_byte = hexbyte_to_bin(ptr);
      if(tmp_byte < 0) //-- Error
      {
         gErrStr.Format("Error: non-hex char. Line: %d.\n",file_line_num);
         return -1; //-- Err
      }
      line_crc_val += tmp_byte;
      if((line_crc_val & 0xFF) != 0) //-- Error
      {
         gErrStr.Format("Error: CRC error. Line: %d\n", file_line_num);
         return -1;
      }

  //-- Processing

      if(rec_type == 0)      //-- Data Record
      {
         mem_addr = linear_base_addr * 65536 + segment_addr * 16 + line_addr;
         if(mem_addr != prev_mem_addr)
         {
  //--------------------------------------------------------
            for(i=0; i < fwu_list_records(&gFileList); i++)
            {
               CHUNKITEM * pItem = (CHUNKITEM *)fwu_list_get(&gFileList, i);
               if(pItem)
               {
                  if(((mem_addr >= pItem->start_addr) &&
                    ((pItem->start_addr + pItem->len) - 1 > mem_addr)) ||
                    (pItem->len == 0 && mem_addr == pItem->start_addr))
                  {
                     gErrStr.Format("Error: Address overlapping. Line: %d\r\n"
                                    "       Ref  addr: 0x%08X (start=0x%08X len=%d)\r\n"
                                    "       Curr addr: 0x%08X\r\n",
                                    file_line_num,
                                    (pItem->start_addr + pItem->len) - 1,
                                     pItem->start_addr, pItem->len,
                                     mem_addr);
                     return -1;
                  }
               }
            }
  //--------------------------------------------------------
            CHUNKITEM * pChunkItem = (CHUNKITEM *)malloc(sizeof(CHUNKITEM));
            if(pChunkItem == NULL)
            {
               gErrStr.Format("Out of memory\n");
               return -1;
            }
            pChunkItem->start_addr = mem_addr;
            pChunkItem->len = 0;
            sprintf(pChunkItem->file_name,"%08X.bin",mem_addr);

            pChunkItem->hFile = fopen(pChunkItem->file_name,"wb+");
            if(pChunkItem->hFile == NULL)
            {
               gErrStr.Format("Error: could not open chunk file %s.\n",
                                             pChunkItem->file_name);
               return -1; //-- Err
            }
            __int32 insert_mode = NOT_INSERT_DUP;
            rc = fwu_list_insert_by_order(&gFileList,pChunkItem,
                                          addr_compare_func,&insert_mode);
            if(rc !=0)
            {
               gErrStr.Format("Error: could add to list(code: %d).\n",rc);
               return -1; //-- Err
            }
            curr_file = pChunkItem->hFile;
            pCurrChunkLen  = &pChunkItem->len;
         }

         for(i=0;i<num_bytes;i++)  //-- num
         {
            if(curr_file)
            {
               int fch = fputc(line_data[i], curr_file);
               if(line_data[i] != fch)
               {
                  gErrStr.Format("Error: could not write to chunk file %s.\n",
                               line_data[i]);
                  return -1; //-- Err
               }
            }
            else  //-- Error
            {
               gErrStr.Format("Error: Current file was not defined.\n");
               return -1;
            }
            if(pCurrChunkLen == NULL)
            {
               gErrStr.Format("Error: Current file length ptr was not defined.\n");
               return -1;
            }

            if(gByteWidth == 16) //-- 16 bits in byte in TI
            {
               if(i&1) //-- at 2nd byte only
               {
                  mem_addr++;
                  if(pCurrChunkLen)
                     (*pCurrChunkLen)++;
               }
            }
            else
            {
               mem_addr++;
               if(pCurrChunkLen)
                  (*pCurrChunkLen)++;
            }
         }

         prev_mem_addr = mem_addr;
      }
      else if(rec_type == 1) //-- End Of File Record
      {
         break;
      }
      else if(rec_type == 2) //-- Extended Segment Address Record
      {
    // Type '02' records are used to pre-set the Extended Segment Address.
    // With this segment address it is possible to send files up to 1Mb
    // in length. The Segment address is multiplied by 16 and then added
    // to all subsequent address fields of type '00' records to obtain
    // the effective address. By default the Extended Segment address
    // will be $0000, until it is specified by a type '02' record.
    // The address field of a type '02' record must be $00.
    // The byte count field will be $02 (the segment address consists of 2 bytes).
    // The data field of the type '02' record contains the actual
    // Extended Segment address. Bits 3..0 of this Extended Segment address
    // always must be 0!

         segment_addr = line_data[0]<< 8;
         segment_addr |= line_data[1] & 0xFF;

//         s1.Format("Extended Segment Address - Value: 0x%08X, Line: %d\n",
//                    segment_addr, file_line_num);

  //   target address = segment*16+aaaa
      }
      else if(rec_type == 3) //-- Start Segment Address Record
      {
    // Type '03' records don't contribute to file transfers. They are used
    // to specify the start address for Intel processors, like the 8086.
    // So if you would upload a file to an Intel based development board,
    // the starting address of the code can be specified with this record type.
    // This starting address will be loaded into the CS and IP registers of
    // the processor. For normal file transfers the type '03' records can be
    // ignored. The byte count of type '03' record is $04, because 4 data bytes
    // will be sent. The address field remains $0000. The data field of type '03'
    // records contain 4 bytes, the first 2 bytes represent the value to be
    // loaded into CS, the last 2 bytes are the value to be loaded into IP.
    // Bytes are sent MSB first

         seg_start_to_run_addr  = (line_data[0]<< 24) & 0xFF000000;
         seg_start_to_run_addr |= (line_data[1]<< 16) & 0x00FF0000;
         seg_start_to_run_addr |= (line_data[2]<<  8) & 0x0000FF00;
         seg_start_to_run_addr |=  line_data[3] & 0xFF;

//         s1.Format("CS:IP Address - Value: 0x%08X, Line: %d\n",
//                    seg_start_to_run_addr, file_line_num);
      }
      else if(rec_type == 4) //-- Extended Linear Address Record
      {
    // Type '04' records are used to pre-set the Linear Base Address. This 16
    // bit Linear Base Address, specified in the data area, is used to obtain
    // a full 32 bit address range when combined with the address field of type
    // '00' records. With this LBA it is possible to send files up to 4Gb in
    // length. The Linear Base Address is used as the upper 16 bits in the 32
    // bit linear address space. The lower 16 bits will come from the address
    // field of type '00' records. By default the Linear Base Address will
    // be $0000, until specified by a type '04' record. The address field of
    // a type '04' record must be $0000. The byte count field will be $02
    // (the LBA consists of 2 bytes). The data field of the type '04' record
    // contains the actual 2 byte Linear Base Address. MSB is sent first.

         linear_base_addr  = line_data[0]<< 8;
         linear_base_addr |= line_data[1] & 0xFF;

//         s1.Format("Linear Base Address - Value: 0x%08X, Line: %d\n",
//                    linear_base_addr, file_line_num);

//   target address = ulba*65536+aaaa

      }
      else if(rec_type == 5) //-- Start Linear Address Record
      {
    // Type '05' records don't contribute to file transfers. They are used to
    // specify the start address for Intel processors, like the 80386.
    // If you would upload a file to an Intel based development board,
    // the starting address of the code can be specified with a type 05 record.
    // This starting address will be loaded in the EIP register of the processor.
    // For normal file transfers the type '05' records can be ignored.
    // The byte count of type '05' records is $04, because 4 data bytes will
    // be sent. The address field remains $0000. The data field of type '05'
    // records contain the 4 byte linear 32 bit starting address to be loaded
    // into the EIP register of the processor
    //

         linear_start_to_run_addr  = (line_data[0]<< 24) & 0xFF000000;
         linear_start_to_run_addr |= (line_data[1]<< 16) & 0x00FF0000;
         linear_start_to_run_addr |= (line_data[2]<<  8) & 0x0000FF00;
         linear_start_to_run_addr |=  line_data[3] & 0xFF;

//         s1.Format("Linear Run Start Address - Value: 0x%08X, Line: %d\n",
//                    linear_start_to_run_addr, file_line_num);

      }
   }

//--- Statistics and Exit

   if(hInFile)
      fclose(hInFile);

   return 0;
}

//----------------------------------------------------------------------------
void dump_list(FWU_LISTINFO * lst_i)
{
   CString s;
   int len;
   for(int i=0; i < fwu_list_records(lst_i); i++)
   {
      CHUNKITEM * pChunkItem = (CHUNKITEM *)fwu_list_get(lst_i, i);

      len = 0;
      if(pChunkItem->hFile)
      {
         fseek(pChunkItem->hFile,0,SEEK_END);
         len = ftell(pChunkItem->hFile);
         fclose(pChunkItem->hFile);
      }
   }
}

//----------------------------------------------------------------------------
static void clear_list()
{
   while(fwu_list_records(&gFileList) > 0)
   {
      CHUNKITEM * pChunkItem = (CHUNKITEM *)fwu_list_get(&gFileList, 0);
      if(pChunkItem->hFile)
      {
         fclose(pChunkItem->hFile);
         ::DeleteFile(pChunkItem->file_name);
      }
      free(pChunkItem);
      fwu_list_delete(&gFileList,0);
   }
}

//----------------------------------------------------------------------------
int make_output_bin_file(char * out_file_name)
{
   CString s;
   int len;
   DWORD start_addr = 0;
   DWORD file_addr = 0;

   gErrStr = _T("");
   FILE * hOutFile = fopen(out_file_name, "wb+");

   if(hOutFile == NULL)
   {
      gErrStr = _T("Could not open output file.");
      return -1;
   }

   for(int i=0; i < fwu_list_records(&gFileList); i++)
   {
      CHUNKITEM * pChunkItem = (CHUNKITEM *)fwu_list_get(&gFileList, i);

      len = 0;
      if(pChunkItem->hFile)
      {
         if(i == 0)  //-- First
            start_addr = pChunkItem->start_addr;

         fseek(pChunkItem->hFile,0,SEEK_END);
         len = ftell(pChunkItem->hFile);

         unsigned char * ptr = (unsigned char *)malloc(len);

         fseek(pChunkItem->hFile, 0, SEEK_SET);
         fread(ptr, len, 1, pChunkItem->hFile);
         fclose(pChunkItem->hFile);
         ::DeleteFile(pChunkItem->file_name);

         file_addr = pChunkItem->start_addr - start_addr;
         fseek(hOutFile, file_addr, SEEK_SET);
         fwrite(ptr, len, 1, hOutFile);
         free(ptr);
      }
   }
   fclose(hOutFile);

   //--- Clear list
   while(fwu_list_records(&gFileList) > 0)
   {
      CHUNKITEM * pChunkItem = (CHUNKITEM *)fwu_list_get(&gFileList, 0);
      free(pChunkItem);
      fwu_list_delete(&gFileList,0);
   }

   return 0; //-- OK
}

//----------------------------------------------------------------------------
void ShowErr()
{
   if(gErrStr != _T(""))
      AfxMessageBox(gErrStr, MB_ICONERROR);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
