#pragma once
#include <boost/preprocessor.hpp>
#include <type_traits>
#include <vector>
#include "function_type.hpp"
#include "signal.hpp"

#ifndef TRACER_ARG_LIMIT
#define TRACER_ARG_LIMIT 10
#endif

#define TYPE_AND_PARAM(z, n, _)															\
	BOOST_PP_CAT(P, n) BOOST_PP_CAT(p, n)

#define REMOVE_REF(p)																	\
	typename std::remove_reference<p>::type

#define CONST_REF(t)																	\
	typename std::add_const<typename std::add_lvalue_reference<typename t>::type>::type

#define GEN_IMPL_(n)																	\
	template<typename T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>							\
	class RetValRecorderImpl<T, void(BOOST_PP_ENUM_PARAMS(n, P))> {						\
		std::vector<REMOVE_REF(P1)> ret_vals_;											\
		Connection conn_;																\
	public:																				\
		RetValRecorderImpl(T &tracer) {													\
			conn_ = tracer.After().connect(												\
				[this] (BOOST_PP_ENUM(n, TYPE_AND_PARAM, _)) {							\
					ret_vals_.push_back(p1);											\
				});																		\
		}																				\
		~RetValRecorderImpl() {															\
			conn_.disconnect();															\
		}																				\
		CONST_REF(P1) RetVal(std::size_t i) {											\
			return ret_vals_.at(i);														\
		}																				\
	};

#define GEN_DUMMY_IMPL_(n)																\
	template<typename T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>		\
	class RetValRecorderImpl<T, void(BOOST_PP_ENUM_PARAMS(n, P))> {						\
	public:																				\
		RetValRecorderImpl(T &) {														\
			static_assert(0, "RetValRecorder can not record void."); 					\
		}																				\
	};

#define GEN_IMPL(z, n, _)																\
	BOOST_PP_IIF(BOOST_PP_LESS(n, 2), GEN_DUMMY_IMPL_, GEN_IMPL_)(n)

namespace tracer {

template<typename T, typename S> class RetValRecorderImpl;

BOOST_PP_REPEAT(TRACER_ARG_LIMIT, GEN_IMPL, _)

template<typename T>
class RetValRecorder : public RetValRecorderImpl<T, typename T::AfterSignal::Signature> {
public:
	RetValRecorder(T &tracer) :
		RetValRecorderImpl(tracer) 
	{}
};

template<typename T>
RetValRecorder<T> RecordRetVal(T& tracer) {
	return RetValRecorder<T>(tracer);
}

}	// namespace tracer

#undef GEN_IMPL
#undef GEN_DUMMY_IMPL_
#undef GEN_IMPL_
#undef CONST_REF
#undef REMOVE_REF
#undef TYPE_AND_PARAM