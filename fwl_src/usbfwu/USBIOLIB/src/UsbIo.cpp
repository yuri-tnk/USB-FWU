/************************************************************************
 *
 *  Module:       UsbIo.cpp
 *  Long name:    CUsbIo class
 *  Description:  CUsbIo base device class implementation
 *
 *  Runtime Env.: Win32, Part of UsbioLib
 *  Author(s):    Guenter Hildebrandt, Udo Eberhardt
 *  Company:      Thesycon GmbH, Ilmenau
 ************************************************************************/

// for shorter and faster windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
// unicode is not supported by USBIOLIB
#ifdef UNICODE
#undef UNICODE
#endif

#pragma warning(disable: 4996)

#include <windows.h>
#include <stdio.h>
#include "usbio.h"


// static members
CSetupApiDll CUsbIo::smSetupApi;



// standard constructor
CUsbIo::CUsbIo()
{
  FileHandle = NULL;
  ZeroMemory(&Overlapped,sizeof(Overlapped));
  InitializeCriticalSection(&CritSect);
  CheckedBuildDetected = FALSE;
  DemoVersionDetected = FALSE;
  LightVersionDetected = FALSE;
  mDevDetail = NULL;
}


// destructor
CUsbIo::~CUsbIo()
{
  // close file handle
  Close();
  // free resources
  DeleteCriticalSection(&CritSect);

}



//static
HDEVINFO 
USBIOLIB_CALL
CUsbIo::CreateDeviceList(const GUID *InterfaceGuid)
{
  HDEVINFO h;

  // make sure the setupapi dll is loaded
  if ( !smSetupApi.Load() ) {
    return NULL;
  }

  h = (smSetupApi.SetupDiGetClassDevs)(
        (GUID*)InterfaceGuid,                 // LPGUID ClassGuid, 
        NULL,                                 // PCTSTR Enumerator, 
        NULL,                                 // HWND hwndParent, 
        DIGCF_DEVICEINTERFACE | DIGCF_PRESENT // DWORD Flags
        );
  return ( (h==INVALID_HANDLE_VALUE) ? NULL : h );
}


//static
void
USBIOLIB_CALL
CUsbIo::DestroyDeviceList(HDEVINFO DeviceList)
{
  // make sure the setupapi dll is loaded
  if ( !smSetupApi.Load() ) {
    return;
  }

  if ( DeviceList!=NULL ) {
    (smSetupApi.SetupDiDestroyDeviceInfoList)(DeviceList);
  }
}




DWORD CUsbIo::Open(int DeviceNumber, HDEVINFO DeviceList, const GUID* InterfaceGuid)
{
  DWORD Status;
  HANDLE h;
  char NameBuffer[80];
  const char* Name;

  if ( FileHandle != NULL ) {
    // already open
    return USBIO_ERR_DEVICE_ALREADY_OPENED;
  } 
  
  if ( DeviceList == NULL ) {
    // use the old way, using a well-known device name
    // build device name
    sprintf(NameBuffer,"\\\\.\\" USBIO_DEVICE_NAME "%d",DeviceNumber);
    Name = NameBuffer;
  } else {
    // use the device interface identified by InterfaceGuid
    // a GUID must be provided in this case
    Status = GetDeviceInstanceDetails(DeviceNumber, DeviceList, InterfaceGuid);
    if ( Status != USBIO_ERR_SUCCESS ) {
      return Status;
    }
    // get name
    Name = GetDevicePathName();
  }

  // try to open the device driver
  h = ::CreateFile(
            Name,
            GENERIC_READ | GENERIC_WRITE,       // access mode
            FILE_SHARE_WRITE | FILE_SHARE_READ, // share mode
            NULL,                               // security desc.
            OPEN_EXISTING,                      // how to create
            FILE_FLAG_OVERLAPPED,               // file attributes
            NULL                                // template file
            );
  if ( h == INVALID_HANDLE_VALUE ) {
    Status = GetLastError();
		// map ERROR_FILE_NOT_FOUND to a specific USBIO error code
		if (Status == ERROR_FILE_NOT_FOUND) {
			Status = USBIO_ERR_DEVICE_NOT_FOUND;
		}
  } else {
    // save handle
    FileHandle = h;
    // init the event with auto reset, not signaled
    Overlapped.hEvent = CreateEvent(NULL ,FALSE ,FALSE ,NULL); 
    if ( Overlapped.hEvent == NULL ) {
      Status = USBIO_ERR_NO_MEMORY;
      Close();
    } else {
      
      // now get version info
      USBIO_DRIVER_INFO info;
      Status = GetDriverInfo(&info);
      if ( Status != USBIO_ERR_SUCCESS ) {
        // failed
        Close();
      } else {

        CheckedBuildDetected = (info.Flags&USBIO_INFOFLAG_CHECKED_BUILD) ? TRUE : FALSE;
        DemoVersionDetected = (info.Flags&USBIO_INFOFLAG_DEMO_VERSION) ? TRUE : FALSE;
        LightVersionDetected = (info.Flags&USBIO_INFOFLAG_LIGHT_VERSION) ? TRUE : FALSE;

        // now check the API version
        // major version must match the driver version.
        if ( (info.APIVersion & 0xff00) != (USBIO_API_VERSION & 0xff00) ||
             (info.APIVersion & 0xff) < (USBIO_API_VERSION & 0xff)) {
          // wrong version
          Status = USBIO_ERR_VERSION_MISMATCH;
          Close();
        } else {
          
          // success
          Status = USBIO_ERR_SUCCESS;
        }
      }
    }
  }
  return Status;
}



