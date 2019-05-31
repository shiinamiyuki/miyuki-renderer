#include <io/importobj.h>


#include <boost/algorithm/string.hpp>

namespace Miyuki {
	namespace IO {
		namespace __Internal {
			Vec3f ParseFloat3(const std::vector<std::string>& tokens) {
				return Vec3f(std::stof(tokens.at(1)),
					std::stof(tokens.at(2)),
					std::stof(tokens.at(3)));
			}
			std::string ParseFilename(const std::vector<std::string>& tokens) {
				std::string s;
				for (auto iter = tokens.begin() + 1; iter != tokens.end(); iter++) {
					s.append(*iter);
				}
				boost::algorithm::replace_all(s, "\\", "/");
				return s;
			}
		}
		static const char* MYK_IMPORTED_OBJ = "# MYK_IMPORTED_OBJ";
		void LoadMTLFile(const std::string& filename, ObjLoadInfo& info) {
			fmt::print("Loading {}\n", filename);
			auto parentPath = cxx::filesystem::path(filename).parent_path();
			std::ifstream in(filename);
			info.mtlDescription = json{};
			std::vector<std::string> lines;
			{
				std::string line;

				while (std::getline(in, line)) {
					lines.push_back(line);
				}
			}
			int i = 0;
			while (i < lines.size()) {
				std::vector<std::string> tokens, _temp;
				boost::algorithm::split(_temp, lines[i], boost::is_any_of(" "));
				std::copy_if(_temp.begin(), _temp.end(), std::back_inserter(tokens),
					[](const std::string & s) {return !s.empty(); });
				if (tokens.empty()) { i++; continue; }
				if (tokens[0] == "newmtl") {
					auto matName = tokens[1];
					info.mtlDescription[matName] = json{};
					info.mtlDescription[matName]["type"] = "OBJMaterial";
					info.mtlDescription[matName]["Tr"] = false;
					info.mtlDescription[matName]["roughness"] = {
						{"type", "Float"},
						{"value", 0.0}
					};
					info.mtlDescription[matName]["ka"] = {
						{"type","RGB"},
						{"value", {0,0,0}}
					};
					info.mtlDescription[matName]["ks"] = {
						{"type","RGB"},
						{"value", {0,0,0}}
					};
					info.mtlDescription[matName]["IOR"] = {
						{"type","Float"},
						{"value", 1.0f }
					};
					i++;
					while (i < lines.size()) {
						if (lines.empty()) { i++; continue; }
						if (tokens[0] == "Ni") {
							info.mtlDescription[matName]["IOR"]["value"] = std::stof(tokens[1]);
						}
						else if (tokens[0] == "Ns") {
							auto Ns = std::stof(tokens[1]);
							info.mtlDescription[matName]["IOR"]["roughness"] = std::sqrt(2 / (2 + Ns));
						}
						else if (tokens[0] == "Ks") {
							if (info.mtlDescription[matName]["ks"]["type"] == "RGB") {
								Vec3f v = __Internal::ParseFloat3(tokens);
								info.mtlDescription[matName]["ks"]["value"] = { v[0],v[1],v[2] };
							}
						}
						else if (tokens[0] == "Kd") {
							if (info.mtlDescription[matName]["kd"]["type"] == "RGB") {
								Vec3f v = __Internal::ParseFloat3(tokens);
								info.mtlDescription[matName]["kd"]["value"] = { v[0],v[1],v[2] };
							}
						}
						else if (tokens[0] == "Ke") {
							if (info.mtlDescription[matName]["ka"]["type"] == "RGB") {
								Vec3f v = __Internal::ParseFloat3(tokens);
								info.mtlDescription[matName]["ka"]["value"] = { v[0],v[1],v[2] };
							}
						}
						else if (tokens[0] == "map_Ks") {
							info.mtlDescription[matName]["ks"]["type"] = "ImageTexture";
							auto s = __Internal::ParseFilename(tokens);
							info.mtlDescription[matName]["ks"]["value"] = s;

						}
						else if (tokens[0] == "map_Kd") {
							info.mtlDescription[matName]["kd"]["type"] = "ImageTexture";
							auto s = __Internal::ParseFilename(tokens);
							info.mtlDescription[matName]["kd"]["value"] = s;
						}
						else if (tokens[0] == "map_Ke") {
							info.mtlDescription[matName]["ka"]["type"] = "ImageTexture";
							auto s = __Internal::ParseFilename(tokens);
							info.mtlDescription[matName]["ka"]["value"] = s;
						}
						else if (tokens[0] == "newmtl")
							break;
						i++;
					}
				}
				else {
					i++;
				}
			}
		}
		void LoadObjFile(const std::string & filename, ObjLoadInfo & info) {
			auto parentPath = cxx::filesystem::path(filename).parent_path();
			std::ifstream in(filename);
			std::string line;
			std::vector<std::string> tokens, _temp;
			std::getline(in, line);
			if (boost::algorithm::starts_with(line, MYK_IMPORTED_OBJ)) {
				throw std::runtime_error("Cannot import an imported obj file");
			}
			std::string shapeBaseName;
			std::ostringstream out(info.outputContent);
			out << MYK_IMPORTED_OBJ << std::endl;
			int part = 0;
			while (std::getline(in, line)) {
				_temp.clear();
				tokens.clear();
				boost::algorithm::split(_temp, line, boost::is_any_of(" "));
				std::copy_if(_temp.begin(), _temp.end(), std::back_inserter(tokens),
					[](const std::string & s) {return !s.empty(); });
				if (tokens.empty())
					continue;
				if (tokens[0] == "g")continue;
				if (tokens[0] == "mtllib") {
					auto s = __Internal::ParseFilename(tokens);
					auto path = parentPath.append(s);
					LoadMTLFile(path.string(), info);
				}
				else if (tokens[0] == "o") {
					shapeBaseName = tokens[1];
					part = 0;
				}
				else if (tokens[0] == "usemtl") {
					std::string matName = tokens[1];
					boost::algorithm::replace_all(matName, "\\", "/");
					std::string shapeName = fmt::format("{}.{}.{}", shapeBaseName, matName, part++);
					out << fmt::format("o {}", shapeName) << std::endl;
					if (info.shapeMat.find(shapeName) == info.shapeMat.end()) {
						info.shapeMat[shapeName] = matName;
					}
				}
				else {
					out << line << std::endl;
				}
			}

		}
	}
} 