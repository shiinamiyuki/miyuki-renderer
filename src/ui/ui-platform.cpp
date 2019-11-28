// MIT License
// 
// Copyright (c) 2019 椎名深雪
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include <api/defs.h>
#include <api/ui/ui.h>


#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
namespace miyuki::ui {

    std::string GetOpenFileNameWithDialog(const char *filter) {
        CurrentPathGuard guard;
        char filename[MAX_PATH];

        OPENFILENAME ofn;
        ZeroMemory(&filename, sizeof(filename));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL; // If you have a window to center over, put its HANDLE here
        if (filter)
            ofn.lpstrFilter = filter;
        else
            ofn.lpstrFilter = "Image\0*.png;*.jpg\0Text Files\0*.txt\0Any File\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = "Select a File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
        if (GetOpenFileName(&ofn))
            return filename; // wstring_to_utf8(filename);
        return "";
    }

    std::string GetSaveFileNameWithDialog(const char *filter) {
        CurrentPathGuard guard;
        char filename[MAX_PATH];

        OPENFILENAME ofn;
        ZeroMemory(&filename, sizeof(filename));
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL; // If you have a window to center over, put its HANDLE here
        if (filter)
            ofn.lpstrFilter = filter;
        else
            ofn.lpstrFilter = "Image\0*.png;*.jpg\0Text Files\0*.txt\0Any File\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrTitle = "Select a File";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;
        if (GetSaveFileName(&ofn))
            return filename; // wstring_to_utf8(filename);
        return "";
    }
} // namespace miyuki::ui

#endif