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
#include "resource.h"
#include "HistoryComboBox.h"
#include "fwu_cmd.h"
#include "fwu_hex_processing.h"

#include "fwu_types.h"
#include "fwu_globals.h"

#include "fwu_utils.h"

#pragma warning(disable: 4996)

//----------------------------------------------------------------------------

const static unsigned __int32 crc32_ref_table[] =
{
   0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,
   0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,
   0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,
   0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
   0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,
   0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,
   0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
   0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
   0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,
   0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,
   0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,0x01DB7106,
   0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
   0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,
   0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
   0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,
   0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
   0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,
   0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,
   0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,
   0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
   0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
   0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,
   0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,
   0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
   0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,
   0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,
   0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,0xA1D1937E,
   0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
   0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,
   0x316E8EEF,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,
   0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,
   0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
   0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,
   0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,
   0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
   0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
   0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,
   0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,
   0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,
   0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
   0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,
   0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
   0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
};

//----------------------------------------------------------------------------
void UpdatePipeStatistics()
{
   USBIO_PIPE_STATISTICS stats;
   CString s;
   CStatic * pStatic;
   static unsigned __int64 prev_bytes = ~0;
   static DWORD prev_val = 0;

   DWORD err = g_RxPipe.QueryPipeStatistics(&stats);
   if(err != USBIO_ERR_SUCCESS)  //-- failed
      return;
   int rate = stats.AverageRate;
   if(prev_val != rate)
   {
      pStatic = (CStatic*)g_dt.RegOpDlg->GetDlgItem(IDC_STATIC_RATE_REGOP);
      s.Format("%d",rate);
      if(pStatic)
         pStatic->SetWindowText(s);
   }
   prev_val = rate;

   unsigned __int64 n = stats.BytesTransferred_H;
   n <<= 32;
   n += stats.BytesTransferred_L;
   if(prev_bytes != n)
   {
      pStatic = (CStatic*)g_dt.RegOpDlg->GetDlgItem(IDC_STATIC_NUM_RXBYTES_REGOP);
      s.Format("%I64u",n);
      if(pStatic)
         pStatic->SetWindowText(s);
   }
   prev_bytes = n;
}

//----------------------------------------------------------------------------
int ErrProc(CString & s_err, DWORD rc)
{
   int pi = 0;

 //--- Err processing
   if(rc == ERR_RXDATA_OK) //--  No Error
   {
      pi = IDI_ICON_OK;
      s_err = _T("Done.");
   }
   else if(rc == ERR_UNKNOWN_ERR)
   {
      pi = IDI_ICON_ERR;
      s_err = _T("Internal error.");
   }
   else if(rc == ERR_RXDATA_BADCRC)   //-- Rx from Device - badCrc
   {
      pi = IDI_ICON_ERR;
      s_err = _T("Bad CRC - command");
   }
   else if(rc == ERR_TIMEOUT)       //-- No answer from Device
   {
      pi = IDI_ICON_ERR;
      s_err = _T("Timeout expiried.");
   }
   else if(rc == ERR_USER_ABORT)       //-- Canceled by user
   {
      pi = IDI_ICON_WARN;
      s_err = _T("Canceled by user.");
   }
   else if(rc == ERR_WIN_INTERNAL)
   {
      pi = IDI_ICON_ERR;
      s_err = _T("Windows error.");
   }
   else if(rc = ERR_WR_ERR)
   {
      pi = IDI_ICON_ERR;
      s_err = _T("Firmware write error.");
   }
   else
   {
      pi = IDI_ICON_ERR;
      s_err = _T("Sanity checking - internal error.");
   }

   return pi;
}

//----------------------------------------------------------------------------
DWORD WaitDevResponse(DWORD Timeout)
{
   DWORD h;
   DWORD del;
   MSG msg;
   DWORD rc = ERR_UNKNOWN_ERR;
   HANDLE TxEvents[2];
   DWORD tmp;

   TxEvents[0] = g_dt.EventRxData;
   TxEvents[1] = g_dt.EventUserAbort;

   h = ::GetTickCount();
   for(;;)
   {
      while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
      {
         if(!AfxGetThread()->PumpMessage())
         {
            ASSERT(AfxGetCurrentMessage()->message == WM_QUIT);
            AfxPostQuitMessage(AfxGetCurrentMessage()->wParam);
            return ERR_WIN_INTERNAL; //-- Never reached
         }
      }

      rc = WaitForMultipleObjects(2,TxEvents,FALSE,1);
      switch (rc)
      {
         case WAIT_OBJECT_0:       //-- Rx from Device

            //-- Get received data (with CrSec)
            EnterCriticalSection(&g_dt.CrSecRxDataExch);
            memcpy(g_dt.m_RxBuf,g_dt.CtrlRxExch,CTRL_BUF_SIZE);
            LeaveCriticalSection(&g_dt.CrSecRxDataExch);

            rc = calc_crc(&g_dt.m_RxBuf[0],CTRL_BUF_CRC_OFF); //-- 60
            tmp =  *((DWORD*)(&g_dt.m_RxBuf[CTRL_BUF_CRC_OFF]));
            if(rc == tmp) //-- crc o.k.
               return ERR_RXDATA_OK;
            else
               return  ERR_RXDATA_BADCRC;
            break;


         case (WAIT_OBJECT_0+1):   //-- User Abort

            cmd_no_data(USER_ABORT);
            ::Sleep(30);

            return ERR_USER_ABORT;

            break;

         case WAIT_TIMEOUT:
            break;

         default:

            return ERR_WIN_INTERNAL;
            break;
      }


      DWORD q = ::GetTickCount();
      if(q < h)
         del = q +(0xFFFFFFFF - h);
      else
         del = q-h;
      if(del >= (DWORD)Timeout)
         break;
   }
   return  ERR_TIMEOUT;
}