void CUsbIo::Close()
{
  if ( FileHandle != NULL ) {
    ::CloseHandle(FileHandle);
    FileHandle = NULL;
  }
  if ( Overlapped.hEvent != NULL ) {
    ::CloseHandle(Overlapped.hEvent);
    Overlapped.hEvent = NULL;
  }
  if ( mDevDetail != NULL ) {
    delete [] (char*)mDevDetail;
    mDevDetail = NULL;
  }
}



DWORD
CUsbIo::GetDeviceInstanceDetails(int DeviceNumber, HDEVINFO DeviceList, const GUID* InterfaceGuid)
{
  DWORD Status;
  BOOL succ;

  // check parameters
  if ( DeviceList==NULL || InterfaceGuid==NULL ) {
    return USBIO_ERR_INVALID_FUNCTION_PARAM;
  }

  // make sure the setupapi dll is loaded
  if ( !smSetupApi.Load() ) {
    return USBIO_ERR_LOAD_SETUP_API_FAILED;
  }

  // delete old detail data if any
  if ( mDevDetail!=NULL ) {
    delete [] (char*)mDevDetail;
    mDevDetail = NULL;
  }

  // enumerate the interface
  // get the device information for the given device number
  SP_DEVICE_INTERFACE_DATA DevData;
  ZeroMemory(&DevData,sizeof(DevData));
  DevData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  succ = (smSetupApi.SetupDiEnumDeviceInterfaces)(DeviceList, NULL, (GUID*)InterfaceGuid, DeviceNumber, &DevData );
  if ( !succ ) {
    Status = GetLastError();
    if ( Status==ERROR_NO_MORE_ITEMS ) {
      Status = USBIO_ERR_NO_SUCH_DEVICE_INSTANCE;
    }
    return Status;
  }

  // get length of the detailed information, allocate buffer
  DWORD ReqLen = 0;
  (smSetupApi.SetupDiGetDeviceInterfaceDetail)(DeviceList, &DevData, NULL, 0, &ReqLen, NULL);
  if ( ReqLen==0 ) {
    return USBIO_ERR_FAILED;
  }
  mDevDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[ReqLen];
  if ( mDevDetail==NULL ) {
    return USBIO_ERR_NO_MEMORY;
  }

  // now get the  detailed device information
  ZeroMemory(mDevDetail,ReqLen);
  mDevDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
  succ = (smSetupApi.SetupDiGetDeviceInterfaceDetail)(DeviceList, &DevData, mDevDetail, ReqLen, &ReqLen, NULL);
  if ( !succ ) {
    Status = GetLastError();
    return Status;
  }

  // success, mDevDetail contains the device instance details now
  return USBIO_ERR_SUCCESS;
}


const char*
CUsbIo::GetDevicePathName()
{
  if ( mDevDetail!=NULL ) {
    return mDevDetail->DevicePath;
  } else {
    return NULL;
  }
}


DWORD CUsbIo::GetDriverInfo(USBIO_DRIVER_INFO *DriverInfo)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_GET_DRIVER_INFO,
              NULL,
              0,
              DriverInfo,
              sizeof(USBIO_DRIVER_INFO),
              NULL
              );

  return Status;
}

