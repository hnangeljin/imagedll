// ImageSearchLibrary.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ImageSearchLibrary.h"
#include "opencv2/core.hpp"
#include "opencv2/ml.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <atlstr.h>

using namespace cv;
using namespace ml;
using namespace std;

const int MIN_CONTOUR_AREA = 10;

const int RESIZED_IMAGE_WIDTH = 30;
const int RESIZED_IMAGE_HEIGHT = 30;
#pragma region Helper Functions

Mat HelperFunctions::ReadImageFromFile(const char* imagePath, bool grayscale) {
	string s = imagePath;
	if (grayscale)
		return imread(s, IMREAD_GRAYSCALE);
	else
		return imread(s, IMREAD_COLOR);
}

Mat HelperFunctions::ReadImageFromBuffer(HANDLE imageBuf, int width, int height, bool grayscale)
{
	// create a bitmap
	BITMAPINFOHEADER  bi;
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	HBITMAP hbwindow = (HBITMAP)imageBuf;
	HDC hwindowDC = GetDC(NULL);
	HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	Mat src = Mat(height, width, CV_8UC4);

	SelectObject(hwindowCompatibleDC, hbwindow);

	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow
	if (grayscale)
		cvtColor(src, src, COLOR_BGRA2GRAY);
	else {
		cvtColor(src, src, COLOR_BGRA2BGR);
	}

	DeleteDC(hwindowCompatibleDC); ReleaseDC(NULL, hwindowDC);

	return src;
}

CString HelperFunctions::FindImageInImage(const char* fullImage, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask, const char* searchMask, bool multiple, bool debug) {
	Mat fullImageMat, searchImageMat;
	fullImageMat = ReadImageFromFile(fullImage, grayscale);
	searchImageMat = ReadImageFromFile(searchImage, grayscale);

	CString points = findImage(fullImageMat, searchImageMat, tolerance, grayscale, useSearchMask, searchMask, multiple, debug);

	if (!fullImageMat.empty())
		fullImageMat.release();
	if (!searchImageMat.empty())
		searchImageMat.release();
	return points;
}

CString HelperFunctions::findImage(Mat fullImageMat, Mat searchImageMat, int tolerance, bool grayscale, bool useSearchMask, const char* searchMask, bool multiple, bool debug) {
	Mat matchResults;
	Mat searchMaskMat;
	if (useSearchMask)
		searchMaskMat = ReadImageFromFile(searchMask, grayscale);
	CString points = "0";
	if (debug) {
		imwrite("C:\\mslimages\\test.bmp", fullImageMat);
		imwrite("C:\\mslimages\\test_search.bmp", searchImageMat);
		if (useSearchMask)
			imwrite("C:\\mslimages\\test_search_mask.bmp", searchMaskMat);
	}

	if (multiple) {
		if (useSearchMask) {
			matchTemplate(fullImageMat, searchImageMat, matchResults, TM_SQDIFF, searchMaskMat);
			threshold(matchResults, matchResults, 20, 255, THRESH_BINARY_INV);
			points = ParseMultipleImagesFoundMask(matchResults, tolerance);
			if (points != "0") {
				CString tmp;
				tmp.Format(_T("|%d|%d"), searchImageMat.cols, searchImageMat.rows);
				points.Insert(points.Find(_T("|"), 0), tmp);
			}
		}
		else {
			matchTemplate(fullImageMat, searchImageMat, matchResults, TM_CCOEFF_NORMED);
			if (debug) {
				imwrite("C:\\mslimages\\results.bmp", matchResults);
			}
			points = ParseMultipleImagesFound(matchResults, tolerance);
			if (points != "0") {
				CString tmp;
				tmp.Format(_T("|%d|%d"), searchImageMat.cols, searchImageMat.rows);
				points.Insert(points.Find(_T("|"), 0), tmp);
			}
		}
	}
	else {
		matchTemplate(fullImageMat, searchImageMat, matchResults, TM_CCOEFF_NORMED);
		double min_val, max_val;
		Point min_loc, max_loc;
		minMaxLoc(matchResults, &min_val, &max_val, &min_loc, &max_loc);
		if (max_val * 100 >= tolerance) {
			points.Format(_T("1|%d|%d|%d|%d|%d"), searchImageMat.size().width, searchImageMat.size().height, max_loc.x, max_loc.y, (int)(max_val * 100));
		}
	}

	if (!fullImageMat.empty())
		fullImageMat.release();
	if (!searchImageMat.empty())
		searchImageMat.release();
	return points;
}

