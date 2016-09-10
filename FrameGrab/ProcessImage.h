#pragma once
#include "stdafx.h"
#include "FrameGrab.h"
#include <Strsafe.h> 
#include <stdio.h>
#include <windows.h>  
#include <vfw.h>  
#include <time.h>
#include <wchar.h>
#include <cwchar>
#include <dshow.h>
#include <vector>


class ProcessImage
{
public:
	ProcessImage();
	ProcessImage(PBITMAPINFOHEADER, LPBYTE);
	ProcessImage(wchar_t[]);


	~ProcessImage();
};