DWORD CUsbIo::AcquireDevice()
{
 DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_ACQUIRE_DEVICE,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CUsbIo::ReleaseDevice()
{
 DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_RELEASE_DEVICE,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


BOOL
CUsbIo::IsOperatingAtHighSpeed()
{
  USBIO_DEVICE_INFO info;
  ZeroMemory(&info,sizeof(info));

  DWORD Status = GetDeviceInfo(&info);
  if ( Status==USBIO_ERR_SUCCESS ) {
    return (info.Flags&USBIO_DEVICE_INFOFLAG_HIGH_SPEED) ? TRUE : FALSE;
  } else {
    // query failed
    return FALSE;
  }
}



DWORD CUsbIo::GetDeviceInfo(USBIO_DEVICE_INFO* DeviceInfo)
{
  return IoctlSync(
              IOCTL_USBIO_GET_DEVICE_INFO,
              NULL,
              0,
              DeviceInfo,
              sizeof(USBIO_DEVICE_INFO),
              NULL
              );
}


DWORD CUsbIo::GetBandwidthInfo(USBIO_BANDWIDTH_INFO* BandwidthInfo)
{
  return IoctlSync(
              IOCTL_USBIO_GET_BANDWIDTH_INFO,
              NULL,
              0,
              BandwidthInfo,
              sizeof(USBIO_BANDWIDTH_INFO),
              NULL
              );
}


DWORD CUsbIo::GetDescriptor(
        void* Buffer,
        DWORD& ByteCount,
        USBIO_REQUEST_RECIPIENT Recipient,
        UCHAR DescriptorType,
        UCHAR DescriptorIndex/*=0*/,
        USHORT LanguageId/*=0*/ 
        )
{
  DWORD Status;
  USBIO_DESCRIPTOR_REQUEST req;

  // zero the struct if any fields are added...
  ZeroMemory(&req,sizeof(req));
  req.Recipient = Recipient;
  req.DescriptorType = DescriptorType;
  req.DescriptorIndex = DescriptorIndex;
  req.LanguageId = LanguageId;
  
  Status = IoctlSync(
              IOCTL_USBIO_GET_DESCRIPTOR,
              &req,
              sizeof(req),
              Buffer,
              ByteCount,
              &ByteCount
              );

  return Status;
}



DWORD CUsbIo::SetDescriptor(
        const void* Buffer,
        DWORD& ByteCount,
        USBIO_REQUEST_RECIPIENT Recipient,
        UCHAR DescriptorType,
        UCHAR DescriptorIndex/*=0*/,
        USHORT LanguageId/*=0*/ 
        )
{
  DWORD Status;
  USBIO_DESCRIPTOR_REQUEST req;

  // zero the struct if any fields are added...
  ZeroMemory(&req,sizeof(req));
  req.Recipient = Recipient;
  req.DescriptorType = DescriptorType;
  req.DescriptorIndex = DescriptorIndex;
  req.LanguageId = LanguageId;
  
  Status = IoctlSync(
              IOCTL_USBIO_SET_DESCRIPTOR,
              &req,
              sizeof(req),
              (void*)Buffer,
              ByteCount,
              &ByteCount
              );

  return Status;
}



DWORD CUsbIo::SetFeature(
        USBIO_REQUEST_RECIPIENT Recipient,
        USHORT FeatureSelector,
        USHORT Index/*=0*/ 
        )
{
  DWORD Status;
  USBIO_FEATURE_REQUEST req;

  // zero the struct if any fields are added...
  ZeroMemory(&req,sizeof(req));
  req.Recipient = Recipient;
  req.FeatureSelector = FeatureSelector;
  req.Index = Index;
  
  Status = IoctlSync(
              IOCTL_USBIO_SET_FEATURE,
              &req,
              sizeof(req),
              NULL,
              0,
              NULL
              );

  return Status;
}



DWORD CUsbIo::ClearFeature(
        USBIO_REQUEST_RECIPIENT Recipient,
        USHORT FeatureSelector,
        USHORT Index/*=0*/ 
        )
{
  DWORD Status;
  USBIO_FEATURE_REQUEST req;

  // zero the struct if any fields are added...
  ZeroMemory(&req,sizeof(req));
  req.Recipient = Recipient;
  req.FeatureSelector = FeatureSelector;
  req.Index = Index;
  
  Status = IoctlSync(
              IOCTL_USBIO_CLEAR_FEATURE,
              &req,
              sizeof(req),
              NULL,
              0,
              NULL
              );

  return Status;
}



DWORD CUsbIo::GetStatus(
        USHORT& StatusValue,
        USBIO_REQUEST_RECIPIENT Recipient,
        USHORT Index/*=0*/ )
{
  DWORD Status;
  USBIO_STATUS_REQUEST req;
  USBIO_STATUS_REQUEST_DATA data;

  // zero the structs if any fields are added...
  ZeroMemory(&req,sizeof(req));
  ZeroMemory(&data,sizeof(data));
  req.Recipient = Recipient;
  req.Index = Index;
  
  Status = IoctlSync(
              IOCTL_USBIO_GET_STATUS,
              &req,
              sizeof(req),
              &data,
              sizeof(data),
              NULL
              );

  StatusValue = data.Status;

  return Status;
}



DWORD CUsbIo::ClassOrVendorInRequest(
        void* Buffer,
        DWORD& ByteCount,
        const USBIO_CLASS_OR_VENDOR_REQUEST* Request 
        )
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_CLASS_OR_VENDOR_IN_REQUEST,
              Request,
              sizeof(USBIO_CLASS_OR_VENDOR_REQUEST),
              Buffer,
              ByteCount,
              &ByteCount
              );

  return Status;
}



