#include <fmt/format.h>
#include <miyuki.renderer/compilable.h>

namespace miyuki {
    std::string ShaderFunction::generate() const {
        auto s = fmt::format("{} {}(", signature.returnType, name);
        for (int i = 0; i < signature.args.size(); i++) {
            s.append(fmt::format("{} {}", signature.args[i].type, signature.args[i].name));
            if (i != signature.args.size() - 1) {
                s.append(",");
            }
        }
        s.append("){").append(source).append("}");
        return s;
    }
} // namespace miyuki