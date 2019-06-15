#include <io/importobj.h>
#include <graph/materialnode.h>
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
			std::vector<Reflection::LocalObject<Graph::MaterialNode>> materials;
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
					[](const std::string& s) {return !s.empty(); });
				if (tokens.empty()) { i++; continue; }
				if (tokens[0] == "newmtl") {
					bool hasMapKd = false;
					bool hasMapKs = false;
					bool hasMapKa = false;
					auto matName = tokens[1];
					auto material = info.runtime->New<Graph::MixedMaterialNode>();
					material->name = info.runtime->New<Reflection::StringNode>(matName);
					auto kd = info.runtime->New<Graph::DiffuseMaterialNode>();
					auto ks = info.runtime->New<Graph::GlossyMaterialNode>();
					auto ka = info.runtime->New<Graph::ShaderNode>();					
					i++;
					while (i < lines.size()) {
						if (lines[i].empty()) { i++; continue; }
						tokens.clear();
						_temp.clear();
						boost::algorithm::split(_temp, lines[i], boost::is_any_of(" "));
						std::copy_if(_temp.begin(), _temp.end(), std::back_inserter(tokens),
							[](const std::string& s) {return !s.empty(); });
						if (tokens[0] == "Ni") {
							//
						}
						else if (tokens[0] == "Ns") {
							auto Ns = std::stof(tokens[1]);
							Float alpha = std::sqrt(2 / (2 + Ns));
							auto roughness = info.runtime->New<Reflection::FloatNode>(alpha);
							auto value = info.runtime->New<Graph::FloatNode>();
							value->value = roughness;
							ks->roughness = value;
						}
						else if (tokens[0] == "Ks") {
							if (!hasMapKs) {
								Vec3f v = __Internal::ParseFloat3(tokens);
								auto color = info.runtime->New<Graph::RGBNode>();
								color->value = info.runtime->New<Reflection::Float3Node>(v);
								ks->color = color;							
							}
						}
						else if (tokens[0] == "Kd") {
							if (!hasMapKd) {
								Vec3f v = __Internal::ParseFloat3(tokens);
								auto color = info.runtime->New<Graph::RGBNode>();
								color->value = info.runtime->New<Reflection::Float3Node>(v);
								kd->color = color;
							}
						}
						else if (tokens[0] == "Ke") {
							if (!hasMapKa) {
								Vec3f v = __Internal::ParseFloat3(tokens);
								auto color = info.runtime->New<Graph::RGBNode>();
								color->value = info.runtime->New<Reflection::Float3Node>(v);
								ka = color;
							}
						}//
						else if (tokens[0] == "map_Ks") {							
							auto s = __Internal::ParseFilename(tokens);
							auto file = info.runtime->New<Reflection::FileNode>(s);
							auto color = info.runtime->New<Graph::ImageTextureNode>();
							color->file = file;
							ks->color = color;
							hasMapKs = true;

						}
						else if (tokens[0] == "map_Kd") {
							auto s = __Internal::ParseFilename(tokens);
							auto file = info.runtime->New<Reflection::FileNode>(s);
							auto color = info.runtime->New<Graph::ImageTextureNode>();
							color->file = file;
							kd->color = color;
							hasMapKd = true;
						}
						else if (tokens[0] == "map_Ke") {
							auto s = __Internal::ParseFilename(tokens);
							auto file = info.runtime->New<Reflection::FileNode>(s);
							auto color = info.runtime->New<Graph::ImageTextureNode>();
							color->file = file;
							ka = color;
							hasMapKa = true;
						}
						else if (tokens[0] == "newmtl")
							break;
						i++;
					}
					material->matA = kd;
					material->matB = ks;
					material->fraction = info.runtime->New<Reflection::FloatNode>(0.5f);
					material->emission = ka;
					
					materials.emplace_back(*info.runtime, material);
				}
				else {
					i++;
				}
			}
			info.mtlDescription = json::array();
			for (auto i : materials) {
				json j;
				i->serialize(j);
				info.mtlDescription.push_back(j);
			}
		}
		void LoadObjFile(const std::string & filename, ObjLoadInfo & info) {
			try {
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
						[](const std::string& s) {return !s.empty(); });
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
			catch (std::runtime_error& e) {
				std::cerr << e.what() << std::endl;
			}
		}
	}
}