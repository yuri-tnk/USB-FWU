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


#ifndef  _TREGISTRY_H_
#define  _TREGISTRY_H_

//---------------------------------------------------------------------------
struct TRegKeyInfo
{
   DWORD NumSubKeys;
   DWORD MaxSubKeyLen;
   DWORD NumValues;
   DWORD MaxValueLen;
   DWORD MaxDataLen;
   FILETIME FileTime;
};

#define rdUnknown        0
#define rdString         1
#define rdExpandString   2
#define rdInteger        3
#define rdBinary         4

struct TRegDataInfo
{
   int RegData;
   DWORD DataSize;
};

class TRegistry
{
  private:

    HKEY CurrentKey;
    HKEY RootKey;
    bool LazyWrite;
    CString CurrentPath;
    bool CloseRootKey;
    DWORD Access;

  protected:

  public:

     TRegistry();
     ~TRegistry();

     void CloseKey();
     void SetRootKey(HKEY Value);
     void ChangeKey(HKEY Value, const CString Path);
     HKEY GetBaseKey(bool Relative);
     void SetCurrentKey(HKEY Value);
     bool CreateKey(const CString Key);
     bool OpenKey(const CString Key, bool CanCreate);
     bool OpenKeyReadOnly(const CString Key);
     bool DeleteKey(const CString Key);
     bool DeleteValue(const CString Name);
     bool GetKeyInfo(TRegKeyInfo & Value);
     void GetKeyNames(CStringArray & Strings);
     void GetValueNames(CStringArray & Strings);
     bool GetDataInfo(const CString ValueName,TRegDataInfo & Value);
     int  GetDataSize(const CString ValueName);
     int  GetDataType(const CString ValueName);
     void WriteString(const CString Name, const CString Value);
     void WriteExpandString(const CString Name, const CString Value);
     CString ReadString(const CString Name);
     void WriteInteger(const CString Name, int Value);
     int  ReadInteger(const CString Name);
     void WriteBool(const CString Name, bool Value);
     bool ReadBool(const CString Name);
     void WriteFloat(const CString Name,double Value);
     double ReadFloat(const CString Name);
     void WriteBinaryData(const CString Name, void* Buffer,int BufSize);
     int  ReadBinaryData(const CString Name, void* Buffer, int BufSize);
     void PutData(const CString Name,void * Buffer,int BufSize, int RegData);
     int  GetData(const CString Name,void* Buffer,int BufSize,int & RegData);
     bool HasSubKeys();
     bool ValueExists(const CString Name);
     HKEY GetKey(const CString Key);
     bool RegistryConnect(const CString UNCName);
     bool LoadKey(const CString Key,const CString FileName);
     bool UnLoadKey(const CString Key);
     bool RestoreKey(const CString Key, const CString FileName);
     bool ReplaceKey(const CString Key, const CString FileName,
                                          const CString BackUpFileName);
     bool SaveKey(const CString Key, const CString FileName);
     bool KeyExists(const CString Key);
     void RenameValue(const CString OldName,const CString NewName);
     void MoveKey(const CString OldName, const CString NewName,bool Delete);

     void MoveValue(HKEY SrcKey, HKEY DestKey,const CString Name);
     void CopyValues(HKEY SrcKey, HKEY DestKey);
     void CopyKeys(HKEY SrcKey, HKEY DestKey);
};

#endif




