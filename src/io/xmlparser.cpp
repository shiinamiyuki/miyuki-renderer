#include "xmlparser.h"
#include <utils/log.h>
#include <stack>

namespace Miyuki {
	namespace IO {
		void XMLNode::accept(XMLVisitor& visitor) {
			for (auto i : _children) {
				visitor.visit(*i);
			}
		}

		std::ostream& operator<<(std::ostream& os, const XMLNode& node) {
			int indent = 0;
			auto printIndent = [&]() {
				for (int i = 0; i < indent; i++)
					os << "  ";
			};
			std::function<void(const XMLNode&)> f = [&](const XMLNode & node) {
				os << fmt::format("<{}", node.name());

				if (!node.attributes().empty()) {
					os << " ";
					for (auto i : node.attributes()) {
						for (auto c : i.first) {
							switch (c) {
							case '\n':
								os << "\\n";
								break;
							case '\t':
								os << "\\t";
								break;
							case'\'':
								os << "\\'";
								break;
							case '\\':
								os << "\\\\";
								break;
							case '\"':
								os << "\\\"";
								break;
							default:
								os << c;
								break;
							}
						}
						os << "=\"" << i.second.value << "\"";
					}
				}
				if (node.children().empty() && node.text().text.empty()) {
					os << "/>\n";
				}
				else {
					os << ">\n"; indent++;
					for (const auto i : node.children()) {
						printIndent();
						f(*i);
					}
					if (!node.text().text.empty()) {
						os << node.text().text << "\n";
					}
					indent--;
					printIndent();
					os << "</" << node.name() << " >\n";
				}
			};
			f(node);
			return os;
		}

		struct XMLParser {
			ParserLoc location;
			const std::string& source;
			std::string filename;
			char at(size_t idx) {
				return idx < source.length() ? source[idx] : 0;
			}
			size_t pos = 0;

			char next() {
				return at(pos + 1);
			}

			char cur() { return at(pos); }

			std::stack<std::string> tags;

			XMLParser(const std::string & source, const std::string & filename = "")
				:source(source), filename(filename) {
				location.line = 1;
				location.col = 1;
				location.filename = filename;
			}
			void advance() {
				if (cur() == '\n') {
					location.line++;
					location.col = 1;
				}
				else {
					location.col++;
				}
				pos++;
			}

			void expect(char c) {
				if (cur() != c) {
					throw std::runtime_error(
						fmt::format("'{}' expected at {}:{}:{} but have '{}'",
							c,
							location.filename,
							location.line, location.col,
							cur()));
				}
				advance();
			}
			void expect(const std::string & s) {
				for (auto c : s) {
					if (cur() != c) {
						throw std::runtime_error(
							fmt::format("'{}' expected at {}:{}:{}",
								s,
								location.filename,
								location.line, location.col));
					}
					advance();
				}
			}
			std::string parseQuote() {
				expect('"');
				std::string s = "";
				while (cur() && cur() != '"') {
					if (cur() != '\\') {
						s += cur();
						advance();
					}
					else {
						advance();
						switch (cur()) {
						case 't':
							s += '\t';
							break;
						case 'n':
							s += '\n';
							break;
						case '\\':
							s += '\\';
							break;
						case '"':
							s += '\"';
							break;
						case '\'':
							s += '\'';
							break;
						default:
							Log::log("unknown escape sequence '\\{}'", cur());
						}
						advance();
					}
				}
				expect('"');
				return s;
			}

			void skip() {
				while (isspace(cur()))advance();
			}
			void parseProlog() {
				if (cur() == '<' && next() == '?') {
					advance();
					advance();

				}
			}

			std::string identifier() {
				std::string s;
				while ((isalnum(cur()) || cur() == '_')) {
					s += cur();
					advance();
					expect("xml");
				}
				return s;
			}

			enum TagType {
				open,
				close,
				selfClose
			};

			std::pair<std::string, TagType> parseTag(XMLAttributeDict & attr) {
				expect('<');
				if (cur() != '/') {
					skip();
					auto tag = identifier();
					skip();
					if ((cur() == '_' || isalpha(cur()))) {
						while (cur() == '_' || isalpha(cur())) {
							auto key = identifier();
							skip();
							expect('=');
							skip();
							auto value = parseQuote();
							attr.insert(std::make_pair(key, XMLAttribute{ key,value }));
							skip();
						}
					}
					if (cur() == '/') {
						advance();
						expect('>');
						return { tag, selfClose };
					}
					tags.push(tag);
					expect('>');
					return { tag, open };
				}
				else {
					advance();
					auto tag = identifier();

					if (tags.empty()) {
						throw std::runtime_error(
							fmt::format("astray close tag {}", tag, tags.top())
						);
					}
					else if (tag != tags.top()) {
						throw std::runtime_error(
							fmt::format("{} does not match open tag {}", tag, tags.top())
						);
					}
					else {
						tags.pop();
						expect('>');
						return { tag, close };
					}
				}
			}
			std::shared_ptr<XMLNode> parseNode() {
				XMLAttributeDict attr;
				skip();
				auto tag = parseTag(attr);
				auto node = std::shared_ptr<XMLNode>(new XMLNode(tag.first, XMLText(), attr, {}));
				if (tag.second == open) {
					auto& text = node->text().text;
					skip();
					while (cur()) {
						if (cur() == '<') {
							if (next() == '/') {
								auto tag = parseTag(attr);
								node->text().trim();
								return node;
							}
							else {
								auto child = parseNode();
								node->addChild(child);
							}
						}
						else {
							if (cur() == '&') {
								advance();
								throw NotImplemented();
							}
							else {
								text += cur();
								advance();
							}
						}
						skip();
					}
				}
				else {
					Assert(tag.second == selfClose);
					return node;
				}
			}
		};

		std::shared_ptr<XMLNode> ParseXMLString(const std::string & s) {
			XMLParser parser(s);
			return parser.parseNode();
		}
		std::shared_ptr<XMLNode> ParseXML(const std::string& filename) {
			std::ifstream in(filename);
			std::string content((std::istreambuf_iterator<char>(in)),
				(std::istreambuf_iterator<char>()));
			return ParseXMLString(content);
		}
	}
}