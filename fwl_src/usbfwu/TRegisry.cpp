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

//-- This is a Visual C++ version of the Borland's Delphi TRegistry type

#include "stdafx.h"
#include <atlsimpstr.h>
#include "tregistry.h"

//---------------------------------------------------------------------------
bool IsRelative(const CString Value)
{
   bool Result = false;
   if((Value != "") && (Value[0] == '\\'))
      Result = true;
   return Result;
}

//---------------------------------------------------------------------------
int RegDataToDataType(int Value)
{
   int Result = REG_NONE;
   switch(Value)
   {
      case rdString:
         Result = REG_SZ;
         break;
      case rdExpandString:
         Result = REG_EXPAND_SZ;
         break;
      case rdInteger:
         Result = REG_DWORD;
         break;
      case rdBinary:
         Result = REG_BINARY;
         break;
      default:
         Result = REG_NONE;
   }
   return Result;
}

//---------------------------------------------------------------------------
int DataTypeToRegData(int Value)
{
   int Result;
   if(Value == REG_SZ)
      Result = rdString;
   else if(Value == REG_EXPAND_SZ)
      Result = rdExpandString;
   else if(Value == REG_DWORD)
      Result = rdInteger;
   else if(Value == REG_BINARY)
      Result = rdBinary;
   else
      Result = rdUnknown;
   return Result;
}

//---------------------------------------------------------------------------
TRegistry::TRegistry()
{
   RootKey = HKEY_CURRENT_USER;
   Access = KEY_ALL_ACCESS;
   LazyWrite = true;
   CurrentKey = 0;
}
//---------------------------------------------------------------------------
TRegistry::~TRegistry()
{
  CloseKey();
}
//---------------------------------------------------------------------------
void TRegistry::CloseKey()
{
   int res;
   if(CurrentKey != 0)
   {
      if(LazyWrite)
      {
         res = ::RegCloseKey(CurrentKey);
         if(res != ERROR_SUCCESS)
            AfxMessageBox("RegCloseKey");
      }
      else
      {
         res = ::RegFlushKey(CurrentKey);
         if(res != ERROR_SUCCESS)
           AfxMessageBox("RegFlushKey");
      }
      CurrentKey = 0;
      CurrentPath = "";
   }
}

//---------------------------------------------------------------------------
void TRegistry::SetRootKey(HKEY Value)
{
   if(RootKey != Value)
   {
          if(CloseRootKey)
          {
                 ::RegCloseKey(RootKey);
                 CloseRootKey = false;
          }
          RootKey = Value;
          CloseKey();
   }
}

//---------------------------------------------------------------------------
void TRegistry::ChangeKey(HKEY Value, const CString Path)
{
   CloseKey();
   CurrentKey = Value;
   CurrentPath = Path;
}

//---------------------------------------------------------------------------
HKEY TRegistry::GetBaseKey(bool Relative)
{
   if(CurrentKey == 0 || !Relative)
      return RootKey;
   return CurrentKey;
}

//---------------------------------------------------------------------------
void TRegistry::SetCurrentKey(HKEY Value)
{
   CurrentKey = Value;
}

