#pragma once
#include <boost/preprocessor.hpp>
#include "trace_normal_func.hpp"
#include "trace_member_func.hpp"

#define TRACER_MIXIN_TRACER_INHERIT_LIST(_, i, tuple)									\
	public BOOST_PP_SEQ_ELEM(i, BOOST_PP_TUPLE_ELEM(2, 1, tuple))						\
		<BOOST_PP_TUPLE_ELEM(2, 0, tuple)>

#define TRACER_MIXIN_TRACER_INIT_LIST(_, i, tuple)										\
	BOOST_PP_SEQ_ELEM(i, BOOST_PP_TUPLE_ELEM(2, 1, tuple))								\
		<BOOST_PP_TUPLE_ELEM(2, 0, tuple)>(												\
			*static_cast<BOOST_PP_TUPLE_ELEM(2, 0, tuple)*>(this))

#define TRACER_MIXIN_TRACER_NAME														\
	BOOST_PP_CAT(MixinTracer, __LINE__)

#define TRACER_TRACE_MEMBER_FUNC_WITH(func, recorders)									\
	TRACER_TRACE_MEMBER_FUNC(func);														\
	struct TRACER_MIXIN_TRACER_NAME :													\
		public TRACER_TRACE_MEMBER_FUNC_CLASS_NAME										\
		BOOST_PP_COMMA_IF(BOOST_PP_SEQ_SIZE(recorders))									\
		BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(recorders), TRACER_MIXIN_TRACER_INHERIT_LIST,	\
			(TRACER_TRACE_MEMBER_FUNC_CLASS_NAME, recorders))	{						\
		TRACER_MIXIN_TRACER_NAME() BOOST_PP_IF(BOOST_PP_SEQ_SIZE(recorders), :, )		\
			BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(recorders), TRACER_MIXIN_TRACER_INIT_LIST,	\
				(TRACER_TRACE_MEMBER_FUNC_CLASS_NAME, recorders))						\
		{}																				\
	}

#define TRACER_TRACE_NORMAL_FUNC_WITH(func, recorders)									\
	TRACER_TRACE_NORMAL_FUNC(func);														\
	struct TRACER_MIXIN_TRACER_NAME :													\
		public TRACER_TRACE_NORMAL_FUNC_CLASS_NAME										\
		BOOST_PP_COMMA_IF(BOOST_PP_SEQ_SIZE(recorders))									\
		BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(recorders), TRACER_MIXIN_TRACER_INHERIT_LIST,	\
			(TRACER_TRACE_NORMAL_FUNC_CLASS_NAME, recorders))	{						\
		TRACER_MIXIN_TRACER_NAME() BOOST_PP_IF(BOOST_PP_SEQ_SIZE(recorders), :, )		\
			BOOST_PP_ENUM(BOOST_PP_SEQ_SIZE(recorders), TRACER_MIXIN_TRACER_INIT_LIST,	\
				(TRACER_TRACE_NORMAL_FUNC_CLASS_NAME, recorders))						\
		{}																				\
	}