CString HelperFunctions::FindImageInImageEX(HANDLE fullImage, int width, int height, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask, const char* searchMask, bool multiple, bool debug) {
	Mat fullImageMat, searchImageMat, matchResults;
	fullImageMat = ReadImageFromBuffer(fullImage, width, height, grayscale);
	searchImageMat = ReadImageFromFile(searchImage, grayscale);
	CString points = findImage(fullImageMat, searchImageMat, tolerance, grayscale, useSearchMask, searchMask, multiple, debug);
	return points;
}

CString HelperFunctions::ParseMultipleImagesFound(Mat matchResults, const int tolerance)
{
	CString locations = "";
	double min_val, max_val;
	Point min_loc, max_loc;
	int count = 0;
	minMaxLoc(matchResults, &min_val, &max_val, &min_loc, &max_loc);
	while (max_val * 100 >= tolerance) {
		locations.Format(_T("%s%d|%d|%d|"), locations, max_loc.x, max_loc.y, (int)(max_val * 100));
		floodFill(matchResults, max_loc, Scalar(0), 0, Scalar(.1), Scalar(1.));
		minMaxLoc(matchResults, &min_val, &max_val, &min_loc, &max_loc);
		count++;
	}
	if (count > 0)
		locations.Format(_T("%d|%s"), count, locations.Left(locations.GetLength() - 1));
	else
		locations = "0";
	if (!matchResults.empty())
		matchResults.release();
	return locations;
}

CString HelperFunctions::ParseMultipleImagesFoundMask(Mat matchResults, const int tolerance)
{
	CString locations = "";
	threshold(matchResults, matchResults, tolerance / 100, 1., THRESH_BINARY);
	Mat1b resB;
	matchResults.convertTo(resB, CV_8U, 255);

	vector<vector<Point>> contours;
	findContours(resB, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); ++i) {
		Mat1b mask(matchResults.rows, matchResults.cols, uchar(0));
		drawContours(mask, contours, i, Scalar(255), CV_FILLED);

		Point matchPoint;
		double matchVal;

		minMaxLoc(matchResults, NULL, &matchVal, NULL, &matchPoint, mask);

		locations.Format(_T("%s%d|%d|%d|"), locations, matchPoint.x, matchPoint.y, (int)(matchVal * 100));
	}

	if (contours.size() > 0)
		locations.Format(_T("%d|%s"), contours.size(), locations.Left(locations.GetLength() - 1));
	else
		locations = "0";
	if (!matchResults.empty())
		matchResults.release();
	return locations;
}

#pragma endregion

#pragma region OCR

Mat HelperFunctions::ThresholdImage(Mat Image, bool grayscale, bool debug) {
	Mat thresholdMat, blurMat, srcMat;

	Image.copyTo(srcMat);

	if (!grayscale)
		cvtColor(srcMat, srcMat, CV_BGR2GRAY);

	if (srcMat.at<uchar>(Point(1, 1)) > 127)
		bitwise_not(srcMat, srcMat);

	GaussianBlur(srcMat,				// input image
		blurMat,						// output image
		Size(0, 0),						// smoothing window width and height in pixels
		3);								// sigma value, determines how much the image will be blurred, zero makes function choose the sigma value

	
	//Threshold to find contour
	addWeighted(srcMat, 1.2, blurMat, -0.5, 0, blurMat);
	if (debug)
		imwrite("G:\\MSL\\testimages\\OcrOutput\\blur.bmp", blurMat);
	threshold(blurMat, thresholdMat, 100, 255, THRESH_BINARY);

	if (debug)
		imwrite("G:\\MSL\\testimages\\OcrOutput\\thr.bmp", thresholdMat);

	return thresholdMat;
}

