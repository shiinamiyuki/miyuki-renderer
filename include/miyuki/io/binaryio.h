#include <miyuki.h>


namespace Miyuki {
	namespace IO {
		class BinaryReader {
			std::vector<char> data;
			int pos = 0;
		public:
			template<class T>
			std::enable_if_t<std::is_fundamental_v<T>, T> read() {
				if (pos + sizeof(T) >= data.size()) {
					throw std::out_of_range("reading T overflows the buffer");
				}
				T result = *reinterpret_cast<T*>(&data[pos]);
				pos += sizeof(T);
				return result;
			}

			std::string readString() {
				auto len = read<size_t>();
				std::string s;
				for (auto i = 0; i < len; i++) {
					s += read<char>();
				}
				return s;
			}
		};
		class BinaryWriter {
			std::vector<char> data;
		public:
			template<class T>
			std::enable_if_t<std::is_fundamental_v<T>, void> write(T v) {
				unsigned char buffer[sizeof(T)];
				*reinterpret_cast<T*>(buffer) = v;
				for (auto i : buffer) {
					data.emplace_back(i);
				}
			}
			void write(const std::string& s);
			void writeToFile(const std::string& filename);
		};
	}
}