#pragma once
#include <miyuki.h>
#include <math/transform.h>
#include <math/clampedfloat.hpp>

namespace Miyuki {
	namespace GUI {
		struct EditEntryBase {
			virtual void uiHandler() = 0;
		};

		template<typename T>
		struct EditEntry : EditEntryBase {
			std::string prompt;
			T* data;
			bool changed = false;
			EditEntry(const std::string& prompt, T* data) :prompt(prompt), data(data) {}
			void uiHandler()override {
				EditEntryUIHandler(*this);
			}

			void change() {
				changed = true;
			}
		};

		// Interface
		class IEditable {
		public:
			virtual std::vector<std::shared_ptr<EditEntryBase>> getEditEntries() = 0;
		};

		// Funny
		class NotEditable : public IEditable {
		public:
			std::vector<std::shared_ptr<EditEntryBase>> getEditEntries()override final { return {}; }
		};

		void EditEntryUIHandler(EditEntry<bool>& entry);
		void EditEntryUIHandler(EditEntry<int>& entry);
		void EditEntryUIHandler(EditEntry<Float>& entry);
		void EditEntryUIHandler(EditEntry<Spectrum>& entry);
		void EditEntryUIHandler(EditEntry<Vec3f>& entry);
		void EditEntryUIHandler(EditEntry<Transform>& entry);
		void EditEntryUIHandler(EditEntry<std::string>& entry);

	}
}