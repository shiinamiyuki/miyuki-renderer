//
// Created by Shiina Miyuki on 2019/2/28.
//

#include <io/io.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>


#include <locale>
#include <string>

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}


namespace Miyuki{
    namespace IO{
        void readUnderPath(const std::string &filename, std::function<void(const std::string &)> f){
            cxx::filesystem::path currentPath = cxx::filesystem::current_path();

            cxx::filesystem::path inputFile(filename);
            auto file = inputFile.filename().string();
            auto parent = inputFile.parent_path();
            if (!parent.empty())
                cxx::filesystem::current_path(parent);

            f(file);
            cxx::filesystem::current_path(currentPath);
        }
		//std::string GetOpenFileNameWithDialog(const wchar_t* filter)	{
		//	wchar_t filename[MAX_PATH];

		//	OPENFILENAME ofn;
		//	ZeroMemory(&filename, sizeof(filename));
		//	ZeroMemory(&ofn, sizeof(ofn));
		//	ofn.lStructSize = sizeof(ofn);
		//	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
		//	if (filter)
		//		ofn.lpstrFilter = filter;
		//	else
		//		ofn.lpstrFilter = L"Image\0*.png;*.jpg\0Text Files\0*.txt\0Any File\0*.*\0";
		//	ofn.lpstrFile = filename;
		//	ofn.nMaxFile = MAX_PATH;
		//	ofn.lpstrTitle = L"Select a File";
		//	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
		//	if(GetOpenFileName(&ofn))
		//		return wstring_to_utf8(filename);
		//	return "";
		//}
    }
}