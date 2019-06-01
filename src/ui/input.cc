#include <ui/input.h>
#include <utils/log.h>
#include <imgui/imgui.h>

namespace Miyuki {
	namespace GUI {
		std::optional<bool> GetBool(const std::string& prompt, bool initial)
		{
			bool temp = initial;
			if (ImGui::Checkbox(prompt.c_str(), &temp)) {
				return std::optional<bool>(temp);
			}
			else {
				return {};
			}
		}
		std::optional<int> GetInt(const std::string& prompt, int initial) {
			int temp = initial;
			if (ImGui::InputInt(prompt.c_str(), &temp, ImGuiInputTextFlags_EnterReturnsTrue)) {
				return std::optional<int>(temp);
			}
			else {
				return {};
			}
		}
		std::optional<Float> GetFloat(const std::string& prompt, Float initial) {
			Float temp = initial;
			if (ImGui::InputFloat(prompt.c_str(), &temp, 0.01, 0.1, "%f", ImGuiInputTextFlags_EnterReturnsTrue)) {
				return std::optional<Float>(temp);
			}
			else {
				return {};
			}
		}

		std::optional<Vec3f> GetVec3f(const std::string& prompt, Vec3f initial) {
			auto temp = initial;
			if (ImGui::InputFloat3(prompt.c_str(), &temp[0], "%f", ImGuiInputTextFlags_EnterReturnsTrue)) {
				return std::optional<Vec3f>(temp);
			}
			return {};
		}

		std::optional<Spectrum> GetSpectrum(const std::string& prompt, Spectrum initial) {
			auto temp = initial;
			if (ImGui::ColorPicker3(prompt.c_str(), &temp[0], ImGuiInputTextFlags_EnterReturnsTrue)) {
				return std::optional<Spectrum>(temp);
			}
			return {};
		}

		std::optional<Float> GetFloatClamped(const std::string& prompt, Float initial, Float minVal, Float maxVal) {
			Float temp = initial;
			if (ImGui::SliderFloat(prompt.c_str(), &temp, minVal, maxVal)) {
				return std::optional<Float>(temp);
			}
			return {};
		}

		std::optional<std::string> GetString(const std::string& prompt, const std::string& initial) {
			std::vector<char> buffer(4096);
			for (auto c : initial) {
				buffer.emplace_back(c);
			}
			if (ImGui::InputText(prompt.c_str(), &buffer[0], buffer.capacity(), ImGuiInputTextFlags_EnterReturnsTrue)) {
				if (buffer.back() != 0) {
					Log::log("buffer.back() != 0\n");
				}
				return std::optional<std::string>(&buffer[0]);
			}
			return {};
		}
	}
}