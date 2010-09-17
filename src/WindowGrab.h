/*
    'accelKit'
    Copyright (C) 2010  Nikos Kastellanos

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
    e-mail: nkastellanos@gmail.com
*/

#ifndef WindowGrab_H_ONCE
#define WindowGrab_H_ONCE

char *_classname;
HWND hWnd;


int wgStart(char* classname)
{	
	_classname = classname;
	hWnd = NULL;

	return TRUE;
}

HBITMAP wgCapture(HBITMAP bmpA)
{
	HWND hWndDesktop;
	RECT desktopRect,visibleRect,appRect;
	int Width, Height;
	HDC hdcFrom,hdcTo;
	HBITMAP hBitmap;
	HBITMAP hLocalBitmap;
	int x=0,y=0;
	int res;
	unsigned char *data;

	BITMAPINFO info;
	BITMAPINFOHEADER header;

	hWnd = FindWindow(_classname,NULL);

	if(hWnd==NULL) return NULL;
	//if(IsWindow(hWnd)==FALSE) return FALSE;
	if(IsWindowVisible(hWnd)==FALSE) return NULL;
	if(IsIconic(hWnd)==TRUE) 
	{
		// bring it to front
		if(SetForegroundWindow(hWnd)==FALSE) return NULL;
		Sleep(100); // allow to redraw
	}

	GetWindowRect(hWnd, &appRect);

	//Intersect with the Desktop rectangle and get what's visible
	hWndDesktop = GetDesktopWindow();
    GetWindowRect(hWndDesktop, &desktopRect);
	if(IntersectRect(&visibleRect,&desktopRect,&appRect)==FALSE)
    {
        visibleRect = appRect;
    }
    if(IsRectEmpty(&visibleRect)) return FALSE;

	Width = visibleRect.right-visibleRect.left;
    Height = visibleRect.bottom-visibleRect.top;

	hdcFrom = GetWindowDC(hWnd);
	
	hdcTo = CreateCompatibleDC(hdcFrom);
	hLocalBitmap = SelectObject(hdcTo, bmpA);
	StretchBlt(hdcTo, 21,39, 214,357, hdcFrom, 0,0, Width,Height, SRCCOPY);
	
    DeleteDC(hdcTo);


	return TRUE;
}


#endif // WindowGrab_H_ONCE


