//
// Created by Shiina Miyuki on 2019/2/28.
//

#ifndef MIYUKI_IO_H
#define MIYUKI_IO_H

#include "miyuki.h"
namespace Miyuki{
    namespace IO{
        void readUnderPath(const std::string &filename, std::function<void(const std::string &)>);
		std::string GetOpenFileNameWithDialog(const char* filter=nullptr);
		std::string GetSaveFileNameWithDialog(const char* filter = nullptr);
    }
}
#endif //MIYUKI_IO_H
