#pragma once
#include <boost/preprocessor.hpp>
#include <tuple>
#include <vector>
#include <functional>

#ifndef TRACER_ARG_LIMIT
#define TRACER_ARG_LIMIT 10
#endif

#define TYPE_AND_PARAM(z, n, _)															\
	BOOST_PP_CAT(P, n) BOOST_PP_CAT(p, n)

#define REMOVE_REF(z, n, _)																\
	typename std::remove_reference<BOOST_PP_CAT(P, n)>::type

#define GEN_IMPL(z, n, _)																\
	template<typename T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>		\
	class ArgRecorderImpl<T, void(BOOST_PP_ENUM_PARAMS(n, P))> {						\
		typedef std::tuple<BOOST_PP_ENUM_SHIFTED(n, REMOVE_REF, _)> Tuple;				\
		std::vector<Tuple> args_;														\
		Connection conn_;																\
	public:																				\
		ArgRecorderImpl(T &tracer) {													\
			conn_ = tracer.Before().connect(std::ref(*this));							\
		}																				\
		~ArgRecorderImpl() {															\
			conn_.disconnect();															\
		}																				\
		void operator () (BOOST_PP_ENUM(n, TYPE_AND_PARAM, _)) {						\
			args_.push_back(Tuple(BOOST_PP_ENUM_SHIFTED_PARAMS(n, p)));					\
		}																				\
		template<std::size_t I>															\
		typename std::remove_reference<decltype(std::get<I>(Tuple()))>::type			\
		Arg(std::size_t i) {															\
			return std::get<I>(args_.at(i));											\
		}																				\
	};

namespace tracer {

template<typename T, typename S> class ArgRecorderImpl;

BOOST_PP_REPEAT(TRACER_ARG_LIMIT, GEN_IMPL, _)

template<typename T>
class ArgRecorder : public ArgRecorderImpl<T, typename T::BeforeSignal::Signature> {
public:
	ArgRecorder(T &tracer) : 
		ArgRecorderImpl(tracer) 
	{}
};

template<typename T>
ArgRecorder<T> RecordArgs(T& tracer) {
	return ArgRecorder<T>(std::forward<T>(tracer));
}
}	// namespace tracer

#undef GEN_IMPL
#undef REMOVE_REF
#undef TYPE_AND_PARAM