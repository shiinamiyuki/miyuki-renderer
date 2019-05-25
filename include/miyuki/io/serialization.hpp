#pragma once
#include "xmlparser.h"
#include <math/vec.hpp>
#include <math/spectrum.h>
#include <math/transform.h>
#include <sstream>

namespace Miyuki {
	namespace IO {
#if 0
		inline XMLNode ToXML(Float x) {
			XMLNode node("float");
			node.addAttribute("value", std::to_string(x));
			return std::move(node);
		}

		inline XMLNode ToXML(int x) {
			XMLNode node("int");
			node.addAttribute("value", std::to_string(x));
			return std::move(node);
		}

		inline XMLNode ToXML(const Vec3f& x) {
			XMLNode node("vec3f");
			node.addAttribute("value", fmt::format("{} {} {}", x[0], x[1], x[2]));
			return std::move(node);
		}

		inline XMLNode ToXML(const Transform& x) {
			XMLNode node("transform");
			node.addChild()
			return std::move(node);
		}

		inline void FromXML(const XMLNode& node, Float& s) {
			Assert(node.name() == "float");
			std::istringstream in(node.attr("value").value().get<std::string>());
			in >> s;
		}

		inline void FromXML(const XMLNode & node, Vec3f & s) {
			Assert(node.name() == "vec3f");
			std::istringstream in(node.attr("value").value().get<std::string>());
			in >> s[0] >> s[1] >> s[2];
		}

		inline void FromXML(const XMLNode & node, Spectrum & s) {
			Assert(node.name() == "rgb");
			std::istringstream in(node.attr("value").value().get<std::string>());
			in >> s[0] >> s[1] >> s[2];
		}

		inline void FromXML(const XMLNode & node, Transform & transform) {
			Assert(node.name() == "transform");
			auto rot = node.findChild([](const XMLNode & x) {
				return x.attr("name").value().value == "rotation"; });
			auto trans = node.findChild([](const XMLNode & x) {
				return x.attr("name").value().value == "translation"; });
			auto scale = node.findChild([](const XMLNode & x) {
				return x.attr("name").value().value == "scale"; });
			transform = Transform(trans->get<Vec3f>(), rot->get<Vec3f>(), scale->get<Float>());
		}
#endif
	}
}