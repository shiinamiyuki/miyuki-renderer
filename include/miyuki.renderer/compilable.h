#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <vector>
namespace miyuki {
    struct ShaderFunctionArg {
        std::string type;
        std::string name;
    };
    struct ShaderFunctionSignature {
        std::vector<ShaderFunctionArg> args;
        std::string returnType;
        bool operator==(const ShaderFunctionSignature &rhs) const {
            if (args.size() != rhs.args.size()) {
                return false;
            }
            return std::equal(args.begin(), args.end(), rhs.args.begin(),
                              [](const ShaderFunctionArg &a, const ShaderFunctionArg &b) {
                                  return a.type == b.type && a.name == b.name;
                              });
        }
    };


    struct ShaderFunction {
        ShaderFunctionSignature signature;
        std::string name;
        std::string source;
        std::string generate() const;
    };

    class Compilable {
      public:
        virtual std::optional<ShaderFunction> compile(const ShaderFunctionSignature &signature) = 0;
    };
} // namespace miyuki