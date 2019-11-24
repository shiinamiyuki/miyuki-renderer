#pragma once

#include <api/defs.h>
#include <api/image.hpp>

namespace miyuki {
    class ImageLoader {
        class Impl;
        std::shared_ptr<Impl> impl;
      public:
        ImageLoader();
        std::shared_ptr<RGBAImage> loadRGBAImage(const fs::path &);
    };
} // namespace miyuki