DWORD CUsbIo::ClassOrVendorOutRequest(
        const void* Buffer,
        DWORD& ByteCount,
        const USBIO_CLASS_OR_VENDOR_REQUEST* Request
        )
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_CLASS_OR_VENDOR_OUT_REQUEST,
              Request,
              sizeof(USBIO_CLASS_OR_VENDOR_REQUEST),
              (void*)Buffer,
              ByteCount,
              &ByteCount
              );

  return Status;
}



DWORD CUsbIo::SetConfiguration(const USBIO_SET_CONFIGURATION* Conf)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_SET_CONFIGURATION,
              Conf,
              sizeof(USBIO_SET_CONFIGURATION),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CUsbIo::UnconfigureDevice()
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_UNCONFIGURE_DEVICE,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CUsbIo::GetConfiguration(UCHAR& ConfigurationValue)
{
  DWORD Status;
  USBIO_GET_CONFIGURATION_DATA data;

  // zero the struct
  ZeroMemory(&data,sizeof(data));
  
  Status = IoctlSync(
              IOCTL_USBIO_GET_CONFIGURATION,
              NULL,
              0,
              &data,
              sizeof(data),
              NULL
              );

  ConfigurationValue = data.ConfigurationValue;

  return Status;
}


DWORD CUsbIo::GetConfigurationInfo(USBIO_CONFIGURATION_INFO *Info)
{
  DWORD Status;

  // zero the struct if any fields are added...
  ZeroMemory(Info,sizeof(USBIO_CONFIGURATION_INFO));

  Status = IoctlSync(
              IOCTL_USBIO_GET_CONFIGURATION_INFO,
              NULL,
              0,
              Info,
              sizeof(USBIO_CONFIGURATION_INFO),
              NULL
              );

  return Status;
}



DWORD CUsbIo::SetInterface(const USBIO_INTERFACE_SETTING* Setting)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_SET_INTERFACE,
              Setting,
              sizeof(USBIO_INTERFACE_SETTING),
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CUsbIo::GetInterface(
        UCHAR& AlternateSetting,
        USHORT Interface/*=0*/
        )
{
  DWORD Status;
  USBIO_GET_INTERFACE req;
  USBIO_GET_INTERFACE_DATA data;

  // zero the structs if any fields are added...
  ZeroMemory(&req,sizeof(req));
  ZeroMemory(&data,sizeof(data));
  req.Interface = Interface;
  
  Status = IoctlSync(
              IOCTL_USBIO_GET_INTERFACE,
              &req,
              sizeof(req),
              &data,
              sizeof(data),
              NULL
              );

  AlternateSetting = data.AlternateSetting;

  return Status;
}


