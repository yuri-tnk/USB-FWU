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
#include "AnimBmp.h"

//----------------------------------------------------------------------------
CAnimBmp::CAnimBmp()
{
   m_hImage = NULL;
   m_size.cx = 0;
   m_size.cy = 0;

   m_Width = 0;
   m_Height = 0;
   m_Rows = 0;
   m_Cols = 0;
   m_NumPictures = 0;
   m_Cnt = 0;
}

//----------------------------------------------------------------------------
CAnimBmp::~CAnimBmp()
{

}

//----------------------------------------------------------------------------
void CAnimBmp::SetBaseImage(HANDLE Image,  //--
                 int x_size,                 //-- Single picture X size
                 int y_size,                 //-- Single picture Y size
                 int n_pictures)             //-- N pictures in BMP(especially when != rows * columns)
{
   if(x_size <= 0 || y_size <= 0 || n_pictures <= 0)
   {
      m_hImage = NULL;
      return;
   }

   m_Width  = x_size;
   m_Height = y_size;

   if(m_hImage && m_hImage != Image)
   {
      m_hImage = NULL;
      m_size.cx = 0;
      m_size.cy = 0;
   }

   if(Image)
   {
      BITMAP bmp;
      if(GetObject((HBITMAP)Image, sizeof(BITMAP), &bmp))
      {
         m_size.cx = bmp.bmWidth;
         m_size.cy = bmp.bmHeight;
      }
   }

   m_Rows = m_size.cy/y_size;
   m_Cols = m_size.cx/x_size;
   if(m_Rows == 0 || m_Cols == 0)
      m_hImage = NULL;
   else
      m_hImage = Image;

   m_NumPictures = min(n_pictures, m_Rows * m_Cols);
}

//----------------------------------------------------------------------------
void CAnimBmp::DrawAnimFrame(CDC *pToDC, int x, int y)
{
   //-- get curr picture coordinates
   if(m_hImage == NULL)
      return;

   int y_pos = (m_Cnt/m_Cols)* m_Height;
   int x_pos = (m_Cnt%m_Cols)* m_Width;

   DrawImage(pToDC,x,y,m_Width,m_Height,x_pos,y_pos);

   m_Cnt++;
   if(m_Cnt >= m_NumPictures)
      m_Cnt = 0;
}

//----------------------------------------------------------------------------
void CAnimBmp::DrawFrameTransparent(CDC *pToDC, int w, int h,int x_pos,int y_pos,CDC *pFromDC)
{
   CDC MonoDC;
   CDC DisabledDC;
   CBitmap DisabledBitmap;
   CBitmap MonoDCbmp;

   CDC *pOutputDC = pFromDC;

   MonoDC.CreateCompatibleDC(pToDC);

   DisabledDC.CreateCompatibleDC(pToDC);

   int savedToDC   = pToDC->SaveDC();
   int savedFromDC = pFromDC->SaveDC();
   int savedMonoDC = MonoDC.SaveDC();
   int savedDisabledDC = DisabledDC.SaveDC();

   pToDC->SetBkColor(RGB(255, 255, 255));
   pToDC->SetTextColor(RGB(0, 0, 0));
   pFromDC->SetBkColor(m_TransparentColour);

   // Create the mask
   MonoDCbmp.CreateBitmap(w, h, 1, 1, NULL);
   MonoDC.SelectObject(&MonoDCbmp);
   MonoDC.BitBlt(0, 0, w, h, pOutputDC, x_pos, y_pos, SRCCOPY);

   // draw the transparent bitmap
   pToDC->BitBlt(0, 0, w, h, pOutputDC, x_pos, y_pos, SRCINVERT);
   pToDC->BitBlt(0, 0, w, h, &MonoDC, 0, 0, SRCAND);
   pToDC->BitBlt(0, 0, w, h, pOutputDC, x_pos, y_pos, SRCINVERT);

   DisabledDC.RestoreDC(savedDisabledDC);
   DisabledDC.DeleteDC();

   MonoDC.RestoreDC(savedMonoDC);
   MonoDC.DeleteDC();

   pFromDC->RestoreDC(savedFromDC);
   pToDC->RestoreDC(savedToDC);
}

//----------------------------------------------------------------------------
void CAnimBmp::DrawImage(CDC *pDC, int x, int y, int w, int h, int x_pos,int y_pos)
{
   CRgn ClipRgn;
   CDC memDC;
   CBitmap memDCBmp;
   CBitmap BackGroundBitmap;
   CDC BackGroundDC;
   int savedBackGroundDC;
   int savedmemDC;
   CDC TransparentDC;
   CBitmap Transparentbmp;
   int savedTransparentDC;

   CDC* pOutputDC = &memDC;
   int left = x;
   int top = y;

   if(!m_hImage)
      return;

// set the clip region to the specified rectangle
   ClipRgn.CreateRectRgn(x, y, x + w, y + h);
   pDC->SelectClipRgn(&ClipRgn);
   ClipRgn.DeleteObject();

// create memory DC
   memDC.CreateCompatibleDC(pDC);
   savedmemDC = memDC.SaveDC();
// Get the background image for transparent images

   BackGroundBitmap.CreateCompatibleBitmap(pDC, w, h);
   BackGroundDC.CreateCompatibleDC(pDC);
   savedBackGroundDC = BackGroundDC.SaveDC();
   BackGroundDC.SelectObject(&BackGroundBitmap);
   BackGroundDC.BitBlt(0, 0, w, h, pDC, x, y, SRCCOPY);

// Create a DC and bitmap for the transparent image

  // place bitmap image into the memory DC
   memDC.SelectObject((HBITMAP)m_hImage);

   m_TransparentColour = memDC.GetPixel(0, m_Height-1);

      // draw the image transparently
   TransparentDC.CreateCompatibleDC(pDC);
   savedTransparentDC = TransparentDC.SaveDC();
   Transparentbmp.CreateCompatibleBitmap(pDC, m_Width, m_Height);
   TransparentDC.SelectObject(&Transparentbmp);
   TransparentDC.BitBlt(0, 0, w, h, &BackGroundDC, 0, 0, SRCCOPY);

   DrawFrameTransparent(&TransparentDC,m_Width,m_Height,x_pos,y_pos,pOutputDC);

   pOutputDC = &TransparentDC;

   pDC->BitBlt(left, top, m_Width, m_Height, pOutputDC, 0, 0, SRCCOPY);

   TransparentDC.RestoreDC(savedTransparentDC);
   TransparentDC.DeleteDC();

   memDC.RestoreDC(savedmemDC);
   memDC.DeleteDC();
   BackGroundDC.RestoreDC(savedBackGroundDC);
   BackGroundDC.DeleteDC();
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
