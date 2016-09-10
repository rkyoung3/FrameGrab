#include "stdafx.h"
#include "ProcessImage.h"
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

// OpenCV
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\imgcodecs\imgcodecs.hpp"
#include "opencv2\highgui\highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

ProcessImage::ProcessImage()
{



}

ProcessImage::ProcessImage(wchar_t* pi_FileName)
{
	/*
	* Function: cvCreateMat(int rows, int cols, int type)
	* Parameters:
	* rows – Number of rows in the matrix
	* cols – Number of columns in the matrix
	* type – The type of the matrix elements in the form CV_<bit depth><S|U|F>C<number of channels>, where S=signed, U=unsigned, F=float.
	* For example: [CV _ 8UC1 means the elements are 8-bit unsigned and the there is 1 channel],
	* and:         [CV _ 32SC2 means the elements are 32-bit signed and there are 2 channels.
	*************************************************************************************************************************************/
	wstring ws(pi_FileName);
	// your new String
	string str(ws.begin(), ws.end());
	// Show String
	cout << str << endl;
	
	Mat image = imread(str);
	namedWindow("image", WINDOW_NORMAL);
	imshow("image", image);
	waitKey(0);
	destroyAllWindows();

}

ProcessImage::ProcessImage(PBITMAPINFOHEADER msBitmap, LPBYTE RGBArray)
{

	
	PBITMAPINFOHEADER pbih;

	pbih = msBitmap;
	int width = pbih->biWidth;
	int height = pbih->biHeight;

	/*
	* Function: cvCreateMat(int rows, int cols, int type)
	* Parameters:
	* rows – Number of rows in the matrix
	* cols – Number of columns in the matrix
	* type – The type of the matrix elements in the form CV_<bit depth><S|U|F>C<number of channels>, where S=signed, U=unsigned, F=float. 
	* For example: [CV _ 8UC1 means the elements are 8-bit unsigned and the there is 1 channel], 
	* and:         [CV _ 32SC2 means the elements are 32-bit signed and there are 2 channels.	
	*************************************************************************************************************************************/

	Mat mat = Mat(width, height, CV_8UC3);
	// mat.ptr = RGBArray; // Point to the raw array
	// Make a copy of the orginal
	imread("FileName");
	Mat image = mat.clone();
	namedWindow("image", WINDOW_NORMAL);
	imshow("image", image);
	waitKey(0); 
	destroyAllWindows();
}

ProcessImage::~ProcessImage()
{


}
