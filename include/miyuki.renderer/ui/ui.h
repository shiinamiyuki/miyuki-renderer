#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace miyuki::ui {
    std::string GetOpenFileNameWithDialog(const char *filter);
    std::string GetSaveFileNameWithDialog(const char *filter);
    class AbstractMainWindow {
        class Impl;
        Impl *impl = nullptr;

      public:
        AbstractMainWindow(int width, int height, const std::string &title);
        void show();
        virtual void update() = 0;
        virtual ~AbstractMainWindow();
    };

    std::shared_ptr<AbstractMainWindow> MakeMainWindow(int width, int height, const std::string &title);

} // namespace miyuki::ui