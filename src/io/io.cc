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


namespace Miyuki{
    namespace IO{
		struct CurrentPathSaver {
			CurrentPathSaver() {
				current = cxx::filesystem::current_path();
			}
			~CurrentPathSaver() {
				cxx::filesystem::current_path(current);
			}
		private:
			cxx::filesystem::path current;
		};
        void readUnderPath(const std::string &filename, std::function<void(const std::string &)> f){
			CurrentPathSaver saver;

            cxx::filesystem::path inputFile(filename);
            auto file = inputFile.filename().string();
            auto parent = inputFile.parent_path();
            if (!parent.empty())
                cxx::filesystem::current_path(parent);

            f(file);

        }
		std::string GetOpenFileNameWithDialog(const char* filter)	{
			CurrentPathSaver saver;
			char filename[MAX_PATH];

			OPENFILENAME ofn;
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
			if (filter)
				ofn.lpstrFilter = filter;
			else
				ofn.lpstrFilter = "Image\0*.png;*.jpg\0Text Files\0*.txt\0Any File\0*.*\0";
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = "Select a File";
			ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
			if (GetOpenFileName(&ofn))
				return filename;// wstring_to_utf8(filename);
			return "";
		}

		std::string GetSaveFileNameWithDialog(const char* filter) {
			CurrentPathSaver saver;
			char filename[MAX_PATH];

			OPENFILENAME ofn;
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
			if (filter)
				ofn.lpstrFilter = filter;
			else
				ofn.lpstrFilter = "Image\0*.png;*.jpg\0Text Files\0*.txt\0Any File\0*.*\0";
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrTitle = "Select a File";
			ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
			if (GetSaveFileName(&ofn))
				return filename;// wstring_to_utf8(filename);
			return "";
		}
    }
}