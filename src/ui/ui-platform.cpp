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