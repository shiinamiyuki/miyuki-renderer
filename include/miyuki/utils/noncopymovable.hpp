#pragma once
namespace Miyuki {
	class NonCopyMovable
	{
	public:
		NonCopyMovable(const NonCopyMovable&&) = delete;
		NonCopyMovable(const NonCopyMovable&) = delete;
		NonCopyMovable& operator = (const NonCopyMovable&) = delete;
	protected:
		NonCopyMovable() = default;
		~NonCopyMovable() = default;
	};
}