CString HelperFunctions::TrainNumerals(Mat Image) {

	Mat matSource,								// input image
		matResized,								// resized image
		matThresh,								// binary threshold image
		matThreshCopy,							//
		matSamples,								// these are our training images, due to the data types that the KNN object KNearest requires,
												// we have to declare a single Mat, then append to it as though it's a vector,
												// also we will have to perform some conversions before writing to file later
		matResponses;							// these are our training classifications, note we will have to perform some conversions before writing to file later

	vector<vector<Point>> ptContours;			// declare contours vector
	vector<Vec4i> v4iHierarchy;					// declare contours hierarchy

	// copy our input image to a mat to be used in the function
	Image.copyTo(matSource);

	// check if the input image is empty
	if (matSource.empty()) return("Error: Input image is blank");

	// resize the input image for better clarity
	resize(matSource, matResized, Size(matSource.size().width * 2, matSource.size().height * 2));

	// Blur and convert the image to binary black and white
	matThresh = ThresholdImage(matResized, !(matResized.channels() > 1), true);

	imshow("matThresh", matThresh);			// show threshold image for reference

	matThreshCopy = matThresh.clone();			// make a copy of the thresh image, this in necessary b/c findContours modifies the image

	findContours(matThreshCopy,				// input image, make sure to use a copy since the function will modify this image in the course of finding contours
		ptContours,								// output contours
		v4iHierarchy,							// output hierarchy
		CV_RETR_CCOMP,						// retrieve the outermost contours only
		CHAIN_APPROX_SIMPLE);				// compress horizontal, vertical, and diagonal segments and leave only their end points
	vector<ContourWithData>	validContoursWithData;
	// For each top level contour
	for (int i = 0; i < ptContours.size(); i = v4iHierarchy[i][0]) {
		// If the contour is big enough
		ContourWithData contourWithData(ptContours[i]);
		// add contour with data object to list of all contours with data
		if (contourWithData.checkIfContourIsValid(matThresh))
			validContoursWithData.push_back(contourWithData);
	}

	sort(validContoursWithData.begin(), validContoursWithData.end(), ContourWithData::sortByBoundingRectXPosition);

	for (int i = 0; i < validContoursWithData.size(); i++) {			// for each contour
		// Get the bounding Rectangle
		Rect boundRect = validContoursWithData[i].boundRect;
			
		// Expand the bounding rectangle to be able to see the image.
		Rect modBoundRect = Rect(boundRect.x - 2, boundRect.y - 2, boundRect.width + 4, boundRect.height + 4);
		Mat matROI,							
			matROIResized,
			matImageFloat,
			matImageReshaped;

		// draw red rectangle around each contour as we ask user for input
		rectangle(matResized, modBoundRect, Scalar(0, 0, 255), 2);

		// get ROI image of bounding rect
		matROI = matThresh(boundRect);

		// resize image, this will be more consistent for recognition and storage
		resize(matROI, matROIResized, Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));

		// show training numbers image, this will now have red rectangles drawn on it
		imshow("training Image", matResized);		

		// get key press
		int intChar = waitKey(0);

		// if esc key was pressed exit function
		if (intChar == 27) return("");
		if (intChar == 8) continue;

		// append response character to integer list of chars(we will convert later before writing to file)
		matResponses.push_back(intChar);

		// Convert the image to float values for the KNearest training
		matROIResized.convertTo(matImageFloat, CV_32FC1);

		// Flatten the sample image
		matImageReshaped = matImageFloat.reshape(1, 1);

		// add to Mat as though it was a vector, this is necessary due to the data types that KNearest.train accepts
		matSamples.push_back(matImageReshaped);

		// Change the rectangle on the image to a green one after getting the response
		rectangle(matResized, modBoundRect, Scalar(0, 255, 0), 2);

	}

	// save responses to file ///////////////////////////////////////////////////////
	// convert responses to floats
	matResponses.convertTo(matResponses, CV_32FC1);

	// open the responses file
	FileStorage responses("Responses.yml", FileStorage::WRITE);			

	// if the file was not opened successfully exit the function
	if (responses.isOpened() == false) return("error, unable to open training responses file, exiting program");

	// write responses into responses section of responses file
	responses << "responses" << matResponses;		
	responses.release();

	// save sample images to file ///////////////////////////////////////////////////////
	// open the sample images file
	FileStorage samples("Samples.yml", FileStorage::WRITE);			

	// if the file was not opened successfully exit the function
	if (samples.isOpened() == false) return("error, unable to open training samples file, exiting program\n\n");

	// write sample images into images section of images file
	samples << "samples" << matSamples;		
	samples.release();

	return "Finished";
}