//---------------------------------------------------------------------------
void OpenDevice(void)
{
   DWORD err;

   char ErrBuffer[256];
   char Buffer[256];

   if(g_dt.UsbIo.IsOpen())
   {
      AfxMessageBox("A device is already opened.");
           return;
   }

   g_dt.DeviceNumber = 0;
   err = g_dt.UsbIo.Open(g_dt.DeviceNumber, g_dt.DevList, &g_dt.UsbioID);
   if(err)
   {
      _snprintf(Buffer,sizeof(Buffer),"%s", CUsbIo::ErrorText(ErrBuffer,sizeof(ErrBuffer),err));
 //      AfxMessageBox(Buffer);
   }
}


//----------------------------------------------------------------------------
BOOL IsFWU(void)
{
   USBIO_CLASS_OR_VENDOR_REQUEST  Request;
   DWORD ByteCount;
   char mbuf[64];
   DWORD id;

   memset(mbuf,0,64);
   ByteCount = 8;
   Request.Type = RequestTypeVendor;
   Request.Recipient = RecipientDevice;
   Request.Request   = GET_ISFWU;

   g_dt.UsbIo.ClassOrVendorInRequest(mbuf, //-- No data phase
                                  ByteCount,
                                  &Request);
   memcpy(&id, mbuf, 4); //-- LE
   if(id == GET_ISFWU_ASK) //--boot loader
      return TRUE;

   return FALSE;
}

//---------------------------------------------------------------------------
void DoSetConfiguration(void)
{
   USBIO_SET_CONFIGURATION SetConfig;
   USBIO_INTERFACE_SETTING *Setting;
   DWORD err;

   ZeroMemory(&SetConfig,sizeof(SetConfig));
   SetConfig.ConfigurationIndex = (USHORT)0;

   Setting = &SetConfig.InterfaceList[0];
   Setting->InterfaceIndex = 0;
   Setting->AlternateSettingIndex = 0;
   Setting->MaximumTransferSize = 65536;

   SetConfig.NbOfInterfaces = 1;

   // send command to driver
   err = g_dt.UsbIo.SetConfiguration(&SetConfig);
   if(err)
   {
      char ErrBuffer[256];
      char Buffer[256];

      if(err != 0)
      {
         _snprintf(Buffer,sizeof(Buffer),"%s", CUsbIo::ErrorText(ErrBuffer,sizeof(ErrBuffer),err));
      }
   }
}

//----------------------------------------------------------------------------
void  EnableItem(GLODATA * pGD, int item, BOOL fEnable)
{
   int item_id = 0;

   CHistoryComboBox * pCombo;
   CButton * pBtn;

   switch(item)
   {
      case BTN_START:

         pBtn = (CButton *)pGD->WrDlg->GetDlgItem(IDC_BUTTON_START_WRITEFW);
         pBtn->EnableWindow(fEnable);
         pBtn = (CButton *)pGD->RdDlg->GetDlgItem(IDC_BUTTON_START_READFW);
         pBtn->EnableWindow(fEnable);
         pBtn = (CButton *)pGD->RegOpDlg->GetDlgItem(IDC_BUTTON_START_REGOP);
         pBtn->EnableWindow(fEnable);
         break;

      case BTN_STOP:

         pBtn = (CButton *)pGD->WrDlg->GetDlgItem(IDC_BUTTON_STOP_WRITEFW);
         pBtn->EnableWindow(fEnable);
         pBtn = (CButton *)pGD->RdDlg->GetDlgItem(IDC_BUTTON_STOP_READFW);
         pBtn->EnableWindow(fEnable);
         pBtn = (CButton *)pGD->RegOpDlg->GetDlgItem(IDC_BUTTON_STOP_REGOP);
         pBtn->EnableWindow(fEnable);
         break;

      case BTN_FILE:

         pBtn = (CButton *)pGD->WrDlg->GetDlgItem(IDC_BUTTON_FILE_WRITEFW);
         pBtn->EnableWindow(fEnable);
         pBtn = (CButton *)pGD->RdDlg->GetDlgItem(IDC_BUTTON_FILE_READFW);
         pBtn->EnableWindow(fEnable);
         pBtn = (CButton *)pGD->RegOpDlg->GetDlgItem(IDC_BUTTON_FILE_REGOP);
         pBtn->EnableWindow(fEnable);
         break;

      case COMBO_FILE:

         pCombo =  (CHistoryComboBox *)pGD->WrDlg->GetDlgItem(IDC_HCOMBO_WRITEFW);
         pCombo->EnableWindow(fEnable);
         pCombo =  (CHistoryComboBox *)pGD->RdDlg->GetDlgItem(IDC_HCOMBO_READFW);
         pCombo->EnableWindow(fEnable);
         pCombo =  (CHistoryComboBox *)pGD->RegOpDlg->GetDlgItem(IDC_HCOMBO_REGOP);
         pCombo->EnableWindow(fEnable);
         break;
   }
}

