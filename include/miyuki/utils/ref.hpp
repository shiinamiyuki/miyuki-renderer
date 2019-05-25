#pragma once
namespace Miyuki {
	template<typename T>
	struct Ref {
	private:
		T * data;
	public:
		Ref() :data(nullptr) {}
		explicit Ref(const T* data) :data(data) {}
		T & operator *() {
			return *data;
		}
		T* operator->() {
			return data;
		}
		const T & operator *() const {
			return *data;
		}
		const T* operator->() const {
			return data;
		}
		operator bool()const {
			return data != nullptr;
		}
		Ref & operator = (T * data) {
			this->data = data;
			return *this;
		}
	};

}