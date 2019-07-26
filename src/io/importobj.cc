#include <io/importobj.h>
#include <boost/algorithm/string.hpp>
#include <utils/log.h>
#include <core/materials/mixedmaterial.h>
#include <core/materials/diffusematerial.h>
#include <core/materials/glossymaterial.h>
#if 1
namespace Miyuki {
	namespace IO {
		namespace __Internal {
			Vec3f ParseFloat3(const std::vector<std::string>& tokens) {
				
				auto v =  Vec3f(std::stof(tokens.at(1)),
					std::stof(tokens.at(2)),
					std::stof(tokens.at(3)));
				return v;
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
			Log::log("Loading {}\n", filename);
			auto parentPath = cxx::filesystem::path(filename).parent_path();
			std::ifstream in(filename);
			auto& materials = info.materials;
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
					auto material = Reflection::makeBox<Core::MixedMaterial>();
					Box<Core::MaterialSlot> slot = Reflection::makeBox<Core::MaterialSlot>();
					slot->name = matName;
					auto kd = Reflection::makeBox<Core::DiffuseMaterial>();
					auto ks = Reflection::makeBox<Core::GlossyMaterial>();
					Box<Core::Shader> ka = Reflection::makeBox<Core::FloatShader>(0);
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
							ks->roughness = Reflection::makeBox<Core::FloatShader>(alpha);
						}
						else if (tokens[0] == "Ks") {
							if (!hasMapKs) {
								Vec3f v = __Internal::ParseFloat3(tokens);
								ks->color = Reflection::makeBox<Core::RGBShader>(v);
							}
						}
						else if (tokens[0] == "Kd") {
							if (!hasMapKd) {
								Vec3f v = __Internal::ParseFloat3(tokens);
								kd->color = Reflection::makeBox<Core::RGBShader>(v);
							}
						}
						else if (tokens[0] == "Ke") {
							if (!hasMapKa) {
								Vec3f v = __Internal::ParseFloat3(tokens);
								ka = Reflection::makeBox<Core::RGBShader>(v);
							}
						}//
						else if (tokens[0] == "map_Ks") {							
							auto s = cxx::filesystem::absolute(__Internal::ParseFilename(tokens));
							ks->color = Reflection::makeBox<Core::ImageTextureShader>(s);
							hasMapKs = true;

						}
						else if (tokens[0] == "map_Kd") {
							auto s = cxx::filesystem::absolute(__Internal::ParseFilename(tokens));
							kd->color = Reflection::makeBox<Core::ImageTextureShader>(s);
							hasMapKd = true;
						}
						else if (tokens[0] == "map_Ke") {
							auto s = cxx::filesystem::absolute(__Internal::ParseFilename(tokens));
							ka = Reflection::makeBox<Core::ImageTextureShader>(s);
							hasMapKa = true;
						}
						else if (tokens[0] == "newmtl")
							break;
						i++;
					}
					material->matA = std::move(kd);
					material->matB = std::move(ks);
					material->fraction = Reflection::makeBox<Core::FloatShader>(0.5f);
					material->emissionShader = std::move(ka);
					slot->material = std::move(material);
					materials.emplace_back(std::move(slot));
				}
				else {
					i++;
				}
			}
		}
		void LoadObjFile(const std::string & filename, ObjLoadInfo & info) {
			try {
				auto parentPath = cxx::filesystem::path(filename).parent_path();
				std::ifstream in(filename);
				std::string line;
				std::vector<std::string> tokens, _temp;
				std::set<std::string> visitedMTL;
				std::getline(in, line);
				if (boost::algorithm::starts_with(line, MYK_IMPORTED_OBJ)) {
					throw std::runtime_error("Cannot import an imported obj file");
				}
				std::string shapeBaseName;
				std::ostringstream out;
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
						if (visitedMTL.find(s) == visitedMTL.end()) {
							auto path = parentPath.append(s);
							LoadMTLFile(path.string(), info);
							visitedMTL.insert(s);
						}
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
			
				info.meshFile = filename;
				info.outputContent = out.str();
			}
			catch (std::runtime_error& e) {
				std::cerr << e.what() << std::endl;
			}
		}
	}
}
#endif