DWORD CUsbIo::StoreConfigurationDescriptor(const USB_CONFIGURATION_DESCRIPTOR *Desc)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_STORE_CONFIG_DESCRIPTOR,
              Desc,
              Desc->wTotalLength,
              NULL,
              0,
              NULL
              );

  return Status;
}



DWORD CUsbIo::GetDeviceParameters(USBIO_DEVICE_PARAMETERS *DevParam)
{
  DWORD Status;

  // zero the struct if any fields are added...
  ZeroMemory(DevParam,sizeof(USBIO_DEVICE_PARAMETERS));

  Status = IoctlSync(
              IOCTL_USBIO_GET_DEVICE_PARAMETERS,
              NULL,
              0,
              DevParam,
              sizeof(USBIO_DEVICE_PARAMETERS),
              NULL
              );

  return Status;
}



DWORD CUsbIo::SetDeviceParameters(const USBIO_DEVICE_PARAMETERS *DevParam)
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_SET_DEVICE_PARAMETERS,
              DevParam,
              sizeof(USBIO_DEVICE_PARAMETERS),
              NULL,
              0,
              NULL
              );

  return Status;
}



DWORD CUsbIo::ResetDevice()
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_RESET_DEVICE,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}


DWORD CUsbIo::CyclePort()
{
  DWORD Status;

  Status = IoctlSync(
              IOCTL_USBIO_CYCLE_PORT,
              NULL,
              0,
              NULL,
              0,
              NULL
              );

  return Status;
}



DWORD CUsbIo::GetCurrentFrameNumber(DWORD &FrameNumber)
{
  DWORD Status;
  USBIO_FRAME_NUMBER data;

  // zero the struct if any fields are added...
  ZeroMemory(&data,sizeof(data));

  Status = IoctlSync(
              IOCTL_USBIO_GET_CURRENT_FRAME_NUMBER,
              NULL,
              0,
              &data,
              sizeof(data),
              NULL
              );

  FrameNumber = data.FrameNumber;

  return Status;
}


DWORD CUsbIo::GetDevicePowerState(USBIO_DEVICE_POWER_STATE& DevicePowerState)
{
  USBIO_DEVICE_POWER PowerRequest;
  DWORD err;

  // zero the struct if any fields are added...
  ZeroMemory(&PowerRequest,sizeof(PowerRequest));

  err = IoctlSync(
              IOCTL_USBIO_GET_DEVICE_POWER_STATE,
              NULL,
              0,
              &PowerRequest,
              sizeof(USBIO_DEVICE_POWER),
              NULL
              );
  if ( err == USBIO_ERR_SUCCESS ) {
    DevicePowerState = PowerRequest.DevicePowerState;
  }
  return err;
}


DWORD CUsbIo::SetDevicePowerState(USBIO_DEVICE_POWER_STATE DevicePowerState)
{
  USBIO_DEVICE_POWER PowerRequest;

  PowerRequest.DevicePowerState = DevicePowerState;

  return IoctlSync(
              IOCTL_USBIO_SET_DEVICE_POWER_STATE,
              &PowerRequest,
              sizeof(USBIO_DEVICE_POWER),
              NULL,
              0,
              NULL
              );
}




DWORD CUsbIo::GetDeviceDescriptor(USB_DEVICE_DESCRIPTOR* Desc)
{
  DWORD ByteCount = sizeof(USB_DEVICE_DESCRIPTOR);

  return GetDescriptor(
              Desc,
              ByteCount,
              RecipientDevice,
              USB_DEVICE_DESCRIPTOR_TYPE,
              0,
              0
              );
}



DWORD CUsbIo::GetConfigurationDescriptor(
        USB_CONFIGURATION_DESCRIPTOR* Desc,
        DWORD& ByteCount,
        UCHAR Index/*=0*/
        )
{

  return GetDescriptor(
              Desc,
              ByteCount,
              RecipientDevice,
              USB_CONFIGURATION_DESCRIPTOR_TYPE,
              Index,
              0
              );
}



DWORD CUsbIo::GetStringDescriptor(
        USB_STRING_DESCRIPTOR* Desc,
        DWORD& ByteCount,
        UCHAR Index/*=0*/,
        USHORT LanguageId/*=0*/
        )
{

  return GetDescriptor(
              Desc,
              ByteCount,
              RecipientDevice,
              USB_STRING_DESCRIPTOR_TYPE,
              Index,
              LanguageId
              );
}