CString HelperFunctions::DecodeNumeralsMat(Mat Image) {
	CString output;
	// declare empty vector we will fill these shortly
	vector<ContourWithData>	validContoursWithData;

	// read in training classifications ///////////////////////////////////////////////////

	Mat matSource,								// input image
		matResized,								// resized image
		matThresh,								// binary threshold image
		matThreshCopy,							//
		matSamples,								// these are our training images, due to the data types that the KNN object KNearest requires,
												// we have to declare a single Mat, then append to it as though it's a vector,
												// also we will have to perform some conversions before writing to file later
		matResponses;							// these are our training classifications, note we will have to perform some conversions before writing to file later

	FileStorage responses("Responses.yml", FileStorage::READ);		// open the classifications file

	// if the file was not opened successfully exit the function
	if (responses.isOpened() == false) return("error, unable to open training responses file, exiting program");

	responses["responses"] >> matResponses;		// read classifications section into Mat classifications variable
	responses.release();											// close the classifications file

																			// read in training images ////////////////////////////////////////////////////////////

	FileStorage samples("Samples.yml", FileStorage::READ);			// open the training images file

	// if the file was not opened successfully exit the function
	if (samples.isOpened() == false) return("error, unable to open training samples file, exiting program\n\n");

	samples["samples"] >> matSamples;				// read images section into Mat training images variable
	samples.release();										// close the traning images file

	Ptr<KNearest> kNearest(KNearest::create());

	Ptr<TrainData> trainData = TrainData::create(matSamples, SampleTypes::ROW_SAMPLE, matResponses);
	kNearest->train(trainData); // Train with sample and responses

	Image.copyTo(matSource);
	// check if the input image is empty
	if (matSource.empty()) return("Error: Input image is blank");

	resize(matSource, matResized, Size(matSource.size().width * 2, matSource.size().height * 2));


	// Blur and convert the image to binary black and white
	matThresh = ThresholdImage(matResized, !(matResized.channels() > 1), true);

	matThreshCopy = matThresh.clone();					// make a copy of the thresh image, this in necessary b/c findContours modifies the image

	vector<vector<Point>> ptContours;		// declare a vector for the contours
	vector<Vec4i> v4iHierarchy;					// declare a vector for the hierarchy (we won't use this in this program but this may be helpful for reference)

	findContours(matThreshCopy,					// input image, make sure to use a copy since the function will modify this image in the course of finding contours
		ptContours,					// output contours
		v4iHierarchy,					// output hierarchy
		CV_RETR_CCOMP,				// retrieve the outermost contours only
		CHAIN_APPROX_SIMPLE);		// compress horizontal, vertical, and diagonal segments and leave only their end points

	for (int i = 0; i < ptContours.size(); i = v4iHierarchy[i][0]) {			// for each contour
		// instantiate a contour with data object
		ContourWithData contourWithData(ptContours[i]);
		// add contour with data object to list of all contours with data
		if (contourWithData.checkIfContourIsValid(matThresh))
			validContoursWithData.push_back(contourWithData);
	}
	// sort contours from left to right
	sort(validContoursWithData.begin(), validContoursWithData.end(), ContourWithData::sortByBoundingRectXPosition);

	// declare decoded characters string, this will have the final character sequence by the end of the program
	CString decodedChars;			

	for (int i = 0; i < validContoursWithData.size(); i++) {			// for each contour

		Mat matROI,
			matROIResized,
			matROIFloat,
			matResults;
		matResults = Mat(0, 0, CV_32F);

		Rect boundRect = validContoursWithData[i].boundRect;

		// get ROI image of bounding rect
		matROI = matThresh(boundRect);

		// resize image, this will be more consistent for recognition and storage
		resize(matROI, matROIResized, Size(RESIZED_IMAGE_WIDTH, RESIZED_IMAGE_HEIGHT));
		// convert Mat to float, necessary for call to find_nearest
		matROIResized.convertTo(matROIFloat, CV_32FC1);

		// flattened resized ROI
		matROIFloat = matROIFloat.reshape(1, 1);
		// finally we can call find_nearest 
		float fltCurrentChar = kNearest->findNearest(matROIFloat, 1, matResults);
		// append current char to decoded string
		decodedChars.AppendChar(char(int(fltCurrentChar)));
	}

	return decodedChars;
}

