#include <ui/editable.h>
#include <ui/input.h>
#include <imgui/imgui.h>

namespace Miyuki {
	namespace GUI {
		void EditEntryUIHandler(EditEntry<bool>& entry) {
			auto temp = *entry.data;
			auto opt = GetBool(entry.prompt, temp);
			if (opt.has_value()) {
				entry.change();
				*entry.data = opt.value();
			}
		}

		void EditEntryUIHandler(EditEntry<int>& entry) {
			auto temp = *entry.data;
			auto opt = GetFloat(entry.prompt, temp);
			if (opt.has_value()) {
				entry.change();
				*entry.data = opt.value();
			}
		}

		void EditEntryUIHandler(EditEntry<Float>& entry) {
			auto temp = *entry.data;
			auto opt = GetFloat(entry.prompt, temp);
			if (opt.has_value()) {
				entry.change();
				*entry.data = opt.value();
			}
		}

		void EditEntryUIHandler(EditEntry<Vec3f>& entry) {
			auto temp = *entry.data;
			auto opt = GetVec3f(entry.prompt, temp);
			if (opt.has_value()) {
				entry.change();
				*entry.data = opt.value();
			}
		}

		void EditEntryUIHandler(EditEntry<Spectrum>& entry) {
			auto temp = *entry.data;
			auto opt = GetSpectrum(entry.prompt, temp);
			if (opt.has_value()) {
				entry.change();
				*entry.data = opt.value();
			}
		}

		void EditEntryUIHandler(EditEntry<std::string>& entry) {
			auto temp = *entry.data;
			auto opt = GetString(entry.prompt, temp);
			if (opt.has_value()) {
				entry.change();
				*entry.data = opt.value();
			}
		}
		bool ISelectable::select(const std::string& opt) {
			if (_allOptions->empty()) {
				option = opt;
				return true;
			}
			if (_allOptions->find(opt) != _allOptions->end()) {
				option = opt;
				return true;
			}
			return false;
		}
	}
}