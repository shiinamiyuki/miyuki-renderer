#ifndef MIYUKI_TRAIT_HPP
#define MIYUKI_TRAIT_HPP
// This file is auto-generated
// Do not edit!!
#include <memory.h>
namespace Miyuki {
	template<class...>
	struct Trait;
	template<class T1>
	struct Trait<T1> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
		};
	private:
		T1* ptr1 = nullptr;
	};
	template<class T1, class T2>
	struct Trait<T1, T2> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
	};
	template<class T1, class T2, class T3>
	struct Trait<T1, T2, T3> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
	};
	template<class T1, class T2, class T3, class T4>
	struct Trait<T1, T2, T3, T4> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5>
	struct Trait<T1, T2, T3, T4, T5> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6>
	struct Trait<T1, T2, T3, T4, T5, T6> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
	struct Trait<T1, T2, T3, T4, T5, T6, T7> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}
		template<>
		T10* get<T10>()const {
			return ptr10;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
			ptr10 = static_cast<T10*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
			ptr10 = trait.get<T10>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
		T10* ptr10 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}
		template<>
		T10* get<T10>()const {
			return ptr10;
		}
		template<>
		T11* get<T11>()const {
			return ptr11;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
			ptr10 = static_cast<T10*>(p);
			ptr11 = static_cast<T11*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
			ptr10 = trait.get<T10>();
			ptr11 = trait.get<T11>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
		T10* ptr10 = nullptr;
		T11* ptr11 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}
		template<>
		T10* get<T10>()const {
			return ptr10;
		}
		template<>
		T11* get<T11>()const {
			return ptr11;
		}
		template<>
		T12* get<T12>()const {
			return ptr12;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
			ptr10 = static_cast<T10*>(p);
			ptr11 = static_cast<T11*>(p);
			ptr12 = static_cast<T12*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
			ptr10 = trait.get<T10>();
			ptr11 = trait.get<T11>();
			ptr12 = trait.get<T12>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
		T10* ptr10 = nullptr;
		T11* ptr11 = nullptr;
		T12* ptr12 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}
		template<>
		T10* get<T10>()const {
			return ptr10;
		}
		template<>
		T11* get<T11>()const {
			return ptr11;
		}
		template<>
		T12* get<T12>()const {
			return ptr12;
		}
		template<>
		T13* get<T13>()const {
			return ptr13;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
			ptr10 = static_cast<T10*>(p);
			ptr11 = static_cast<T11*>(p);
			ptr12 = static_cast<T12*>(p);
			ptr13 = static_cast<T13*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
			ptr10 = trait.get<T10>();
			ptr11 = trait.get<T11>();
			ptr12 = trait.get<T12>();
			ptr13 = trait.get<T13>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
		T10* ptr10 = nullptr;
		T11* ptr11 = nullptr;
		T12* ptr12 = nullptr;
		T13* ptr13 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}
		template<>
		T10* get<T10>()const {
			return ptr10;
		}
		template<>
		T11* get<T11>()const {
			return ptr11;
		}
		template<>
		T12* get<T12>()const {
			return ptr12;
		}
		template<>
		T13* get<T13>()const {
			return ptr13;
		}
		template<>
		T14* get<T14>()const {
			return ptr14;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
			ptr10 = static_cast<T10*>(p);
			ptr11 = static_cast<T11*>(p);
			ptr12 = static_cast<T12*>(p);
			ptr13 = static_cast<T13*>(p);
			ptr14 = static_cast<T14*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
			ptr10 = trait.get<T10>();
			ptr11 = trait.get<T11>();
			ptr12 = trait.get<T12>();
			ptr13 = trait.get<T13>();
			ptr14 = trait.get<T14>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
		T10* ptr10 = nullptr;
		T11* ptr11 = nullptr;
		T12* ptr12 = nullptr;
		T13* ptr13 = nullptr;
		T14* ptr14 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}
		template<>
		T10* get<T10>()const {
			return ptr10;
		}
		template<>
		T11* get<T11>()const {
			return ptr11;
		}
		template<>
		T12* get<T12>()const {
			return ptr12;
		}
		template<>
		T13* get<T13>()const {
			return ptr13;
		}
		template<>
		T14* get<T14>()const {
			return ptr14;
		}
		template<>
		T15* get<T15>()const {
			return ptr15;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
			ptr10 = static_cast<T10*>(p);
			ptr11 = static_cast<T11*>(p);
			ptr12 = static_cast<T12*>(p);
			ptr13 = static_cast<T13*>(p);
			ptr14 = static_cast<T14*>(p);
			ptr15 = static_cast<T15*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
			ptr10 = trait.get<T10>();
			ptr11 = trait.get<T11>();
			ptr12 = trait.get<T12>();
			ptr13 = trait.get<T13>();
			ptr14 = trait.get<T14>();
			ptr15 = trait.get<T15>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
		T10* ptr10 = nullptr;
		T11* ptr11 = nullptr;
		T12* ptr12 = nullptr;
		T13* ptr13 = nullptr;
		T14* ptr14 = nullptr;
		T15* ptr15 = nullptr;
	};
	template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16>
	struct Trait<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16> {
		template<class T>
		T* get()const {
			static_assert(false, "Does not have that trait");
		}
		template<>
		T1* get<T1>()const {
			return ptr1;
		}
		template<>
		T2* get<T2>()const {
			return ptr2;
		}
		template<>
		T3* get<T3>()const {
			return ptr3;
		}
		template<>
		T4* get<T4>()const {
			return ptr4;
		}
		template<>
		T5* get<T5>()const {
			return ptr5;
		}
		template<>
		T6* get<T6>()const {
			return ptr6;
		}
		template<>
		T7* get<T7>()const {
			return ptr7;
		}
		template<>
		T8* get<T8>()const {
			return ptr8;
		}
		template<>
		T9* get<T9>()const {
			return ptr9;
		}
		template<>
		T10* get<T10>()const {
			return ptr10;
		}
		template<>
		T11* get<T11>()const {
			return ptr11;
		}
		template<>
		T12* get<T12>()const {
			return ptr12;
		}
		template<>
		T13* get<T13>()const {
			return ptr13;
		}
		template<>
		T14* get<T14>()const {
			return ptr14;
		}
		template<>
		T15* get<T15>()const {
			return ptr15;
		}
		template<>
		T16* get<T16>()const {
			return ptr16;
		}

		template<class Any>
		Trait(Any p) {
			reset(p);
		}
		Trait() {}
		template<class Any>
		Trait& operator = (Any p) {
			reset(p);
			return *this;
		}
		template<class Any>
		void reset(Any* p) {
			ptr1 = static_cast<T1*>(p);
			ptr2 = static_cast<T2*>(p);
			ptr3 = static_cast<T3*>(p);
			ptr4 = static_cast<T4*>(p);
			ptr5 = static_cast<T5*>(p);
			ptr6 = static_cast<T6*>(p);
			ptr7 = static_cast<T7*>(p);
			ptr8 = static_cast<T8*>(p);
			ptr9 = static_cast<T9*>(p);
			ptr10 = static_cast<T10*>(p);
			ptr11 = static_cast<T11*>(p);
			ptr12 = static_cast<T12*>(p);
			ptr13 = static_cast<T13*>(p);
			ptr14 = static_cast<T14*>(p);
			ptr15 = static_cast<T15*>(p);
			ptr16 = static_cast<T16*>(p);
		}
		template<class... T>
		void reset(const Trait<T...>& trait) {
			ptr1 = trait.get<T1>();
			ptr2 = trait.get<T2>();
			ptr3 = trait.get<T3>();
			ptr4 = trait.get<T4>();
			ptr5 = trait.get<T5>();
			ptr6 = trait.get<T6>();
			ptr7 = trait.get<T7>();
			ptr8 = trait.get<T8>();
			ptr9 = trait.get<T9>();
			ptr10 = trait.get<T10>();
			ptr11 = trait.get<T11>();
			ptr12 = trait.get<T12>();
			ptr13 = trait.get<T13>();
			ptr14 = trait.get<T14>();
			ptr15 = trait.get<T15>();
			ptr16 = trait.get<T16>();
		};
	private:
		T1* ptr1 = nullptr;
		T2* ptr2 = nullptr;
		T3* ptr3 = nullptr;
		T4* ptr4 = nullptr;
		T5* ptr5 = nullptr;
		T6* ptr6 = nullptr;
		T7* ptr7 = nullptr;
		T8* ptr8 = nullptr;
		T9* ptr9 = nullptr;
		T10* ptr10 = nullptr;
		T11* ptr11 = nullptr;
		T12* ptr12 = nullptr;
		T13* ptr13 = nullptr;
		T14* ptr14 = nullptr;
		T15* ptr15 = nullptr;
		T16* ptr16 = nullptr;
	};


	using unique_void_ptr = std::unique_ptr<void, void(*)(void*)>;

	template<class T>
	unique_void_ptr make_unique_void(T* ptr) {
		return unique_void_ptr(ptr, [](void* p) {
			auto true_p = static_cast<T*>(p);
			delete true_p;
		});
	};
	unique_void_ptr make_unique_void_nullptr() {
		return unique_void_ptr(nullptr, [](void* p) {

		});
	}

	template<class... T>
	struct BoxedTrait;
	template<class... T>
	struct BoxedTrait {
		template<class... U>
		friend struct BoxedTrait;
		BoxedTrait() :ptr(make_unique_void_nullptr()) {}
		template<class Any>
		BoxedTrait(Any* p) : ptr(std::move(make_unique_void(p))) {
			trait.reset(p);
		}
		template<class... U>
		BoxedTrait(BoxedTrait<U...>&& box) : ptr(std::move(box.ptr)) {
			trait = box.trait;
		}
		template<class Any>
		void reset(Any* p) {
			ptr = make_unique_void(p);
			trait.reset(p);
		}
		const Trait<T...>* operator ->()const {
			return &trait;
		}
		const Trait<T...>& operator*()const {
			return trait;
		}
	private:
		Trait<T...> trait;
		unique_void_ptr ptr;
	};


}
#endif