CString HelperFunctions::DecodeNumerals(const char* Image) {
	Mat imageMat = ReadImageFromFile(Image, false);
	return DecodeNumeralsMat(imageMat);
}

CString HelperFunctions::TrainDecodeNumerals(const char* Image) {
	Mat imageMat = ReadImageFromFile(Image, false);
	return TrainNumerals(imageMat);
}

CString HelperFunctions::DecodeNumeralsEX(HANDLE Image, int width, int height) {
	Mat imageMat = ReadImageFromBuffer(Image, width, height, true);
	return DecodeNumeralsMat(imageMat);
}

CString HelperFunctions::TrainDecodeNumeralsEX(HANDLE Image, int width, int height) {
	Mat imageMat = ReadImageFromBuffer(Image, width, height, true);
	return TrainNumerals(imageMat);
}

#pragma endregion

#pragma region Dll Exports

extern "C" DLL_EXPORT const wchar_t* DecodeNumerals(const char* Image) {
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.DecodeNumerals(Image);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}
	return outValue;
}

extern "C" DLL_EXPORT const wchar_t* TrainDecodeNumerals(const char* Image) {
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.TrainDecodeNumerals(Image);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}
	return outValue;
}

extern "C" DLL_EXPORT const wchar_t* DecodeNumeralsEX(HANDLE Image, int width, int height) {
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.DecodeNumeralsEX(Image, width, height);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}
	return outValue;
}

extern "C" DLL_EXPORT const wchar_t* TrainDecodeNumeralsEX(HANDLE Image, int width, int height) {
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.TrainDecodeNumeralsEX(Image, width, height);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}
	return outValue;
}

extern "C" DLL_EXPORT const wchar_t* FindImage(const char* fullImage, const char* searchImage, const int tolerance, bool grayscale, bool multiple, bool debug)
{
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.FindImageInImage(fullImage, searchImage, tolerance, grayscale, false, "", multiple, debug);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}

	return outValue.GetString();
}

extern "C" DLL_EXPORT const wchar_t* FindImageEX(HANDLE fullImagePtr, int width, int height, const char* searchImage, const int tolerance, bool grayscale, bool multiple, bool debug)
{
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.FindImageInImageEX(fullImagePtr, width, height, searchImage, tolerance, grayscale, false, "", multiple, debug);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}

	return outValue.GetString();
}

extern "C" DLL_EXPORT const wchar_t* FindImageWithMask(const char* fullImage, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask, const char* searchMask, bool multiple, bool debug)
{
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.FindImageInImage(fullImage, searchImage, tolerance, grayscale, useSearchMask, searchMask, multiple, debug);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}

	return outValue.GetString();
}

extern "C" DLL_EXPORT const wchar_t* FindImageEXWithMask(HANDLE fullImagePtr, int width, int height, const char* searchImage, const int tolerance, bool grayscale, bool useSearchMask, const char* searchMask, bool multiple, bool debug)
{
	HelperFunctions h;
	CString outValue = "-1";
	try {
		outValue = h.FindImageInImageEX(fullImagePtr, width, height, searchImage, tolerance, grayscale, useSearchMask, searchMask, multiple, debug);
	}
	catch (cv::Exception e) {
		outValue.Append(_T("|"));
		outValue.Append(CString(e.what()));
	}

	return outValue.GetString();
}

#pragma endregion

