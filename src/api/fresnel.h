#pragma once

#include <api/math.hpp>
#include <api/object.hpp>

namespace miyuki::core {
    class Frensel : public Object {
      public:
        virtual Float evaluate(const Float cosTheta) const = 0;
    };
} // namespace miyuki::core