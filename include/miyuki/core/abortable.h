#ifndef MIYUKI_ABORTABLE_H
#define MIYUKI_ABORTABLE_H

#include <reflection.h>

namespace Miyuki {
	namespace Core {
		class Abortable {
		public:
			MYK_INTERFACE(Abortable);
			virtual void abort() = 0;
		};		
	}		
}

#endif