//----------------------------------------------------------------------------
void CreateHisoryComboBox(CWnd * pWnd, CHistoryComboBox * pHComboBox,
                          CString & RegistryPath,
                          int placeholder_id, int historycombobox_id)

{
   CComboBox * pCombo = (CComboBox*)pWnd->GetDlgItem(placeholder_id);
   CRect r;
   LOGFONT	lf;
 

   pCombo->GetWindowRect(&r);
   pWnd->ScreenToClient(&r);

   pHComboBox->Create(WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | CBS_DROPDOWN |
                         CBS_AUTOHSCROLL | WS_TABSTOP,
                         r, pWnd, historycombobox_id);

   //pHComboBox->m_font.CreatePointFont(8, "MS Sans Serif", NULL);

   ::GetObject((HFONT)GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
   pHComboBox->m_font.CreateFontIndirect(&lf);

   ::SendMessage(pHComboBox->GetSafeHwnd(),
           WM_SETFONT,(WPARAM)((HFONT)pHComboBox->m_font),(LPARAM)FALSE);

   pHComboBox->SetRegKey(RegistryPath);
   pHComboBox->LoadHistory();
}

//----------------------------------------------------------------------------
DWORD calc_crc(unsigned char * buf,int nbytes)
{
   DWORD result;
   int i;

   result = 0xffffffff;
   for(i=0; i < nbytes; i++)
      result = (result >> 8) ^ crc32_ref_table[(result & 0xFF) ^ buf[i]];
   result ^= 0xffffffff;

   return result;
}
//----------------------------------------------------------------------------
DWORD calc_file_crc(FILE * hFile)
{
   DWORD result;
   int i;
   int file_len;

   fseek(hFile,0,SEEK_END);
   file_len = ftell(hFile);

   fseek(hFile,FW_START_OFFSET,SEEK_SET);
   //rewind(hFile);

   result = 0xffffffff;
   for(i=0; i < file_len - FW_START_OFFSET; i++)
      result = (result >> 8) ^ crc32_ref_table[(result & 0xFF) ^ (unsigned char) fgetc(hFile)];
   result ^= 0xffffffff;

   return result;
}

//----------------------------------------------------------------------------
DWORD wait_for_flag(BOOL & flag, int timeout)
{
   DWORD h;
   DWORD del;
   MSG msg;
   h = ::GetTickCount();
   for(;;)
   {
      while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
      {
         if(!AfxGetThread()->PumpMessage())
         {
            ASSERT(AfxGetCurrentMessage()->message == WM_QUIT);
            AfxPostQuitMessage(AfxGetCurrentMessage()->wParam);
            return WAIT_TIMEOUT; //-- Never reached
         }
      }
      ::Sleep(10);

      if(flag == TRUE)
         return WAIT_OBJECT_0;
      DWORD q = ::GetTickCount();
      if(q < h)
         del = q +(0xFFFFFFFF - h);
      else
         del = q-h;
      if(del >= (DWORD)timeout)
         break;
   }
   return  WAIT_TIMEOUT;
}


//----------------------------------------------------------------------------
BOOL LoadHexFile(CString & hex_path, //-- [IN]
                 int * filelen)      //-- [OUT]
{
   CString tmpFileName;
   int rc;

   tmpFileName = hex_path + ".tmp";

   rc = proc_hex_file((char*)((LPCSTR)hex_path));
   if(rc != 0)
   {
      ShowErr();
      return FALSE;
   }
   else
   {
      rc = make_output_bin_file((char*)((LPCSTR)tmpFileName));
      if(rc != 0)
      {
         ShowErr();
         return FALSE;
      }
   }

   FILE * hInFile = fopen((LPCSTR)tmpFileName,"rb");
   if(hInFile == NULL)
   {
      CString s;
      s.Format("Could not open bin file: \n %s", hex_path);
      AfxMessageBox(s);
      return FALSE;
   }
   fseek(hInFile,0,SEEK_END);
   int total = ftell(hInFile);
   rewind(hInFile);
   if(total > 0)
   {
      fread(g_dt.FWBuf, total, 1, hInFile);
   }

   fclose(hInFile);
   ::DeleteFile((LPCTSTR)tmpFileName);

   if(total > 0)
   {
      *filelen = total;
      return TRUE;
   }
   return FALSE;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

