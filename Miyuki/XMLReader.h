#pragma once
#include "Miyuki.h"
namespace Miyuki {
	class _XMLNode;
	using XMLNode = std::shared_ptr<_XMLNode>;
	class _XMLNode {
		std::string tag,text;
		std::map<std::string, std::string> attributes;
		std::vector<XMLNode> children;
	public:
		const std::string & getTag()const { return tag; }
		_XMLNode() {}
		friend class XMLReader;
		std::string getAttribute(const std::string&s) {
			return attributes[s]; 
		
		}
		void addNode(XMLNode node) {
			children.emplace_back(node);
		}
		void setText(const std::string&t) { text = t; }
		std::vector<XMLNode> findAllTags(const std::string &tag) {
			std::vector<XMLNode> v;
			for (auto i : children) {
				if (i->tag == tag) {
					v.emplace_back(i);
				}
			}
			return v;
		}
		Float getFloatAttribute(const std::string &s) {
			auto a = getAttribute(s);
			Float x = -inf;
			x = std::stod(a);
			return x;
		}
		template<typename T>
		void addAttribute(const std::string&k, const T& v) {
			attributes[k] = fmt::format("{}", v);
		}
		std::string toString()const {
			std::string s = fmt::format("<{}", tag);
			for (auto i : attributes) {
				s.append(fmt::format(" {}=\"{}\"", i.first, i.second));
			}
			s.append(">\n");
			s.append(text);
			for (auto i : children) {
				s.append(i->toString());
			}
			s.append(fmt::format("</{}>\n", tag));
			return s;
		}
		friend inline XMLNode makeNode(const std::string & tag);
	};
	inline XMLNode makeNode(const std::string & tag) {
		XMLNode node(new _XMLNode());
		node->tag = tag;
		return node;
	}
	class XMLReader {
		std::string source;
		int p;
		int get(int x)const { return x >= 0 && x < source.size() ? source[x] : 0; }
		int cur()const { return get(p); }
		int peek()const { return get(p + 1); }
		void next() { p++; }
		bool tryChar(int x) {
			if (cur() == x) {
				next();
				return true;
			}
			return false;
		}
		void skipSpace() { while (isspace(cur()))next(); }
		void panic() { exit(-1); }
		std::string readQuote() {
			std::string s;
			if (tryChar('\"')) {
				while (cur() && cur() != '\"') {
					s += cur();
					next();
				}
				if (cur() != '\"') {
					fmt::print("'\\\"' expected but found '{}'\n", cur());
					panic();
				}
				next();
			}
			else {
				fmt::print("'\\\"' expected but found '{}'\n", cur());
				panic();
			}
			return s;
		}
		std::string readTag(bool close,std::map<std::string,std::string> & attr) {
			std::string s;
			if (tryChar('<')) {
				if (close) {
					if (!tryChar('/')) {
						fmt::print("'/' expected but found '{}'\n", cur());
						panic();
					}
				}
				while (cur() && cur() != '>' && ! isspace(cur())) {
					s += cur();
					next();
				}
				if (cur() == '>') {
					next();
				}
				else {
					if (isspace(cur()) && !close) {						
						while (cur() && cur() != '>') {
							skipSpace();
							std::string key, value;
							while (cur() &&!isspace(cur()) && cur()!='=') {
								key += cur();
								next();
							}
							skipSpace();
							if (!tryChar('=')) {
								fmt::print("'=' expected but found '{}'\n", cur());
								panic();
							}
							value = readQuote();
							attr.insert(std::make_pair(key, value));
							skipSpace();
						}
					}
					if (!tryChar('>')) {
						fmt::print("'>' expected but found '{}'\n", cur());
						panic();
					}
				}
			}
			return s;
		}
		std::string readText() {
			std::string s;
			while (cur() && cur() != '<' && cur() != '>') {
				s += cur();
				next();
			}
			return s;
		}
		XMLNode readNode() {
			skipSpace();
			std::string text,close;
			XMLNode node(new _XMLNode());
			auto open = readTag(false,node->attributes);
			while (cur()) {
				skipSpace();
				if (!cur())break;
				if (cur() == '<') {
					if (peek() == '/') {
						close = readTag(true, node->attributes);
						break;
					}
					else {
						node->children.emplace_back(readNode());
					}
				}
				else {
					text += readText();
				}
			}
			if (open != close) {
				fmt::print("unmatched tags {} and {}", open, close);
			}			
			node->tag = open;
			node->text = text;
			return node;
		}
	public:
		XMLReader(const std::string & s) { p = 0; source = s; }
		XMLNode read() { return readNode(); }
	};
}