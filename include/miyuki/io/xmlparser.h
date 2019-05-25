#pragma once
#include <miyuki.h>

namespace Miyuki {
	namespace IO {
		struct XMLAttribute {
			std::string key;
			std::string value;

			template<typename T>
			T get()const {
				T tmp;
				Miyuki::IO::FromXMLAttribute(*this, tmp);
				return std::move(tmp);
			}

			template<>
			std::string get<std::string>()const {
				return value;
			}

			template<>
			Float get<Float>()const {
				return std::stof(value);
			}

			template<>
			int get<int>()const {
				return std::stoi(value);
			}

			template<>
			double get<double>()const {
				return std::stod(value);
			}
		};

		struct XMLText {
			std::string text;
			void trim() {
				std::string temp;
				int i = 0;
				while (i < text.length() && isspace(text[i]))i++;
				int j = text.length();
				while (j > i && isspace(text[j - 1]))j--;
				for (; i < j; i++) {
					temp += text[i];
				}
				std::swap(temp, text);
			}
		};

		class XMLNode;

		class XMLVisitor {
		public:
			virtual void visit(const XMLNode&) = 0;
		};

		struct ParserLoc {
			int line, col;
			std::string filename;
			ParserLoc() :line(0), col(0) {}
		};

		using XMLAttributeDict = std::unordered_map<std::string, XMLAttribute>;
		class XMLNode {
		private:
			std::string _name;
			XMLText _text;
			XMLAttributeDict _attributes;
			std::vector<std::shared_ptr<XMLNode>> _children;
		public:
			XMLNode() {}
			XMLNode(const std::string& name) :_name(name) {}
			XMLNode(const std::string& name,
				const XMLText& text,
				const std::unordered_map<std::string, XMLAttribute>& attributes,
				const std::vector<std::shared_ptr<XMLNode>>& children)
				:_name(name), _attributes(attributes), _children(children), _text(text) {}

			const std::string& name() const { return _name; }

			template<typename T>
			void addAttribute(const std::string& key, const T& value) {
				std::stringstream ss;
				ss << value;
				CHECK(attribute.find(key) == attributes.end());
				attributes.insert(std::make_pair(key, XMLAttribute{ key, ss.str() }));
			}

			std::optional<XMLAttribute> attr(const std::string & key)const {
				auto iter = _attributes.find(key);
				if (iter != _attributes.end())
					return { iter->second };
				return {};
			}
			const XMLAttributeDict& attributes()const { return _attributes; }

			const std::vector<std::shared_ptr<XMLNode>> children()const { return _children; }

			std::shared_ptr<XMLNode> findChild(const std::function<bool(const XMLNode&)>& pred)const {
				std::vector<std::shared_ptr<XMLNode>> vec;
				for (auto i : _children) {
					if (pred(*i)) {
						return i;
					}
				}
				return nullptr;
			}

			std::vector<std::shared_ptr<XMLNode>> findChildren(const std::function<bool(const XMLNode&)> & pred)const {
				std::vector<std::shared_ptr<XMLNode>> vec;
				for (auto i : _children) {
					if (pred(*i)) {
						vec.emplace_back(i);
					}
				}
				return std::move(vec);
			}

			std::vector<std::shared_ptr<XMLNode>> findChildrenByName(const std::string & name)const {
				return std::move(findChildren([&](const XMLNode & x) {return x.name() == name; }));
			}
			std::shared_ptr<XMLNode> child(size_t idx) {
				return _children.at(idx);
			}

			void addChild(std::shared_ptr<XMLNode> node) {
				_children.emplace_back(node);
			}

			const XMLText& text()const {
				return _text;
			}

			XMLText& text() {
				return _text;
			}

			virtual void accept(XMLVisitor & visitor);

			template<typename T>
			T get()const {
				T tmp;
				Miyuki::IO::FromXML(*this, tmp);
				return std::move(tmp);
			}
		};
		
		

		std::ostream& operator<<(std::ostream & os, const XMLNode & node);

		std::shared_ptr<XMLNode> ParseXMLString(const std::string &);

		std::shared_ptr<XMLNode> ParseXML(const std::string & filename);
	}
}