//---------------------------------------------------------------------------
bool TRegistry::CreateKey(const CString Key)
{
   HKEY TempKey;
   CString s;
   DWORD Disposition;
   bool Relative;
   bool Result;

   TempKey = 0;
   s = Key;
   Relative = IsRelative(s);
   if(Relative)
      s.Delete(0,1);
   Result = (::RegCreateKeyEx(GetBaseKey(Relative),(LPCSTR)s, 0, NULL,
      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &TempKey, &Disposition) == ERROR_SUCCESS);
   if(Result)
      ::RegCloseKey(TempKey);
   else
      ::MessageBox(NULL,"RegCreateFailed","Error",MB_ICONERROR | MB_OK);
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::OpenKey(const CString Key, bool CanCreate)
{
   HKEY TempKey;
   CString s;
   DWORD Disposition;
   bool Relative;
   bool Result;

   s = Key;
   Relative = IsRelative(s);

   if(Relative)
      s.Delete(0, 1);
   TempKey = 0;
   if(!CanCreate || s.IsEmpty())
   {
      Result = (::RegOpenKeyEx(GetBaseKey(Relative),(LPCSTR)s, 0,
         Access, &TempKey) == ERROR_SUCCESS);
   }
   else
      Result = (::RegCreateKeyEx(GetBaseKey(Relative),(LPCSTR)s, 0,NULL,
         REG_OPTION_NON_VOLATILE, Access,NULL, &TempKey, &Disposition) == ERROR_SUCCESS);

   if(Result)
   {
      if(CurrentKey != 0 && Relative)
         s = CurrentPath + "\\" + s;
      ChangeKey(TempKey, s);
   }
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::OpenKeyReadOnly(const CString Key)
{
   HKEY TempKey;
   CString s;
   bool Relative;
   bool Result;

   s = Key;
   Relative = IsRelative(s);

   if(Relative)
      s.Delete(0, 1);
   TempKey = 0;
   Result = (::RegOpenKeyEx(GetBaseKey(Relative),(LPCSTR)s, 0,
      KEY_READ, &TempKey) == ERROR_SUCCESS);
   if(Result)
   {
      Access = KEY_READ;
      if(CurrentKey != 0 && Relative)
         s = CurrentPath + "\\" + s;
      ChangeKey(TempKey, s);
   }
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::DeleteKey(const CString Key)
{
   DWORD Len;
   int i;
   bool Relative;
   CString s;
   CString KeyName;
   HKEY OldKey, DeleteKey;
   TRegKeyInfo Info;
   bool Result;

   s = Key;
   Relative = IsRelative(s);
   if(Relative)
      s.Delete(0,1);
   OldKey = CurrentKey;
   DeleteKey = GetKey(Key);
   if(DeleteKey != 0)
   {
      SetCurrentKey(DeleteKey);
      if(GetKeyInfo(Info))
      {
         KeyName.Preallocate(Info.MaxSubKeyLen + 1);
         for(i = Info.NumSubKeys - 1;i>=0;i--)
         {
            Len = Info.MaxSubKeyLen + 1;
            if(::RegEnumKeyEx(DeleteKey,(DWORD)i,(LPTSTR)((LPCSTR)KeyName), &Len,NULL,NULL,NULL,NULL) == ERROR_SUCCESS)
               this->DeleteKey((LPCSTR)KeyName);
         }
      }
      SetCurrentKey(OldKey);
      RegCloseKey(DeleteKey);
   }
   Result = (::RegDeleteKey(GetBaseKey(Relative),(LPCSTR)s) == ERROR_SUCCESS);
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::DeleteValue(const CString Name)
{
   bool Result = ::RegDeleteValue(CurrentKey,(LPCSTR)Name) == ERROR_SUCCESS;
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::GetKeyInfo(TRegKeyInfo & Value)
{
   bool Result;
   memset(&Value,0, sizeof(TRegKeyInfo));
   Result = (::RegQueryInfoKey(CurrentKey,NULL,NULL,NULL, &Value.NumSubKeys,
     &Value.MaxSubKeyLen,NULL,&Value.NumValues,&Value.MaxValueLen,
     &Value.MaxDataLen, NULL, &Value.FileTime) == ERROR_SUCCESS);

   return Result;
}

//---------------------------------------------------------------------------
void TRegistry::GetKeyNames(CStringArray & Strings)
{
   DWORD Len;
   DWORD i;
   TRegKeyInfo Info;
   CString s;

   Strings.RemoveAll();
   if(GetKeyInfo(Info))
   {
      s.Preallocate(Info.MaxSubKeyLen + 1);
      for(i = 0; i < Info.NumSubKeys; i++)
      {
         Len = Info.MaxSubKeyLen + 1;
         ::RegEnumKeyEx(CurrentKey,i,(LPTSTR)((LPCSTR)s),&Len,NULL,NULL,NULL,NULL);
         Strings.Add((LPCSTR)s);
      }
   }
}

//---------------------------------------------------------------------------
void TRegistry::GetValueNames(CStringArray & Strings)
{
   DWORD Len;
   DWORD i;
   TRegKeyInfo Info;
   CString s;

   Strings.RemoveAll();
   if(GetKeyInfo(Info))
   {
          s.Preallocate(Info.MaxValueLen + 1);
          for(i = 0;i< Info.NumValues;i++)
          {
                 Len = Info.MaxValueLen + 1;
                 ::RegEnumValue(CurrentKey,i,(LPTSTR)((LPCSTR)s),&Len,NULL,NULL,NULL,NULL);
                 Strings.Add((LPCSTR)s);
          }
   }
}
//---------------------------------------------------------------------------
bool TRegistry::GetDataInfo(const CString ValueName,TRegDataInfo & Value)
{
   DWORD DataType;

   memset(&Value,0, sizeof(TRegDataInfo));
   bool Result = (::RegQueryValueEx(CurrentKey,(LPCSTR)ValueName,NULL,
         &DataType,NULL,&Value.DataSize) == ERROR_SUCCESS);
   Value.RegData = DataTypeToRegData(DataType);
   return Result;
}

//---------------------------------------------------------------------------
int TRegistry::GetDataSize(const CString ValueName)
{
   TRegDataInfo Info;

   if(GetDataInfo(ValueName,Info))
      return Info.DataSize;
   return (-1);
}
//---------------------------------------------------------------------------
int TRegistry::GetDataType(const CString ValueName)
{
   TRegDataInfo Info;
   int Result;

   if(GetDataInfo(ValueName,Info))
      Result = Info.RegData;
   else
      Result = rdUnknown;
   return Result;
}

//---------------------------------------------------------------------------
void TRegistry::WriteString(const CString Name, const CString Value)
{
   PutData(Name,(void*)((LPCSTR)Value),lstrlen((LPCSTR)Value)+1,rdString);
}

//---------------------------------------------------------------------------
void TRegistry::WriteExpandString(const CString Name, const CString Value)
{
   PutData(Name,(void*)((LPCSTR)Value),lstrlen((LPCSTR)Value)+1,rdExpandString);
}

//---------------------------------------------------------------------------
CString  TRegistry::ReadString(const CString Name)
{
   int Len;
   int RegData;
   CString Result;

   Len = GetDataSize(Name);
   if(Len > 0)
   {
          Result.Preallocate(Len);
          GetData(Name,Result.GetBuffer(),Len, RegData);
          if(RegData == rdString || RegData == rdExpandString)
          {
                 Result = CString((LPCSTR)Result,Len);
          }
          else
                 AfxMessageBox(Name);
   }
   else
          Result = "";
   return Result;

}
//---------------------------------------------------------------------------
void  TRegistry::WriteInteger(const CString Name, int Value)
{
   PutData(Name, &Value, sizeof(int), rdInteger);
}

//---------------------------------------------------------------------------
int TRegistry::ReadInteger(const CString Name)
{
   int RegData;
   int Result;

   GetData(Name, &Result, sizeof(int), RegData);
   if(RegData != rdInteger)
      AfxMessageBox(Name);
   return Result;
}

//---------------------------------------------------------------------------
void TRegistry::WriteBool(const CString Name, bool Value)
{
   WriteInteger(Name,(int)Value);
}

//---------------------------------------------------------------------------
bool TRegistry::ReadBool(const CString Name)
{
   return (ReadInteger(Name) != 0);
}

//---------------------------------------------------------------------------
void TRegistry::WriteFloat(const CString Name,double Value)
{
   PutData(Name, &Value, sizeof(double), rdBinary);
}

//---------------------------------------------------------------------------
double TRegistry::ReadFloat(const CString Name)
{
   int Len;
   int RegData;
   double Result;

   Len = GetData(Name, &Result, sizeof(double),RegData);
   if(RegData != rdBinary || Len != sizeof(double))
      AfxMessageBox(Name);

   return Result;
}

//---------------------------------------------------------------------------
void TRegistry::WriteBinaryData(const CString Name, void * Buffer,int BufSize)
{
   PutData(Name, Buffer, BufSize, rdBinary);
}

//---------------------------------------------------------------------------
int TRegistry::ReadBinaryData(const CString Name, void * Buffer, int BufSize)
{
   int RegData;
   TRegDataInfo Info;
   int Result;

   if(GetDataInfo(Name, Info))
   {
      Result = Info.DataSize;
      RegData = Info.RegData;
      if((RegData == rdBinary || RegData == rdUnknown) && (Result <= BufSize))
         GetData(Name, Buffer, Result, RegData);
      else
         AfxMessageBox(Name);
   }
   else
      Result = 0;
   return Result;
}

//---------------------------------------------------------------------------
void TRegistry::PutData(const CString Name,void * Buffer,int BufSize, int RegData)
{
   int  DataType;

   DataType = ::RegDataToDataType(RegData);
   if(::RegSetValueEx(CurrentKey,(LPCSTR)Name, 0, DataType,(const BYTE *)Buffer,
       BufSize) != ERROR_SUCCESS)
   {
      ::MessageBox(NULL,"RegSetDataFailed","Error",MB_ICONERROR | MB_OK);
   }
}

//---------------------------------------------------------------------------
int TRegistry::GetData(const CString Name,void* Buffer,int BufSize,int & RegData)
{
   DWORD DataType;
   int Result;
   DataType = REG_NONE;
   if(::RegQueryValueEx(CurrentKey, (LPCSTR)Name,NULL,&DataType,(LPBYTE)Buffer,
    (DWORD*)&BufSize) != ERROR_SUCCESS)
   {
     ::MessageBox(NULL,"RegGetDataFailed","Error",MB_ICONERROR | MB_OK);
   }
   Result = BufSize;
   RegData = DataTypeToRegData(DataType);
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::HasSubKeys()
{
   TRegKeyInfo Info;

   bool Result = (GetKeyInfo(Info) && (Info.NumSubKeys > 0));
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::ValueExists(const CString Name)
{
   TRegDataInfo Info;
   bool Result;

   Result = GetDataInfo(Name,Info);
   return Result;
}

//---------------------------------------------------------------------------
HKEY TRegistry::GetKey(const CString Key)
{
   CString s;
   bool Relative;
   HKEY Result;

   s = Key;
   Relative = IsRelative(s);
   if(Relative)
     s.Delete(0, 1);
   Result = 0;
   ::RegOpenKeyEx(GetBaseKey(Relative), (LPCSTR)s, 0, Access, &Result);
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::RegistryConnect(const CString UNCName)
{
   HKEY TempKey;
   bool Result;

   Result = (::RegConnectRegistry((LPCSTR)UNCName,RootKey,&TempKey) == ERROR_SUCCESS);
   if(Result)
   {
      RootKey = TempKey;
      CloseRootKey = true;
   }
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::LoadKey(const CString Key,const CString FileName)
{
   CString s;
   bool Result;

   s = Key;
   if(IsRelative(s))
      s.Delete(0, 1);
   Result = (::RegLoadKey(RootKey,(LPCSTR)s,(LPCSTR)FileName) == ERROR_SUCCESS);
   return Result;
}
//---------------------------------------------------------------------------
bool TRegistry::UnLoadKey(const CString Key)
{
   CString s;
   bool Result;

   s = Key;
   if(IsRelative(s))
      s.Delete(0,1);
   Result = (::RegUnLoadKey(RootKey,(LPCSTR)s) == ERROR_SUCCESS);
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::RestoreKey(const CString Key, const CString FileName)
{
   HKEY RestoreKey;
   bool Result;

   Result = false;
   RestoreKey = GetKey(Key);
   if(RestoreKey != 0)
   {
      Result = (::RegRestoreKey(RestoreKey, (LPCSTR)FileName, 0) == ERROR_SUCCESS);
      ::RegCloseKey(RestoreKey);
   }
   return Result;
}
//---------------------------------------------------------------------------
bool TRegistry::ReplaceKey(const CString Key, const CString FileName,
                                                  const CString BackUpFileName)
{
   CString s;
   bool Result;
   bool Relative;

   s = Key;
   Relative = IsRelative(s);
   if(Relative)
      s.Delete(0, 1);
   Result = (::RegReplaceKey(GetBaseKey(Relative), (LPCSTR)s,
        (LPCSTR)FileName, (LPCSTR)BackUpFileName) == ERROR_SUCCESS);

   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::SaveKey(const CString Key, const CString FileName)
{
   HKEY SaveKey;
   bool Result;

   Result = false;
   SaveKey = GetKey(Key);
   if(SaveKey != 0)
   {
      Result = (::RegSaveKey(SaveKey,(LPCSTR)FileName,NULL) == ERROR_SUCCESS);
      ::RegCloseKey(SaveKey);
   }
   return Result;
}

//---------------------------------------------------------------------------
bool TRegistry::KeyExists(const CString Key)
{
   HKEY TempKey;
   bool Result;

   TempKey = GetKey(Key);
   if(TempKey != 0)
      ::RegCloseKey(TempKey);
   Result = (TempKey != 0);

   return Result;
}
//---------------------------------------------------------------------------
void TRegistry::RenameValue(const CString OldName,const CString NewName)
{
  int Len;
  int RegData;
  char * Buffer;

  if(ValueExists(OldName) && !ValueExists(NewName))
  {
         Len = GetDataSize(OldName);
         if(Len > 0)
         {
                Buffer = (char*)malloc(Len);
                Len = GetData(OldName, Buffer, Len, RegData);
                DeleteValue(OldName);
                PutData(NewName, Buffer, Len, RegData);
                free(Buffer);
         }
  }
}
//---------------------------------------------------------------------------
void TRegistry::MoveKey(const CString OldName, const CString NewName,bool Delete)
{
   HKEY SrcKey;
   HKEY DestKey;

   if(KeyExists(OldName) && !KeyExists(NewName))
   {
      SrcKey = GetKey(OldName);
      if(SrcKey != 0)
      {
         CreateKey(NewName);
         DestKey = GetKey(NewName);
         if(DestKey != 0)
         {
            CopyValues(SrcKey, DestKey);
            CopyKeys(SrcKey, DestKey);
            if(Delete)
               DeleteKey(OldName);
            RegCloseKey(DestKey);
         }
         RegCloseKey(SrcKey);
      }
   }
}

//-------- Private ----------------------------------------------------------
void TRegistry::MoveValue(HKEY SrcKey, HKEY DestKey,const CString Name)
{
   int Len;
   HKEY OldKey;
   HKEY PrevKey;
   char * Buffer;
   int RegData;

   OldKey = CurrentKey;
   SetCurrentKey(SrcKey);
   Len = GetDataSize(Name);
   if(Len > 0)
   {
      Buffer = (char*)malloc(Len);
      Len = GetData(Name, Buffer, Len, RegData);
      PrevKey = CurrentKey;
      SetCurrentKey(DestKey);
      PutData(Name, Buffer, Len, RegData);
      SetCurrentKey(PrevKey);
      free(Buffer);
   }
   SetCurrentKey(OldKey);
}
//-------------------------------------------------------------------
void TRegistry::CopyValues(HKEY SrcKey, HKEY DestKey)
{
  DWORD Len;
  DWORD i;
  TRegKeyInfo  KeyInfo;
  CString s;
  HKEY OldKey;

  OldKey = CurrentKey;
  SetCurrentKey(SrcKey);
  if(GetKeyInfo(KeyInfo))
  {
     MoveValue(SrcKey,DestKey,"");
     s.Preallocate(KeyInfo.MaxValueLen + 1);
     for(i = 0;i < KeyInfo.NumValues;i++)
     {
        Len = KeyInfo.MaxValueLen + 1;
        if(::RegEnumValue(SrcKey,i,(LPTSTR)(PCTSTR(s)),&Len,NULL,NULL,NULL,NULL) == ERROR_SUCCESS)
           MoveValue(SrcKey,DestKey,(LPTSTR)(PCTSTR(s)));
     }
  }
  SetCurrentKey(OldKey);
}

//-------------------------------------------------------------------
void TRegistry::CopyKeys(HKEY SrcKey, HKEY DestKey)
{
   DWORD Len;
   DWORD i;
   TRegKeyInfo Info;
   CString s;
   HKEY OldKey, PrevKey, NewSrc, NewDest;

   OldKey = CurrentKey;
   SetCurrentKey(SrcKey);
   if(GetKeyInfo(Info))
   {
      s.Preallocate(Info.MaxSubKeyLen + 1);
      for(i = 0; i< Info.NumSubKeys;i++)
      {
         Len = Info.MaxSubKeyLen + 1;
         if(::RegEnumKeyEx(SrcKey,i,(LPTSTR)(PCTSTR(s)),&Len,NULL,NULL,NULL,NULL) == ERROR_SUCCESS)
         {
            NewSrc = GetKey((LPCSTR)s);
            if(NewSrc != 0)
            {
               PrevKey = CurrentKey;
               SetCurrentKey(DestKey);
                  CreateKey((LPCSTR)s);
                  NewDest = GetKey((LPCSTR)s);
                     CopyValues(NewSrc, NewDest);
                     CopyKeys(NewSrc, NewDest);
                     RegCloseKey(NewDest);
                  SetCurrentKey(PrevKey);
              RegCloseKey(NewSrc);
            }
         }
      }
   }
   SetCurrentKey(OldKey);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------






