#pragma region Contours

bool ContourWithData::sortByBoundingRectXPosition(const ContourWithData& cwdLeft, const ContourWithData& cwdRight)
{		// this function allows us to sort
	if (cwdLeft.boundRect.x == cwdRight.boundRect.x)
		return (cwdLeft.boundRect.y < cwdRight.boundRect.y);

	return(cwdLeft.boundRect.x < cwdRight.boundRect.x);													// the contours from left to right
}


ContourWithData::ContourWithData(vector<Point> contours, Rect bRect, float area) {
	ptContour = contours;
	boundRect = bRect;
	fltArea = area;
}

ContourWithData::ContourWithData(vector<Point> contours) {
	ptContour = contours;
	// get the bounding rect
	boundRect = boundingRect(contours);
	// calculate the contour area
	fltArea = contourArea(contours);
}

bool ContourWithData::checkIfContourIsValid(Mat matThresh) {
	if (!modifyBoundRectForSpecificCharacters(matThresh))
		return false;
	return fltArea >= MIN_CONTOUR_AREA;	// identifying if a contour is valid !!
}

bool ContourWithData::modifyBoundRectForSpecificCharacters(Mat matThresh) {
	uchar tmp = 55;
	if (boundRect.width <= 7 && boundRect.height <= 7) { // check for period like character
		if (boundRect.y > 10 && matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y - 10)) == 255) // check for exclamation or question mark dot
			return false;
		if (boundRect.y > 15 && matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y - 15)) == 255) // check for colon bottom dot
			return false;
		if (boundRect.y + boundRect.height + 8 < matThresh.rows && matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y + boundRect.height + 8)) == 255) // check for lower case i or j
			return false;

		if (boundRect.y + boundRect.height + 24 < matThresh.rows) // check for colon or semi colon top dot
			boundRect.height += 24;
	}
	else if (boundRect.height <= 7 && boundRect.width >= 17) { // check for equals bottom dash
		if (boundRect.y > 8) {
			if (matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y - 8)) == 255)
				return false;
			if (boundRect.y + boundRect.height + 7 < matThresh.rows && matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y + boundRect.height + 7)) == 255)
				boundRect.height += 10;
		}
	}
	else if (boundRect.height <= 13 && boundRect.width <= 7) { // check for semi colon or double quote
		if (boundRect.y > 15) {
			if (matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y - 15)) == 255) // check for semi colon comma
				return false;
		}
		else {
			if (boundRect.width < boundRect.x) {
				if (matThresh.at<uchar>(Point(boundRect.x - boundRect.width - 2, boundRect.y + 5)) == 255) // check for right quote of double quote
					return false;
				if (matThresh.at<uchar>(Point(boundRect.x + boundRect.width + 4, boundRect.y + 5)) == 255) // check for left quote of double quote
					boundRect.width += 7;
			}
		}
	}
	else if (boundRect.height < 30 && boundRect.width < 7) { // check for lower case i or exclamation point
		if (boundRect.y > 11) {
			if (matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y - 6)) == 255) { // check for lower case i
				boundRect.y -= 11;
				boundRect.height += 11;
			}
			else if (boundRect.y + boundRect.height + 12 < matThresh.rows) {
				if (matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y + boundRect.height + 7)) == 255) // check for exclamation point
					boundRect.height += 12;
			}
		}
	}
	else if (boundRect.height > 24 && boundRect.height < 30) { // check for question mark
		if (boundRect.y + boundRect.height + 11 < matThresh.rows)
			if (matThresh.at<uchar>(Point(boundRect.x + 3, boundRect.y + boundRect.height + 7)) == 255)
				boundRect.height += 11;
	}
	else if (boundRect.height < 36 && boundRect.height > 32 && boundRect.width > 8 && boundRect.width < 12) {
		if (matThresh.at<uchar>(Point(boundRect.x + boundRect.width - 3, boundRect.y - 6)) == 255) { // check for lower case j
			boundRect.y -= 11;
			boundRect.height += 11;
		}
	}

	return true;
}

#pragma endregion

#pragma region ReturnInfo

#pragma endregion







