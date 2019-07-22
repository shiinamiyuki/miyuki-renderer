#ifndef MIYUKI_ABORTABLE_H
#define MIYUKI_ABORTABLE_H

#include <reflection.h>

namespace Miyuki {
	namespace Core {
		struct Abortable {
			MYK_INTERFACE(Abortable);
			virtual void abort() = 0;
			virtual bool aborted()const = 0;
			virtual void restart() = 0;
		};		
	}		
}

#endif