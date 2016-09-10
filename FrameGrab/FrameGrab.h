#pragma once

#include "resource.h"
#include "ProcessImage.h"
#include <Mfidl.h>

#define CANERA_WIDTH 1280 
#define CANERA_HEIGHT 720 
#define MENU_BUTTON_STARTCAM 1
#define MENU_BUTTON_STOPCAM 2
#define MENU_BUTTON_CHANGECAM 4
#define MENU_BUTTON_CAPTURE_FRAME 3
#define MENU_BUTTON_SAVE_DIR 5

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

template <class T> void SafeReleaseAllCount(T **ppT)
{
	if (*ppT)
	{
		ULONG e = (*ppT)->Release();

		while (e)
		{
			e = (*ppT)->Release();
		}

		*ppT = NULL;
	}
}

struct IMFMediaSource;

// Structure for collecting info about types of video, which are supported by current video device
struct MediaType
{
	unsigned int MF_MT_FRAME_SIZE;

	unsigned int height;

	unsigned int width;

	unsigned int MF_MT_YUV_MATRIX;

	unsigned int MF_MT_VIDEO_LIGHTING;

	unsigned int MF_MT_DEFAULT_STRIDE;

	unsigned int MF_MT_VIDEO_CHROMA_SITING;

	GUID MF_MT_AM_FORMAT_TYPE;

	wchar_t *pMF_MT_AM_FORMAT_TYPEName;

	unsigned int MF_MT_FIXED_SIZE_SAMPLES;

	unsigned int MF_MT_VIDEO_NOMINAL_RANGE;

	unsigned int MF_MT_FRAME_RATE;

	unsigned int MF_MT_FRAME_RATE_low;

	unsigned int MF_MT_PIXEL_ASPECT_RATIO;

	unsigned int MF_MT_PIXEL_ASPECT_RATIO_low;

	unsigned int MF_MT_ALL_SAMPLES_INDEPENDENT;

	unsigned int MF_MT_FRAME_RATE_RANGE_MIN;

	unsigned int MF_MT_FRAME_RATE_RANGE_MIN_low;

	unsigned int MF_MT_SAMPLE_SIZE;

	unsigned int MF_MT_VIDEO_PRIMARIES;

	unsigned int MF_MT_INTERLACE_MODE;

	unsigned int MF_MT_FRAME_RATE_RANGE_MAX;

	unsigned int MF_MT_FRAME_RATE_RANGE_MAX_low;

	GUID MF_MT_MAJOR_TYPE;

	wchar_t *pMF_MT_MAJOR_TYPEName;

	GUID MF_MT_SUBTYPE;

	wchar_t *pMF_MT_SUBTYPEName;

	MediaType();
	~MediaType();
	void Clear();
};

// Structure for collecting info about one parametr of current video device
struct Parametr
{
	long CurrentValue;

	long Min;

	long Max;

	long Step;

	long Default;

	long Flag;

	Parametr();
};

// Structure for collecting info about 17 parametrs of current video device
struct CamParametrs
{
	Parametr Brightness;
	Parametr Contrast;
	Parametr Hue;
	Parametr Saturation;
	Parametr Sharpness;
	Parametr Gamma;
	Parametr ColorEnable;
	Parametr WhiteBalance;
	Parametr BacklightCompensation;
	Parametr Gain;


	Parametr Pan;
	Parametr Tilt;
	Parametr Roll;
	Parametr Zoom;
	Parametr Exposure;
	Parametr Iris;
	Parametr Focus;
};


// IMFMediaSource *vd_pSource;

// emergensyStopEventCallback vd_func;

// void *vd_userData;
