#pragma once
#include <miyuki.h>
#include <math/transform.h>
#include <math/clampedfloat.hpp>
#include <utils/cloneable.hpp>

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

		class ISelectable : public IEditable{
			bool _selectable;
			std::string option;
			std::shared_ptr<std::set<std::string>> _allOptions;
		public:
			ISelectable():_allOptions(new std::set<std::string>()){}
			void addOptions(const std::string& s) { _allOptions->insert(s); }
			void enable() { _selectable = true; }
			void disable() { _selectable = false; }
			bool selectable()const { return _selectable; }
			virtual bool select(const std::string&);
		};

		struct EditTreeNode : IEditable {
			std::vector<std::shared_ptr<EditEntryBase>> entries;
		public:
			void add(const std::shared_ptr<EditEntryBase>& e) { entries.emplace_back(e); }
			std::vector<std::shared_ptr<EditEntryBase>> getEditEntries()override { return entries; }
		};
	}
}