DWORD CUsbIo::IoctlSync(
        DWORD IoctlCode,
        const void *InBuffer,
        DWORD InBufferSize,
        void *OutBuffer,
        DWORD OutBufferSize,
        DWORD *BytesReturned
        )
{
  DWORD Status;
  DWORD BytesRet = 0;
  BOOL succ;

  // check if the driver was opened
  if ( FileHandle == NULL ) {
    return USBIO_ERR_DEVICE_NOT_OPEN;
  }

  // IOCTL requests must be serialized
  // bec. there is only one event object per instance
  EnterCriticalSection(&CritSect);

  // call the device driver
  succ = DeviceIoControl(
            FileHandle,         // driver handle
            IoctlCode,          // IOCTL code
            (void*)InBuffer,    // input buffer
            InBufferSize,       // input buffer size
            OutBuffer,          // output buffer
            OutBufferSize,      // output buffer size
            &BytesRet,          // number of bytes returned
            &Overlapped         // overlapped structure (async.)
            );
  if ( succ ) {
    // ioctl completed successfully
    Status = USBIO_ERR_SUCCESS;
  } else {
    Status = GetLastError();
    if ( Status == ERROR_IO_PENDING ) {
      // the operation is pending, wait for completion
      succ = GetOverlappedResult(
                FileHandle,
                &Overlapped,
                &BytesRet,  // byte count
                TRUE        // wait flag
                );
      if ( succ ) {
        // completed successfully
        Status = USBIO_ERR_SUCCESS;
      } else {
        Status = GetLastError();
      }
    }
  }

  LeaveCriticalSection(&CritSect);

  if ( BytesReturned != NULL ) {
    *BytesReturned = BytesRet;
  }

  return Status;
}



BOOL CUsbIo::CancelIo()
{
  // cancel all outstanding requests that were
  // issued by the calling thread on this handle
  return ::CancelIo(FileHandle);
}



// helper struct
struct _ErrorCodeTable {
  DWORD Code;
  const char *String;
};

