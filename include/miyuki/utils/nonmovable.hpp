#pragma once
namespace Miyuki {
	class NonMovable
	{
	public:
		NonMovable& operator = (const NonMovable&&) = delete;

	protected:
		NonMovable() = default;
		~NonMovable() = default;
	};
}