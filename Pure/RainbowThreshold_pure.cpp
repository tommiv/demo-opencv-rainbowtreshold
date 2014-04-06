#include "stdafx.h"
#include <Windows.h>
#include <conio.h>
#include "highgui.h"
#include "cv.h"

#pragma comment (lib, "opencv_core248.lib")
#pragma comment (lib, "opencv_highgui248.lib")

using namespace std;

const int white_threshold = 10;
const int black_threshold = 10;

inline float getTime(unsigned __int64 startCounter) {
	float time = 0;
	unsigned __int64 end = 0, QPF = 0;
	QueryPerformanceCounter((PLARGE_INTEGER)&end);
	QueryPerformanceFrequency((PLARGE_INTEGER) &QPF);
	return time = (float)(end-startCounter)*1000/(float)QPF;
}

int findHue(int cRed, int cGreen, int cBlue) {
	float h = 0, s = 0, v = 0;
	float c_max = max(max(cBlue, cGreen), cRed);
	float c_min = min(min(cBlue, cGreen), cRed);
	
	v = c_max * 100/255;
	
	if (v != 0) { 
		s = (c_max - c_min) * 100 / c_max; 
	}
	
	if (v < black_threshold) {
		h = -1; // black
	} else if (s < white_threshold & v > 100-black_threshold) {
		h = -2; // white
	} else {
        float dmax = c_max - c_min; 
        float delta_r = (c_max - cRed)	/ dmax;
        float delta_g = (c_max - cGreen)/ dmax;
        float delta_b = (c_max - cBlue)	/ dmax; 
        if (cRed == c_max) { 
			h = delta_b - delta_g; // yellow to purple
		} else if (cGreen == c_max) { 
			h = delta_r - delta_b + 2; // blue to yellow
		} else { 
			h = delta_g - delta_r + 4; // purple to blue
		}
        h *= 60.0; 
        if (h < 0.0) { 
			h += 360.0; 
		}
        if (h == 360) { 
			h = 0; 
		} 
    } 
	return floor(h);
}

void rainbowFilter(IplImage* toFilter, IplImage* filtered) {
	int height = toFilter->height;
	int width = toFilter->width;
	int step = toFilter->widthStep/sizeof(uchar);
	int channels = toFilter->nChannels;
	uchar* resultData = (uchar*)filtered->imageData;
	uchar* inputData  = (uchar*)toFilter->imageData;
	int hue = 0;

	int ridx, gidx, bidx;

	for (int row = 0; row < height; row ++)
		for (int col = 0; col < width; col++) {
			// IplImage channels is reversed
			bidx = row*step + col*channels;
			gidx = bidx + 1;
			ridx = bidx + 2;
			
			hue = findHue(
				inputData[ridx],
				inputData[gidx],
				inputData[bidx]
			);
			if (hue == -1) { // black
				resultData[ridx] = 0;
				resultData[gidx] = 0;
				resultData[bidx] = 0; 
			} else if (hue == -2) { // white
				resultData[ridx] = 255;
				resultData[gidx] = 255;
				resultData[bidx] = 255; 
			} else if (hue > 0 & hue < 20) { // red
				resultData[ridx] = 255;
				resultData[gidx] = 0;
				resultData[bidx] = 0; 
			} else if (hue >= 20 & hue < 50) { // orange
				resultData[ridx] = 255;
				resultData[gidx] = 165;
				resultData[bidx] = 0; 
			} else if (hue >= 50 & hue < 90) { // yellow
				resultData[ridx] = 255;
				resultData[gidx] = 255;
				resultData[bidx] = 0; 	
			} else if (hue >= 90 & hue < 164) { // green
				resultData[ridx] = 0;
				resultData[gidx] = 255;
				resultData[bidx] = 0; 	
			} else if (hue >= 164 & hue < 210) { // light blue
				resultData[ridx] = 0;
				resultData[gidx] = 170;
				resultData[bidx] = 255; 	
			} else if (hue >= 210 & hue < 287) { // blue
				resultData[ridx] = 0;
				resultData[gidx] = 0;
				resultData[bidx] = 255; 	
			} else { // hue >= 287, violet
				resultData[ridx] = 139;
				resultData[gidx] = 0;
				resultData[bidx] = 255; 	
			}
	}
}

int _tmain(int argc, _TCHAR* argv[]) {	
	CvCapture* capture = cvCaptureFromCAM(0);
	if (capture == NULL) {
		cout << "No camera found" << endl;
		_getch();
		return 1;
	}

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 480);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, 60);
	IplImage* filtered = cvCreateImage(
		cvSize(
			cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH),
			cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT)
		),
		IPL_DEPTH_8U, 
		3
	);

	unsigned __int64 start;
	float time;

	IplImage* frame;
	while (cvWaitKey(1)!=0x1b) {
		frame = cvQueryFrame(capture);
		cvShowImage("Original", frame);
		QueryPerformanceCounter((PLARGE_INTEGER)&start);
		rainbowFilter(frame, filtered);
		cvShowImage("Filtered", filtered);
		time = getTime(start);
		cout << (1000/time) << endl;
	}

	cvReleaseCapture(&capture);
	cvDestroyWindow("Original");
	cvDestroyWindow("Filtered");

	_getch();
	return 0;
}