//static 
char* 
USBIOLIB_CALL
CUsbIo::ErrorText(char* StringBuffer, DWORD StringBufferSize, DWORD ErrorCode)
{
  // string table
  static const struct _ErrorCodeTable ErrorTable[] = {
    {USBIO_ERR_SUCCESS                , "No error."},
    {USBIO_ERR_CRC                    , "HC Error: Wrong CRC."},
    {USBIO_ERR_BTSTUFF                , "HC Error: Wrong bit stuffing."},
    {USBIO_ERR_DATA_TOGGLE_MISMATCH   , "HC Error: Data toggle mismatch."},
    {USBIO_ERR_STALL_PID              , "HC Error: stall PID."},
    {USBIO_ERR_DEV_NOT_RESPONDING     , "HC Error: Device not responding."},
    {USBIO_ERR_PID_CHECK_FAILURE      , "HC Error: PID check failed."},
    {USBIO_ERR_UNEXPECTED_PID         , "HC Error: Unexpected PID."},
    {USBIO_ERR_DATA_OVERRUN           , "HC Error: Data Overrun."},
    {USBIO_ERR_DATA_UNDERRUN          , "HC Error: Data Underrun."},
    {USBIO_ERR_RESERVED1              , "HC Error: Reserved1."},
    {USBIO_ERR_RESERVED2              , "HC Error: Reserved2."},
    {USBIO_ERR_BUFFER_OVERRUN         , "HC Error: Buffer Overrun."},
    {USBIO_ERR_BUFFER_UNDERRUN        , "HC Error: Buffer Underrun."},
    {USBIO_ERR_NOT_ACCESSED           , "HC Error: Not accessed."},
    {USBIO_ERR_FIFO                   , "HC Error: FIFO error."},
    {USBIO_ERR_XACT_ERROR             , "HC Error: XACT error."},
    {USBIO_ERR_BABBLE_DETECTED        , "HC Error: Babble detected."},
    {USBIO_ERR_DATA_BUFFER_ERROR      , "HC Error: Data buffer error."},

    {USBIO_ERR_ENDPOINT_HALTED        , "USBD Error: Endpoint halted."},
    {USBIO_ERR_NO_MEMORY              , "USBD Error: No system memory."},
    {USBIO_ERR_INVALID_URB_FUNCTION   , "USBD Error: Invalid URB function."},
    {USBIO_ERR_INVALID_PARAMETER      , "USBD Error: Invalid parameter."},
    {USBIO_ERR_ERROR_BUSY             , "USBD Error: Busy."},
    {USBIO_ERR_REQUEST_FAILED         , "USBD Error: Request failed."},
    {USBIO_ERR_INVALID_PIPE_HANDLE    , "USBD Error: Invalid pipe handle."},
    {USBIO_ERR_NO_BANDWIDTH           , "USBD Error: No bandwidth available."},
    {USBIO_ERR_INTERNAL_HC_ERROR      , "USBD Error: Internal HC error."},
    {USBIO_ERR_ERROR_SHORT_TRANSFER   , "USBD Error: Short transfer."},
    {USBIO_ERR_BAD_START_FRAME        , "USBD Error: Bad start frame."},
    {USBIO_ERR_ISOCH_REQUEST_FAILED   , "USBD Error: Isochronous request failed."},
    {USBIO_ERR_FRAME_CONTROL_OWNED    , "USBD Error: Frame control owned."},
    {USBIO_ERR_FRAME_CONTROL_NOT_OWNED, "USBD Error: Frame control not owned."},
    {USBIO_ERR_NOT_SUPPORTED          , "USBD Error: Not supported."},
    {USBIO_ERR_INVALID_CONFIGURATION_DESCRIPTOR, "USBD Error: Invalid configuration descriptor."},

    {USBIO_ERR_INSUFFICIENT_RESOURCES   , "USBD Error: Insufficient resources."},
    {USBIO_ERR_SET_CONFIG_FAILED        , "USBD Error: Set configuration failed."},
    {USBIO_ERR_USBD_BUFFER_TOO_SMALL    , "USBD Error: Buffer too small."},
    {USBIO_ERR_USBD_INTERFACE_NOT_FOUND , "USBD Error: Interface not found."},
    {USBIO_ERR_INVALID_PIPE_FLAGS       , "USBD Error: Invalid pipe flags."},
    {USBIO_ERR_USBD_TIMEOUT             , "USBD Error: Timeout."},
    {USBIO_ERR_DEVICE_GONE              , "USBD Error: Device gone."},
    {USBIO_ERR_STATUS_NOT_MAPPED        , "USBD Error: Status not mapped."},


    {USBIO_ERR_CANCELED               , "USBD Error: cancelled."},
    {USBIO_ERR_ISO_NOT_ACCESSED_BY_HW , "USBD Error: ISO not accessed by hardware."},
    {USBIO_ERR_ISO_TD_ERROR           , "USBD Error: ISO TD error."},
    {USBIO_ERR_ISO_NA_LATE_USBPORT    , "USBD Error: ISO NA late USB port."},
    {USBIO_ERR_ISO_NOT_ACCESSED_LATE  , "USBD Error: ISO not accessed, submitted too late."},

    {USBIO_ERR_FAILED                 , "Operation failed."},
    {USBIO_ERR_INVALID_INBUFFER       , "Input buffer too small."},
    {USBIO_ERR_INVALID_OUTBUFFER      , "Output buffer too small."},
    {USBIO_ERR_OUT_OF_MEMORY          , "Out of memory."},
    {USBIO_ERR_PENDING_REQUESTS       , "There are pending requests."},
    {USBIO_ERR_ALREADY_CONFIGURED     , "USB device is already configured."},
    {USBIO_ERR_NOT_CONFIGURED         , "USB device is not configured."},
    {USBIO_ERR_OPEN_PIPES             , "There are open pipes."},
    {USBIO_ERR_ALREADY_BOUND          , "Either handle or pipe is already bound."},
    {USBIO_ERR_NOT_BOUND              , "Handle is not bound to a pipe."},
    {USBIO_ERR_DEVICE_NOT_PRESENT     , "Device is removed."},
    {USBIO_ERR_CONTROL_NOT_SUPPORTED  , "Control code is not supported."},
    {USBIO_ERR_TIMEOUT                , "The request has been timed out."},
    {USBIO_ERR_INVALID_RECIPIENT      , "Invalid recipient."},
    {USBIO_ERR_INVALID_TYPE           , "Invalid pipe type or invalid request type."},
    {USBIO_ERR_INVALID_IOCTL          , "Invalid I/O control code."},
    {USBIO_ERR_INVALID_DIRECTION      , "Invalid direction of read/write operation."},
    {USBIO_ERR_TOO_MUCH_ISO_PACKETS   , "Too much ISO packets."},
    {USBIO_ERR_POOL_EMPTY             , "Request pool empty."},
    {USBIO_ERR_PIPE_NOT_FOUND         , "Pipe not found."},
    {USBIO_ERR_INVALID_ISO_PACKET     , "Invalid ISO packet."},
    {USBIO_ERR_OUT_OF_ADDRESS_SPACE   , "Out of address space. Not enough system resources."},
    {USBIO_ERR_INTERFACE_NOT_FOUND    , "Interface not found."},
    {USBIO_ERR_INVALID_DEVICE_STATE   , "Invalid device state (stopped or power down)."},
    {USBIO_ERR_INVALID_PARAM          , "Invalid parameter."},
    {USBIO_ERR_DEMO_EXPIRED           , "DEMO version has timed out. Reboot required!"},
    {USBIO_ERR_INVALID_POWER_STATE    , "Power state not allowed. Set to D0 first."},
    {USBIO_ERR_POWER_DOWN             , "Device powered down."},
    {USBIO_ERR_VERSION_MISMATCH       , "API Version does not match."},
    {USBIO_ERR_SET_CONFIGURATION_FAILED,"Set configuration failed."},
    {USBIO_ERR_INVALID_PROCESS,         "Invalid process."},
    {USBIO_ERR_DEVICE_ACQUIRED,         "The device is acquired by another process for exclusive use."},
    {USBIO_ERR_DEVICE_OPENED,           "A different process has opened the device."},

    {USBIO_ERR_VID_RESTRICTION,         "Light version restriction: Unsupported Vendor ID."},
    {USBIO_ERR_ISO_RESTRICTION,         "Light version restriction: Iso pipes are not supported."},
    {USBIO_ERR_BULK_RESTRICTION,        "Light version restriction: Bulk pipes are not supported."},
    {USBIO_ERR_EP0_RESTRICTION,         "Light version restriction: EP0 requests are not fully supported."},
    {USBIO_ERR_PIPE_RESTRICTION,        "Light version restriction: Too many pipes."},
    {USBIO_ERR_PIPE_SIZE_RESTRICTION,   "Light version restriction: Maximum FIFO size exceeded."},
    {USBIO_ERR_CONTROL_RESTRICTION,     "Light version restriction: Control pipes are not supported."},
    {USBIO_ERR_INTERRUPT_RESTRICTION,   "Light version restriction: Interrupt pipes are not supported."},

    {USBIO_ERR_DEVICE_NOT_FOUND       , "Device not found or acquired for exclusive use."},
    {USBIO_ERR_DEVICE_NOT_OPEN        , "Device not open."},
    {USBIO_ERR_NO_SUCH_DEVICE_INSTANCE, "No such device instance."},
    {USBIO_ERR_INVALID_FUNCTION_PARAM,  "An invalid parameter was passed."},
    {USBIO_ERR_DEVICE_ALREADY_OPENED,   "The handle is already opened."},
  };

  static int Size = sizeof(ErrorTable)/sizeof(struct _ErrorCodeTable);
  const char *ErrorString = "Windows system error code.";
  int i;
  BOOL found=FALSE;

  if ( (StringBuffer==NULL) || (StringBufferSize==0) ) {
    return StringBuffer;
  }

  for (i=0;i<Size;i++) {
    if (ErrorTable[i].Code == ErrorCode) {
      ErrorString=ErrorTable[i].String;
      found = TRUE;
      break;
    }
  }

  // the following does not produce useful error messages
  // so we don't use it anymore
  /*
  char* MsgBuffer = NULL;
  if (!found) {
    if (0 != FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                ErrorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR)&MsgBuffer,    
                0,    
                NULL 
                )) {
      // found
      ErrorString = MsgBuffer; 
    }
  }
  */
  // print to string buffer
  _snprintf(StringBuffer,StringBufferSize,"Error code 0x%08X: %s",ErrorCode,ErrorString); 
  // make sure the string is zero-terminated
  StringBuffer[StringBufferSize-1] = 0;

/*
  // free resources
  if ( MsgBuffer!=NULL ) {
    LocalFree(MsgBuffer);
  }
*/

  return StringBuffer;

}//ErrorText



/*************************** EOF **************************************/
