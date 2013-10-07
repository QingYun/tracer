#pragma once
#include <boost/preprocessor.hpp>
#include <type_traits>
#include "function_type.hpp"

#ifndef TRACER_ARG_LIMIT
#define TRACER_ARG_LIMIT 10
#endif

#define TYPE_AND_PARAM(z, n, _)														\
	BOOST_PP_CAT(P, n) BOOST_PP_CAT(p, n)

#define GEN_FAKE_FUNC(n, cc)														\
	template<typename R BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>	\
	struct Fake<R (cc *)(BOOST_PP_ENUM_PARAMS(n, P))> {								\
		static R cc Func(BOOST_PP_ENUM(n, TYPE_AND_PARAM, _)) {						\
			bool call_ori = true;													\
			R ret;																	\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),		\
				call_ori BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));			\
			if (call_ori)															\
				ret = RealFuncKeeper<T>::real(BOOST_PP_ENUM_PARAMS(n, p));			\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),			\
				call_ori, ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));		\
			return ret;																\
		}																			\
	};																				

#define GEN_FAKE_VOID_FUNC(n, cc)													\
	template<BOOST_PP_ENUM_PARAMS(n, typename P)>									\
	struct Fake<void (cc *)(BOOST_PP_ENUM_PARAMS(n, P))> {							\
		static void cc Func(BOOST_PP_ENUM(n, TYPE_AND_PARAM, _)) {					\
			bool call_ori = true;													\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),		\
				call_ori BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));			\
			if (call_ori)															\
				RealFuncKeeper<T>::real(BOOST_PP_ENUM_PARAMS(n, p));				\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),			\
				call_ori BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));			\
		}																			\
	};

#define GEN_FAKE_MEMBER(n, cc)														\
	template<typename R, typename C													\
		BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>					\
	struct Fake<R (cc C::*)(BOOST_PP_ENUM_PARAMS(n, P))> : public C {				\
		R cc Func(BOOST_PP_ENUM(n, TYPE_AND_PARAM, _)) {							\
			bool call_ori = true;													\
			R ret;																	\
			C *self = this;															\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),		\
				call_ori, self BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));	\
			if (call_ori)															\
				ret = (self->*RealFuncKeeper<T>::real)(BOOST_PP_ENUM_PARAMS(n, p));	\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),			\
				call_ori, ret, self													\
				BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));					\
			return ret;																\
		}																			\
	};	

#define GEN_FAKE_VOID_MEMBER(n, cc)													\
	template<typename C	BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>	\
	struct Fake<void (cc C::*)(BOOST_PP_ENUM_PARAMS(n, P))> : public C {			\
		void cc Func(BOOST_PP_ENUM(n, TYPE_AND_PARAM, _)) {							\
			bool call_ori = true;													\
			C *self = this;															\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),		\
				call_ori, self BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));	\
			if (call_ori)															\
				(self->*RealFuncKeeper<T>::real)(BOOST_PP_ENUM_PARAMS(n, p));		\
			::tracer::ForwardToFoldedParameters(									\
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),			\
				call_ori, self BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));	\
		}																			\
	};

#define GEN_FAKE_CC(n, cc)															\
	GEN_FAKE_FUNC(n, cc)															\
	GEN_FAKE_VOID_FUNC(n, cc)														\
	GEN_FAKE_MEMBER(n, cc)															\
	GEN_FAKE_VOID_MEMBER(n, cc)
	
			
#define GEN_FAKE_N(z, n, _)															\
	GEN_FAKE_CC(n, BOOST_PP_EMPTY())												\
	GEN_FAKE_CC(n, __stdcall)

#define GEN_FAKE																	\
	BOOST_PP_REPEAT(TRACER_ARG_LIMIT, GEN_FAKE_N, _)


namespace tracer {

template<typename T>
struct FakeFuncKeeper {
	template<typename S>
	static S *Pointer_(typename ::std::enable_if<
		!::std::is_member_function_pointer<S>::value>::type*);
	template<typename S>
	static S Pointer_(typename ::std::enable_if<
		::std::is_member_function_pointer<S>::value>::type*);
	typedef decltype(Pointer_<typename T::Signature>(0)) Pointer;
	static Pointer fake;

	template<typename T> struct Fake;
	GEN_FAKE
};

template<typename T>
typename FakeFuncKeeper<T>::Pointer FakeFuncKeeper<T>::fake = 
	reinterpret_cast<FakeFuncKeeper<T>::Pointer>(
	&FakeFuncKeeper<T>::Fake<FakeFuncKeeper<T>::Pointer>::Func);

}	// namespace tracer

#undef GEN_FAKE
#undef GEN_FAKE_N
#undef GEN_FAKE_CC
#undef TYPE_AND_PARAM