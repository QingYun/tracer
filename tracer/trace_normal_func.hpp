#pragma once
#include <boost/preprocessor.hpp>
#include "function_type.hpp"
#include "signal_keeper.hpp"
#include "fake_func_keeper.hpp"
#include "real_func_keeper.hpp"
#include "hook_manager.h"

#define TRACER_TRACE_NORMAL_FUNC_GEN_BEFORE_SIGNAL										\
	struct BeforeSignal {																\
		typedef																			\
			::tracer::FoldParameters<													\
				::tracer::AllParamsToRef<												\
					::tracer::PrependParameter<											\
						::tracer::SetResultType<Signature, void>::type,					\
						bool															\
					>::type																\
				>::type																	\
			>::type Signature;															\
	};
	 
#define TRACER_TRACE_NORMAL_FUNC_GEN_AFTER_SIGNAL										\
	struct AfterSignal {																\
		typedef																			\
			::tracer::FoldParameters<													\
				::tracer::PrependParameter<												\
					::tracer::AllParamsToRef<											\
						::tracer::PrependParameter<										\
							::tracer::SetResultType<Signature, void>::type,				\
							::tracer::ResultType<Signature>::type						\
						>::type															\
					>::type,															\
					bool																\
				>::type																	\
			>::type Signature;															\
	};

#define TRACER_TRACE_NORMAL_FUNC_CLASS_NAME BOOST_PP_CAT(Tracer, __LINE__)

#define TRACER_TRACE_NORMAL_FUNC(func)													\
	struct TRACER_TRACE_NORMAL_FUNC_CLASS_NAME {										\
		typedef decltype(func) Signature;												\
		TRACER_TRACE_NORMAL_FUNC_GEN_BEFORE_SIGNAL										\
		TRACER_TRACE_NORMAL_FUNC_GEN_AFTER_SIGNAL										\
	private:																			\
		::tracer::Signal<BeforeSignal::Signature> &before_signal_;						\
		::tracer::Signal<AfterSignal::Signature> &after_signal_;						\
	public:																				\
		TRACER_TRACE_NORMAL_FUNC_CLASS_NAME() :											\
			before_signal_(::tracer::SignalKeeper<BeforeSignal>::signal),				\
			after_signal_(::tracer::SignalKeeper<AfterSignal>::signal) {				\
			::tracer::RealFuncKeeper<TRACER_TRACE_NORMAL_FUNC_CLASS_NAME>::real = func;	\
			if (NO_ERROR != ::tracer::HookManager::Instance().Install(					\
				&::tracer::RealFuncKeeper<												\
					TRACER_TRACE_NORMAL_FUNC_CLASS_NAME>::real,							\
				::tracer::FakeFuncKeeper<												\
					TRACER_TRACE_NORMAL_FUNC_CLASS_NAME>::fake))						\
				throw ::std::runtime_error(BOOST_PP_STRINGIZE(							\
						BOOST_PP_CAT(BOOST_PP_CAT(Hook,									\
							TRACER_TRACE_NORMAL_FUNC_CLASS_NAME), fail)));				\
		}																				\
		~TRACER_TRACE_NORMAL_FUNC_CLASS_NAME() {										\
			::tracer::HookManager::Instance().Remove(									\
				&::tracer::RealFuncKeeper<TRACER_TRACE_NORMAL_FUNC_CLASS_NAME>::real);	\
		}																				\
		::tracer::Signal<BeforeSignal::Signature> &Before() { return before_signal_; }	\
		::tracer::Signal<AfterSignal::Signature> &After() { return after_signal_; }		\
		Signature *RealFunc() { return ::tracer::RealFuncKeeper<						\
										TRACER_TRACE_NORMAL_FUNC_CLASS_NAME>::real; }	\
	}
