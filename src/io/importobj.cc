#include <io/importobj.h>
#include <graph/graph.h>

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
					json& mat = info.mtlDescription[matName];
					mat["type"] = "OBJMaterial";
					json Tr = {
						{"key", "Tr"},
						{"value", json{
							{"type", "Bool"},
							{"value",  false}
						}}
					};

					json roughness = {
						{"key", "roughness"},
						{"value", json{
							{"type", "Float"},
							{"value",  0.0f}
						}}
					};

					json ka = {
						{"key", "ka"},
						{"value", json{
							{"type", "Float3"},
							{"value",  {0,0,0}}
						}}
					};

					json ks = {
						{"key", "ks"},
						{"value", json{
							{"type", "Float3"},
							{"value",  {0,0,0}}
						}}
					};

					json kd = {
						{"key", "kd"},
						{"value", json{
							{"type", "Float3"},
							{"value", {0,0,0}}
						}}
					};

					json ior = {
						{"key", "ior"},
						{"value", json{
							{"type", "Float"},
							{"value",  1.0f}
						}}
					};

					i++;
					while (i < lines.size()) {
						if (lines.empty()) { i++; continue; }
						if (tokens[0] == "Ni") {
							ior["value"] = json{
								{"type", "Float"},
								{"value", std::stof(tokens[1])}
							};
						}
						else if (tokens[0] == "Ns") {
							auto Ns = std::stof(tokens[1]);
							roughness["value"] = json{
								{"type", "Float"},
								{"value", std::sqrt(2 / (2 + Ns))}
							};
						}
						else if (tokens[0] == "Ks") {
							if (ks["value"]["type"] == "Float3") {
								Vec3f v = __Internal::ParseFloat3(tokens);
								ks["value"]["value"] = json{ v[0],v[1],v[2] };
							}
						}
						else if (tokens[0] == "Kd") {
							if (kd["value"]["type"] == "Float3") {
								Vec3f v = __Internal::ParseFloat3(tokens);
								kd["value"]["value"] = json{ v[0],v[1],v[2] };
							}
						}
						else if (tokens[0] == "Ke") {
							if (ka["value"]["type"] == "Float3") {
								Vec3f v = __Internal::ParseFloat3(tokens);
								ka["value"]["value"] = json{ v[0],v[1],v[2] };
							}
						}//
						else if (tokens[0] == "map_Ks") {
							ks["value"]["type"] = "ImageTexture";
							auto s = __Internal::ParseFilename(tokens);
							ks["value"]["value"] = s;

						}
						else if (tokens[0] == "map_Kd") {
							kd["value"]["type"] = "ImageTexture";
							auto s = __Internal::ParseFilename(tokens);
							kd["value"]["value"] = s;
						}
						else if (tokens[0] == "map_Ke") {
							ka["value"]["type"] = "ImageTexture";
							auto s = __Internal::ParseFilename(tokens);
							ka["value"]["value"] = s;
						}
						else if (tokens[0] == "newmtl")
							break;
						i++;
					}
					mat["subnodes"] = json{
						ka,
						kd,
						ks,
						Tr,
						roughness,
						ior
					};
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