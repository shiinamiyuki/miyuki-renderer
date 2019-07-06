#include <core/materials/material.h>

namespace Miyuki {
	namespace Core {
		std::string getMaterialName(Core::Material* material) {
			std::string name = "unknown";
			Reflection::match(material)
				.with<Core::MixedMaterial>([&](Core::MixedMaterial* mat) {
				name = mat->name;
			}).with<Core::DiffuseMaterial>([&](Core::DiffuseMaterial* mat) {
				name = mat->name;
			}).with<Core::GlossyMaterial>([&](Core::GlossyMaterial* mat) {
				name = mat->name;
			});
			return name;
		}
	}
}