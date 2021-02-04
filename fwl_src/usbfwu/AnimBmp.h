#ifndef _CANIMBMP_H_
#define _CANIMBMP_H_

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

class CAnimBmp
{
   public:

      CAnimBmp();
      virtual ~CAnimBmp();


      void SetBaseImage(HANDLE Image,
                 int x_size,                 //-- Single pictures X size
                 int y_size,                 //-- Single pictures Y size
                 int n_pictures);            //-- N pictures in BMP(especially when != rows * columns)
      void DrawAnimFrame(CDC *pToDC, int x, int y);


   protected:

      void DrawFrameTransparent(CDC *pToDC, int w, int h,int x_pos,int y_pos,CDC *pFromDC);
      void DrawImage(CDC *pDC, int x, int y, int w, int h, int x_pos,int y_pos);

private:
    HANDLE m_hImage;                    // the image
    CSize m_size;                       // the image size
    COLORREF m_TransparentColour;       // the transparent colour
    int m_Width;
    int m_Height;
    int m_Rows;
    int m_Cols;
    int m_NumPictures;
    int m_Cnt;
};

#endif

