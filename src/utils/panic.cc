#include <utils/panic.h>

namespace Miyuki {
	static std::mutex m;
	void panic(const char* msg)  {
		std::lock_guard<std::mutex> lock(m);
		FILE* file = fopen("error.txt", "w");
		fprintf(file, "%s", msg);
		fclose(file);
		std::abort();
	}
}