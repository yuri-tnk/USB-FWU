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

#ifndef _FWU_UTILS_H_
#define _FWU_UTILS_H_

#include "fwu_types.h"
#include "HistoryComboBox.h"


DWORD calc_crc(unsigned char * buf,int nbytes);
DWORD calc_file_crc(FILE * hFile);

 void UpdatePipeStatistics();
  int ErrProc(CString & s_err,DWORD rc);
DWORD WaitDevResponse(DWORD Timeout);
 void DoSetConfiguration(void);
 void OpenDevice(void);
 BOOL IsFWU(void);
 void EnableItem(GLODATA * pGD, int item, BOOL fEnable);
 void CreateHisoryComboBox(CWnd * pWnd, CHistoryComboBox * pHComboBox,
                          CString & RegistryPath,
                          int placeholder_id, int historycombobox_id);
 void GetFileNameByFileDlg(CWnd * pWnd, CHistoryComboBox * pCombo, int mode);

 BOOL LoadHexFile(CString & hex_path,      //-- [IN]
                 int * filelen);          //-- [OUT]
DWORD wait_for_flag(BOOL & flag,int timeout);

#endif
