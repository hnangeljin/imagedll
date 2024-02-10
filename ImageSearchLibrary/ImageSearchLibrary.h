#pragma once

#include "opencv2/core/core.hpp"
#include <atlstr.h>
#include <vector>

#ifdef IMAGESEARCHLIBRARY_EXPORTS
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif

using namespace cv;

extern "C" DLL_EXPORT const wchar_t* FindImage(const char* fullImage, const char* searchImage, const int tolerance, bool grayscale, bool multiple = false, bool debug = false);
extern "C" DLL_EXPORT const wchar_t* FindImageWithMask(const char* fullImage, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask = false, const char* searchMask = "", bool multiple = false, bool debug = false);
extern "C" DLL_EXPORT const wchar_t* FindImageEX(HANDLE fullImagePtr, int width, int height, const char* searchImage, const int tolerance, bool grayscale, bool multiple = false, bool debug = false);
extern "C" DLL_EXPORT const wchar_t* FindImageEXWithMask(HANDLE fullImagePtr, int width, int height, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask = false, const char* searchMask = "", bool multiple = false, bool debug = false);
extern "C" DLL_EXPORT const wchar_t* DecodeNumerals(const char* Image);
extern "C" DLL_EXPORT const wchar_t* DecodeNumeralsEX(HANDLE Image, int width, int height);
extern "C" DLL_EXPORT const wchar_t* TrainDecodeNumerals(const char* Image);
extern "C" DLL_EXPORT const wchar_t* TrainDecodeNumeralsEX(HANDLE Image, int width, int height);
//extern "C" DLL_EXPORT LPCWSTR FindArrayImage(const char* fullImage, const char* searchImage, const int tolerance, bool multiple = false, bool debug = false);
//extern "C" DLL_EXPORT LPCWSTR FindArrayImageEX(HANDLE fullImagePtr, int width, int height, const char* searchImage, const int tolerance, bool multiple = false, bool debug = false);

class HelperFunctions {
	public:
		CString FindImageInImage(const char* fullImage, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask = false, const char* searchMask = "", bool multiple = false, bool debug = false);
		CString FindImageInImageEX(HANDLE fullImage, int width, int height, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask = false, const char* searchMask = "", bool multiple = false, bool debug = false);
		CString DecodeNumerals(const char* Image);
		CString DecodeNumeralsEX(HANDLE Image, int width, int height);
		CString TrainDecodeNumerals(const char* Image);
		CString TrainDecodeNumeralsEX(HANDLE Image, int width, int height);
	private:
		CString findImage(Mat fullImageMat, Mat searchImageMat, int tolerance, bool grayscale, bool useSearchMask, const char* searchMask, bool multiple, bool debug);
		Mat ReadImageFromFile(const char* imagePath, bool grayscale);
		Mat ReadImageFromBuffer(HANDLE imageBuf, int width, int height, bool grayscale);
		CString ParseMultipleImagesFound(Mat matchResults, const int tolerance);
		CString ParseMultipleImagesFoundMask(Mat matchResults, const int tolerance);
		CString DecodeNumeralsMat(Mat Image);
		CString TrainNumerals(Mat Image);
		Mat ThresholdImage(Mat Image, bool grayscale, bool debug);
};

class ContourWithData {
	public:
		std::vector<Point> ptContour;			// contour
		Rect boundRect;						// bounding rect for contour
		float fltArea;								// area of contour

		bool checkIfContourIsValid(Mat matThresh);
		static bool sortByBoundingRectXPosition(const ContourWithData& cwdLeft, const ContourWithData& cwdRight);
		ContourWithData(std::vector<Point> contours, Rect bRect, float area);
		ContourWithData(std::vector<Point> contours);
	private:
		bool modifyBoundRectForSpecificCharacters(Mat matThresh);

};

class ReturnInfo {
	public:
		bool Success;
		int ErrorCode;
		CString ErrorMsg;
		CString ReturnData;
		ReturnInfo() : Success(true), ErrorCode(0), ErrorMsg(""), ReturnData("") {};
		ReturnInfo(bool success, int errorCode, CString errorMsg, CString returnData) : Success(success), ErrorCode(errorCode), ErrorMsg(errorMsg), ReturnData(returnData) {};
};
//
//class ImageLocation {
//	public:
//		int numFound;
//		int xPos;
//		int yPos;		
//		int tolerance;
//		int width;
//		int height;
//		CString searchImage;
//		ImageLocation();
//
//		CString getImageLocationString(bool includeSource = false);
//};
