//
// Created by Shiina Miyuki on 2019/2/10.
//

#ifndef MIYUKI_JSONPARSER_HPP
#define MIYUKI_JSONPARSER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <exception>
#include <memory>
#include <thirdparty/fmt/format.h>

namespace Miyuki {
    namespace Json {
        class JsonObject;

        class BadElementType : public std::runtime_error {
        public:
            BadElementType(const std::string &msg) : std::runtime_error(msg) {}
        };

        class JsonObject {
            enum Type {
                Bool, Int, Float, String, Array, Object
            };
            Type type;

        public:
            using ObjectT = std::unordered_map<std::string, JsonObject>;
            using ArrayT = std::vector<JsonObject>;
        private:
            std::shared_ptr<ObjectT> asObject;
            std::shared_ptr<ArrayT> asArray;
            std::string asString;
            int asInt;
            float asFloat;
            bool asBool;

        public:
            JsonObject() : type(Int), asInt(0) {}

            static JsonObject makeObject() {
                JsonObject object;
                object.type = Object;
                object.asObject = std::make_shared<ObjectT>();
                return object;
            }

            static JsonObject makeArray() {
                JsonObject object;
                object.type = Array;
                object.asArray = std::make_shared<ArrayT>();
                return object;
            }

            JsonObject(bool i) : type(Bool), asBool(i) {}

            JsonObject(int i) : type(Int), asInt(i) {}

            JsonObject(float x) : type(Float), asFloat(x) {}

            JsonObject(const std::string &s) : type(String), asString(s) {}

            bool isArray() const { return type == Array; }

            bool isInt() const { return type == Int; }

            bool isFloat() const { return type == Float; }

            bool isObject() const { return type == Object; }

            bool isString() const { return type == String; }

            bool isBool() const { return type == Bool; }

            bool operator==(const JsonObject &rhs) const {
                if (type != rhs.type)return false;
                if (isInt()) {
                    return rhs.asInt == asInt;
                } else if (isFloat()) {
                    return rhs.asFloat == asFloat;
                } else if (isObject()) {
                    return asObject == rhs.asObject;
                } else if (isArray()) {
                    return asArray == rhs.asArray;
                } else if (isString()) {
                    return asString == rhs.asString;
                }
                return false;
            }

            const char *typeStr() const {
                if (isInt()) {
                    return "int";
                } else if (isFloat()) {
                    return "float";
                } else if (isObject()) {
                    return "object";
                } else if (isArray()) {
                    return "array";
                } else if (isString()) {
                    return "string";
                }
                return "unknown";
            }

            std::string toString(int level = 0) const {
                std::ostringstream out;
                if (isInt()) {
                    out << asInt;
                } else if (isFloat()) {
                    out << asFloat;
                } else if (isObject()) {
                    out << "{\n";
                    int cnt = 0;
                    for (const auto &p : *asObject) {
                        for (int i = 0; i < level + 1; i++)
                            out << "  ";
                        out << "\"" << p.first << "\" :" << p.second.toString(level + 2);
                        if (cnt != asObject->size() - 1) {
                            out << ",";
                        }
                        out << "\n";
                        cnt++;
                    }
                    for (int i = 0; i < level; i++)
                        out << "  ";
                    out << "}";
                } else if (isArray()) {
                    out << "[\n";
                    int cnt = 0;
                    for (const auto &i: *asArray) {
                        for (int i = 0; i < level + 1; i++)
                            out << "  ";
                        out << i.toString(level + 2);
                        if (cnt != asArray->size() - 1)
                            out << ",";
                        out << "\n";
                        cnt += 1;
                    }
                    for (int i = 0; i < level; i++)
                        out << "  ";
                    out << "]";

                } else if (isString()) {
                    out << "\"";
                    for (auto i:asString) {
                        if (i == '\"') {
                            out << R"(\")";
                        } else {
                            out << i;
                        }
                    }
                    out << "\"";
                } else if (isBool()) {
                    out << asBool;
                } else {
                    out << "undefined";
                }
                return out.str();
            }

            int getInt() const {
                if (!isInt() && !isFloat()) {
                    throw BadElementType(std::string("Expected int but have ").append(typeStr()));
                }
                if (isInt())
                    return asInt;
                else return asFloat;
            }

            void setInt(int v) {
                if (!isInt() && !isFloat()) {
                    throw BadElementType(std::string("Expected int but have ").append(typeStr()));
                }
                asInt = v;
                type = Int;
            }

