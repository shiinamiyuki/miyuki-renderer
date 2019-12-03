#pragma once

#include "material.h"
#include "shader.h"

namespace miyuki::core {
    class SceneGraph final : public Object {
        std::vector<std::shared_ptr<Material>> materials;
        std::vector<std::shared_ptr<Shader>> shaders;
      public:
    };
} // namespace miyuki::core