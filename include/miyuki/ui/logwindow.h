#ifndef MYK_LOG_WINDOW_H
#define MYK_LOG_WINDOW_H
#include <utils/log.h>
#include <imgui/imgui.h>

namespace Miyuki {
	namespace GUI {
		struct LogWindowContent {

			std::string content;
			void append(const std::string& str) {
				content.append(str);
			}

			void draw() {
				ImGui::TextUnformatted(&content[0]);
			}

			static LogWindowContent* GetInstance() {
				static std::mutex mutex;
				if (!instance) {
					std::lock_guard<std::mutex> lock(mutex);
					if (!instance) {
						instance = new LogWindowContent();
					}
				}
				return instance;
			}

			static LogWindowContent* instance;

		private:
			LogWindowContent() {}
		};
	}
}
#endif