            void setFloat(int v) {
                if (!isInt() && !isFloat()) {
                    throw BadElementType(std::string("Expected int but have ").append(typeStr()));
                }
                asFloat = v;
                type = Float;
            }

            bool getBool() const {
                if (isInt()) {
                    return asInt != 0;
                }
                if (isFloat()) {
                    return asFloat != 0;
                }
                if (!isBool()) {
                    throw BadElementType(std::string("Expected bool but have ").append(typeStr()));
                }
                return asBool;
            }

            float getFloat() const {
                if (!isInt() && !isFloat()) {
                    throw BadElementType(std::string("Expected float but have ").append(typeStr()));
                }
                if (isInt())
                    return asInt;
                else return asFloat;
            }

            const ArrayT &getArray() const {
                if (!isArray()) {
                    throw BadElementType(std::string("Expected array but have ").append(typeStr()));
                }
                return *asArray;
            }

            const ObjectT &getObject() const {
                if (!isObject()) {
                    throw BadElementType(std::string("Expected object but have ").append(typeStr()));
                }
                return *asObject;
            }

            const std::string &getString() const {
                if (!isString()) {
                    throw BadElementType(std::string("Expected string but have ").append(typeStr()));
                }
                return asString;
            }

            ArrayT &getArray() {
                if (!isArray()) {
                    throw BadElementType(std::string("Expected array but have ").append(typeStr()));
                }
                return *asArray;
            }

            ObjectT &getObject() {
                if (!isObject()) {
                    throw BadElementType(std::string("Expected object but have ").append(typeStr()));
                }
                return *asObject;
            }

            std::string &getString() {
                if (!isString()) {
                    throw BadElementType(std::string("Expected string but have ").append(typeStr()));
                }
                return asString;
            }

            bool hasKey(const std::string &key) const {
                auto map = getObject();
                return map.find(key) != map.end();
            }

            JsonObject &get(const std::string &key) {
                auto &t = getObject();
                auto iter = t.find(key);
                if (t.end() == iter) {
                    std::cerr << "Cannot find key " << key << std::endl;
                    throw std::runtime_error("Key error");
                }
                return iter->second;
            }

            JsonObject getDefault(const std::string &key, const JsonObject &value) const {
                auto iter = getObject().find(key);
                if (iter != getObject().end()) {
                    return std::move(iter->first);
                }
                return value;
            }

            void setIfHasNotKey(const std::string &key, const JsonObject &value) {
                auto iter = getObject().find(key);
                if (iter != getObject().end()) {
                    return;
                }
                getObject()[key] = value;
            }

            const JsonObject &operator[](const std::string &key) const {
                const auto &t = getObject();
                auto iter = t.find(key);
                if (t.end() == iter) {
                    std::cerr << "Cannot find key " << key << std::endl;
                    throw std::runtime_error("Key error");
                }
                return iter->second;
            }

            JsonObject &operator[](const std::string &key) {
                return getObject()[key];
            }

            const JsonObject &operator[](const uint32_t index) const {
                return getArray()[index];
            }

            JsonObject &operator[](const uint32_t index) {
                return getArray()[index];
            }
        };

        class ParserError : public std::runtime_error {
        public:
            ParserError(const std::string &msg) : std::runtime_error(msg) {}
        };

        class JsonParser {
            const std::string &src;
            int pos;
            int line = 1, col = 1;

            int get(int idx) { return idx >= src.length() ? 0 : src[idx]; }

            int cur() { return get(pos); }

            int next() { return get(pos + 1); }

            void advance() {
                if (cur() == '\n') {
                    line += 1;
                    col = 1;
                } else
                    col++;
                pos++;
            }

        public:
            JsonParser(const std::string &source) : src(source), pos(0) {}

            void skipSpace() {
                while (isspace(cur()))advance();
            }

            JsonObject parseNumber() {
                int sign = 1;
                if (cur() == '-') {
                    sign = -1;
                    advance();
                } else if (cur() == '+') {
                    sign = 1;
                    advance();
                }
                double x = 0;
                while (isdigit(cur())) {
                    x = 10 * x + cur() - '0';
                    advance();
                }
                if (cur() == '.') {
                    advance();
                    double p = 0.1;
                    while (isdigit(cur())) {
                        x = x + p * (cur() - '0');
                        p *= 0.1;
                        advance();
                    }
                }
                x *= sign;
                if(cur() == 'e'){
                    advance();
                    if(cur() == '-'){
                        sign = -1;
                        advance();
                    } else
                        sign = 1;
                    int p = 0;
                    while (isdigit(cur())) {
                        p = 10 * p + cur() - '0';
                        advance();
                    }
                    x = x * std::pow(10.0, sign * p);
                }
                if ((int) x == x) {
                    return JsonObject(int(x));
                }
                return JsonObject{(float) x};
            }

