#ifndef MIYUKI_ABORTABLE_H
#define MIYUKI_ABORTABLE_H

#include <reflection.h>

namespace Miyuki {
	class Abortable {
	public:
		MYK_INTERFACE(Abortable);
		virtual void abort() = 0;
		virtual bool isAborted()const = 0;
	};		
	MYK_REFL(Abortable, MYK_NIL_BASE, MYK_REFL_NIL);		
}

#endif