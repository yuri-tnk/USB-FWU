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


// CHistoryComboBox.cpp : implementation file
//
#include "stdafx.h"
#include "HistoryComboBox.h"
#include "tregistry.h"
#include "Resource.h"


void  WriteStrToRegistry(const CString & VarName,const CString & StrToStore);
void  WriteComboToRegistry(CComboBox * ComboBox,const CString RegKey);
void  ReadComboFromRegistry(CComboBox * ComboBox,const CString RegKey);

// CHistoryComboBox

IMPLEMENT_DYNAMIC(CHistoryComboBox, CComboBox)
CHistoryComboBox::CHistoryComboBox()
{
   RegKey = "";
}

CHistoryComboBox::~CHistoryComboBox()
{
}


BEGIN_MESSAGE_MAP(CHistoryComboBox, CComboBox)
 ON_WM_CREATE()
 ON_WM_DESTROY()
END_MESSAGE_MAP()

// CHistoryComboBox message handlers

//-------------------------------------------------------------------------
int CHistoryComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
   if(CComboBox::OnCreate(lpCreateStruct) == -1)
      return -1;

   LoadHistory();

   return 0;
}

//-------------------------------------------------------------------------
void CHistoryComboBox::OnDestroy()
{
   SaveHistory();

   CComboBox::OnDestroy();
}

//-------------------------------------------------------------------------
BOOL CHistoryComboBox::PreTranslateMessage(MSG* pMsg)
{
   return CComboBox::PreTranslateMessage(pMsg);
}

//-------------------------------------------------------------------------
void CHistoryComboBox::HistoryChanged(void)
{
   int Index;
   CString s;

   GetWindowText(s);
   Index = FindStringExact(0,s);
   if(Index >= 0)
      DeleteString(Index);
   if(!s.IsEmpty())
      InsertString(0,s);
   SetWindowText(s);
}

//-------------------------------------------------------------------------
void CHistoryComboBox::SetRegKey(const CString Key)
{
   RegKey = Key;
}

//-------------------------------------------------------------------------
void CHistoryComboBox::SaveHistory(void)
{
   WriteComboToRegistry(this,RegKey);
}
//-------------------------------------------------------------------------
void CHistoryComboBox::LoadHistory(void)
{
   ReadComboFromRegistry(this,RegKey);
}

//----------------------------------------------------------------------------
// Utils
//----------------------------------------------------------------------------
void  WriteComboToRegistry(CComboBox * ComboBox,const CString RegKey)
{
   CString s = RegKey;
   CString t;
   int i,j;
   CStringArray sa;

   if(ComboBox == NULL)
      return;
   if(ComboBox->GetCount() <= 0)
      return;

   sa.RemoveAll();
   TRegistry *Reg = new TRegistry;

   Reg->SetRootKey(HKEY_CURRENT_USER);
   if(Reg->OpenKey(s,true))
   {
      Reg->GetValueNames(sa);
      if(sa.GetCount() > 0)
      {
         for(i = 0;i < sa.GetCount();i++)
            Reg->DeleteValue(sa.GetAt(i));
      }
      j = min(ComboBox->GetCount(),32);

      for(i=0;i<j;i++)
      {
         int n;
         CString txt;
         t.Format("Item %#2d",i);
         //---- Get text from LB --
         n = ComboBox->GetLBTextLen(i);
         ComboBox->GetLBText(i, txt.GetBuffer(n));
         //------------------------
         Reg->WriteString(t,txt);
         txt.ReleaseBuffer();
      }
      Reg->CloseKey();
   }
   delete Reg;
}

//----------------------------------------------------------------------------
void  ReadComboFromRegistry(CComboBox * ComboBox,const CString RegKey)
{
   CString s = RegKey;
   CString t;
   CString s1;
   int i;
   CStringArray sa;

   if(ComboBox == NULL)
      return;

   TRegistry * Reg = new TRegistry;
   sa.RemoveAll();

   //-- Clear Box from prev(or random) info
   while(ComboBox->GetCount() > 0)
      ComboBox->DeleteString(0);

   Reg->SetRootKey(HKEY_CURRENT_USER);
   if(Reg->OpenKey(s,false))
   {
      Reg->GetValueNames(sa);
      if(sa.GetCount() > 0)
      {
         ComboBox->ResetContent();
         for(i = 0;i < sa.GetCount();i++)
         {
            s1 = sa.GetAt(i);
            t =  Reg->ReadString(s1);
            ComboBox->AddString(t);
         }
      }
      Reg->CloseKey();
   }
   delete Reg;
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------




