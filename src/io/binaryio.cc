#include <io/binaryio.h>

namespace Miyuki {
	namespace IO {

		void BinaryWriter::writeToFile(const std::string& filename) {
			std::ofstream out(filename, std::ios::binary);
			out.write(&data[0], data.size());
		}
		void BinaryWriter::write(const std::string& s) {
			write(s.length());
			for (auto i : s) {
				write(i);
			}
		}
	}
}