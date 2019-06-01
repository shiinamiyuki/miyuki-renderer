#ifndef MIYUKI_NOTIFYABLE_H
#define MIYUKI_NOTIFYABLE_H

namespace Miyuki {
	class INotifyable{
	public:
		virtual void notify() = 0;
	};
}

#endif