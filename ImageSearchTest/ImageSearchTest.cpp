// ImageSearchTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ImageSearchLibrary.h"
#include <atlstr.h>
#include <iostream>
#include <Windows.h>

using namespace std;

vector<CString> getFolderFiles(CString folder)
{
	vector<CString> names;
	CString search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				names.push_back(folder + "\\" + fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

int main() {
	/*CString testImageLocation = "C:\\mslimages\\FullMonInv.bmp";
	vector<CString> files = getFolderFiles(_T("C:\\mslimages\\manage"));
	for (int i = 0; i < 25; i++) {
		for (CString img : files)
		{
			int len = wcslen(testImageLocation.GetBuffer()) + 1;
			char* loc1 = new char[len];
			wsprintfA(loc1, "%S", testImageLocation.GetBuffer());
			len = wcslen(img.GetBuffer()) + 1;
			char* loc2 = new char[len];
			wsprintfA(loc2, "%S", img.GetBuffer());
			const wchar_t* tmp = FindImage(loc1, loc2, 90, false, true, true);
			wcout << (const wchar_t*)tmp << endl;
		}
	}*/
	//CString tmp = CString(FindImage("C:\\mslimages\\map1.png", "C:\\mslimages\\map-astromon-league2.bmp", 90, false, false, true));
	//CString tmp = CString(TrainDecodeNumerals("G:\\MSL\\testimages\\font2.bmp"));
	//CString tmp = CString(TrainDecodeNumerals("G:\\MSL\\testimages\\font2.bmp"));
	//CString tmp = CString(DecodeNumerals("G:\\MSL\\testimages\\test2.bmp"));
	CString tmp = CString(DecodeNumerals("G:\\MSL\\testimages\\testgem.bmp"));
	wcout << (const wchar_t*)tmp << endl;
	//cout << "\n";

	cin.get();

	return 0;
}