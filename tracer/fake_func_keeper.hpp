#pragma once
#include <type_traits>
#include "function_type.hpp"

#define GEN_FAKE_FUNC(cc)															\
	template<typename R, typename... Args> struct Fake<R(cc *)(Args...),			\
		typename std::enable_if<!std::is_void<R>::value>::type> {					\
		static R cc Func(Args... args) {											\
			bool call_ori = true;													\
			SignalKeeper<T::BeforeSignal>::signal(call_ori, args...);				\
			if (call_ori) {															\
				R ret(RealFuncKeeper<T>::real(args...));							\
				SignalKeeper<T::AfterSignal>::signal(call_ori, ret, args...);		\
				return ret;															\
			}																		\
			R ret;																	\
			SignalKeeper<T::AfterSignal>::signal(call_ori, ret, args...);			\
			return ret;																\
		}																			\
	};

#define GEN_FAKE_VOID_FUNC(cc)														\
	template<typename R, typename... Args> struct Fake<R(cc *)(Args...),			\
		typename std::enable_if<std::is_void<R>::value>::type> {					\
		static void cc Func(Args... args) {											\
			bool call_ori = true;													\
			SignalKeeper<T::BeforeSignal>::signal(call_ori, args...);				\
			if (call_ori)															\
				RealFuncKeeper<T>::real(args...);									\
			SignalKeeper<T::AfterSignal>::signal(call_ori, args...);				\
		}																			\
	};

#define GEN_FAKE_MEMBER(cc)															\
	template<typename R, typename C, typename... Args>								\
	struct Fake<R(cc C::*)(Args...),												\
		typename std::enable_if<!std::is_void<R>::value>::type> : public C {		\
		R cc Func(Args... args) {													\
			bool call_ori = true;													\
			C *self = this;															\
			SignalKeeper<T::BeforeSignal>::signal(call_ori, self, args...);			\
			if (call_ori) {															\
				R ret((self->*RealFuncKeeper<T>::real)(args...));					\
				SignalKeeper<T::AfterSignal>::signal(call_ori, ret, self, args...);	\
				return ret;															\
			}																		\
			R ret;																	\
			SignalKeeper<T::AfterSignal>::signal(call_ori, ret, self, args...);		\
			return ret;																\
		}																			\
	};

#define GEN_FAKE_VOID_MEMBER(cc)													\
	template<typename R, typename C, typename... Args>								\
	struct Fake<R(cc C::*)(Args...),												\
		typename std::enable_if<std::is_void<R>::value>::type> : public C {			\
		void cc Func(Args... args) {												\
			bool call_ori = true;													\
			C *self = this;															\
			SignalKeeper<T::BeforeSignal>::signal(call_ori, self, args...);			\
			if (call_ori)															\
				(self->*RealFuncKeeper<T>::real)(args...);							\
			SignalKeeper<T::AfterSignal>::signal(call_ori, self, args...);			\
		}																			\
	};

#define GEN_FAKE_CC(cc)																\
	GEN_FAKE_FUNC(cc)																\
	GEN_FAKE_VOID_FUNC(cc)															\
	GEN_FAKE_MEMBER(cc)																\
	GEN_FAKE_VOID_MEMBER(cc)

#define GEN_FAKE																	\
	GEN_FAKE_CC()																	\
	GEN_FAKE_CC( __stdcall)

namespace tracer {

template<typename T>
struct FakeFuncKeeper {
	typedef typename FunctionPointer<typename T::Signature>::type Pointer;
	static Pointer fake;

	template<typename T, typename Enable = void> struct Fake;
	GEN_FAKE
};

template<typename T>
typename FakeFuncKeeper<T>::Pointer FakeFuncKeeper<T>::fake = 
	reinterpret_cast<FakeFuncKeeper<T>::Pointer>(
	&FakeFuncKeeper<T>::Fake<FakeFuncKeeper<T>::Pointer>::Func);

}	// namespace tracer

#undef GEN_FAKE
#undef GEN_FAKE_CC
#undef GEN_FAKE_FUNC
#undef GEN_FAKE_VOID_FUNC
#undef GEN_FAKE_MEMBER
#undef GEN_FAKE_VOID_MEMBER