            JsonObject parseString() {
                if (cur() != '\"') {
                    throw ParserError(fmt::format("'\\\"' expected at {}:{}\n", line, col));
                }
                std::string s;
                advance();
                while (cur() && cur() != '\"') {
                    if (cur() == '\\') {
                        advance();
                        if (cur() == '\"')
                            s += '\"';
                        else if (cur() == '\\')
                            s += '\\';
                        else if (cur() == 'r')
                            s += '\r';
                        else if (cur() == 't')
                            s += '\t';
                        else if (cur() == 'b')
                            s += '\b';
                        else if (cur() == 'f')
                            s += '\f';
                        else if (cur() == 'n')
                            s += '\n';
                    } else
                        s += (char) cur();
                    advance();
                }
                if (cur() != '\"') {
                    throw ParserError(fmt::format("'\\\"' expected at {}:{}\n", line, col));
                }
                advance();
                return JsonObject(s);
            }

            JsonObject parseArray() {
                if (cur() != '[') {
                    throw ParserError(fmt::format("'[' expected at {}:{}\n", line, col));
                }
                advance();
                skipSpace();
                auto object = JsonObject::makeArray();
                while (cur() && cur() != ']') {
                    skipSpace();
                    object.getArray().emplace_back(parse());
                    skipSpace();
                    if (cur() == ',')
                        advance();
                    skipSpace();
                }
                if (cur() != ']') {
                    throw ParserError(fmt::format("']' expected at {}:{}\n", line, col));
                }
                advance();
                return object;
            }

            JsonObject parseObject() {
                if (cur() != '{') {
                    throw ParserError(fmt::format("'{{' expected at {}:{}\n", line, col));
                }
                advance();
                skipSpace();
                auto object = JsonObject::makeObject();
                while (cur() && cur() != '}') {
                    skipSpace();
                    auto key = parseString();
                    skipSpace();
                    if (cur() != ':') {
                        throw ParserError(fmt::format("':' expected at {}:{}\n", line, col));
                    }
                    advance();
                    skipSpace();
                    auto val = parse();
                    if (object.hasKey(key.getString())) {
                        std::cerr << "duplicated key " << key.getString() << std::endl;
                    }
                    object[key.getString()] = val;
                    if (cur() == ',')
                        advance();
                    skipSpace();
                }
                if (cur() != '}') {
                    throw ParserError(fmt::format("'}}' expected at {}:{}\n", line, col));
                }
                advance();
                return object;
            }


            JsonObject parse() {
                skipSpace();
                if (cur() == '"') {
                    return parseString();
                } else if (isdigit(cur()) || cur() == '+' || cur() == '-') {
                    return parseNumber();
                } else if (cur() == '[')
                    return parseArray();
                else if (cur() == '{')
                    return parseObject();
                else if (cur() == 't' && next() == 'r' && get(pos + 2) == 'u' && get(pos + 3) == 'e') {
                    pos += 4;
                    return JsonObject(true);
                } else if (cur() == 'f' && next() == 'a' && get(pos + 2) == 'l' && get(pos + 3) == 's' &&
                           get(pos + 4) == 'e') {
                    pos += 5;
                    return JsonObject(false);
                }
                throw ParserError("");
            }

            JsonObject parseToplevel() {
                skipSpace();
                if (cur() == '{') {
                    try {
                        return parseObject();
                    } catch (Json::BadElementType &e) {
                        fmt::print(stderr, "Parser Internal Error when processing {}:{}\n", line, col);
                        std::cerr << e.what() << std::endl;
                    } catch (Json::ParserError &e) {
                        fmt::print(stderr, "Parser Internal Error when processing {}:{}\n", line, col);
                        std::cerr << e.what() << std::endl;
                    }
                }
                throw ParserError(fmt::format("'{{' expected at {}:{}", line, col));
            }
        };

        inline JsonObject parse(const std::string &source) {
            return JsonParser(source).parseToplevel();
        }
    }
}


#endif //MIYUKI_JSONPARSER_HPP
