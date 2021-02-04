//-----------------------------------------------------------------------------

  A directory 'usbfwu' contains a source code for the 'USB Firmware upgrader'
demo application.
  The project uses MFC and was compiled by Visual Studio 2008.

  To use this 'USB Firmware upgrader' with software examples from the file
'usb-fwu-1-0-1-lpc214x.zip', you should change line N 232 in the file 'fwu.c'
from
     for(dly=0;dly<10000;dly++); //--10000
to
     for(dly=0;dly<600000;dly++);

//----------------------------------------------------------------------------

  A file 'usbio_demo.exe' is a Thesycon(r) free USB Development Kit Demo for
Windows v.2.41 (includes demo version of the host USB driver);
  A source codes for the USBIOLIB library (directory 'USBIOLIB' inside
directory 'usbfwu') has been taken therefrom.
