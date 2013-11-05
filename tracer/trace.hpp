#pragma once
#include <boost/preprocessor.hpp>
#include "function_type.hpp"
#include "signal_keeper.hpp"
#include "fake_func_keeper.hpp"
#include "real_func_keeper.hpp"
#include "hook_manager.h"

namespace tracer {

template<typename S, typename Enable = void> struct BeforeSignalSignature;
template<typename S, typename Enable = void> struct AfterSignalSignature;

template<typename S>
struct BeforeSignalSignature<S, 
	typename std::enable_if<std::is_member_function_pointer<S>::value>::type> {
	typedef								
		typename AllParamsToRef<								
			typename PrependParameter<							
				typename SetResultType<						
					typename MemberToNormal<S>::type,	
					void										
				>::type,										
				bool											
			>::type												
		>::type type;												
};

template<typename S>
struct BeforeSignalSignature<S,
	typename std::enable_if<!std::is_member_function_pointer<S>::value>::type> {
	typedef 								
		typename AllParamsToRef<								
			typename PrependParameter<							
				typename SetResultType<S, void>::type,	
				bool											
			>::type												
		>::type	type;												
};

template<typename S>
struct AfterSignalSignature<S,
	typename std::enable_if<std::is_member_function_pointer<S>::value>::type> {
	typedef 											
		typename PrependParameter<										
			typename AllParamsToRef<									
				typename PrependParameter<								
					typename SetResultType<							
						typename MemberToNormal<S>::type,		
						void											
					>::type,											
					typename ResultType<								
						typename MemberToNormal<S>::type		
					>::type												
				>::type													
			>::type,													
			bool														
		>::type	type;															
};

template<typename S>
struct AfterSignalSignature<S,
	typename std::enable_if<!std::is_member_function_pointer<S>::value>::type> {
	typedef 									
		typename PrependParameter<									
			typename AllParamsToRef<								
				typename PrependParameter<							
					typename SetResultType<S, void>::type,	
					typename ResultType<S>::type			
				>::type												
			>::type,												
			bool													
		>::type	type;													
};

}	// namespace tracer

#define TRACER_CLASS_NAME BOOST_PP_CAT(Tracer, __LINE__)

#define TRACER_TRACE(func)																	\
	struct TRACER_CLASS_NAME {																\
		typedef ::std::remove_pointer<decltype(func)>::type Signature;						\
		struct BeforeSignal {																\
			typedef ::tracer::BeforeSignalSignature<Signature>::type Signature;				\
		};																					\
		struct AfterSignal {																\
			typedef ::tracer::AfterSignalSignature<Signature>::type Signature;				\
		};																					\
	private:																				\
		::tracer::Signal<BeforeSignal::Signature> &before_signal_;							\
		::tracer::Signal<AfterSignal::Signature> &after_signal_;							\
	public:																					\
		TRACER_CLASS_NAME() :																\
			before_signal_(::tracer::SignalKeeper<BeforeSignal>::signal),					\
			after_signal_(::tracer::SignalKeeper<AfterSignal>::signal) {					\
			::tracer::RealFuncKeeper<TRACER_CLASS_NAME>::real = func;						\
			if (NO_ERROR != ::tracer::HookManager::Instance().Install(						\
				reinterpret_cast<void**>(&::tracer::RealFuncKeeper<							\
					TRACER_CLASS_NAME>::real),												\
				reinterpret_cast<void*&>(::tracer::FakeFuncKeeper<							\
					TRACER_CLASS_NAME>::fake)))												\
				throw ::std::runtime_error(BOOST_PP_STRINGIZE(								\
						BOOST_PP_CAT(BOOST_PP_CAT(Hook,										\
							TRACER_CLASS_NAME), fail)));									\
		}																					\
		~TRACER_CLASS_NAME() {																\
			::tracer::HookManager::Instance().Remove(reinterpret_cast<void**>(				\
				&::tracer::RealFuncKeeper<TRACER_CLASS_NAME>::real));						\
		}																					\
		::tracer::Signal<BeforeSignal::Signature> &Before() { return before_signal_; }		\
		::tracer::Signal<AfterSignal::Signature> &After() { return after_signal_; }			\
		::tracer::FunctionPointer<Signature>::type RealFunc() {								\
			return ::tracer::RealFuncKeeper<TRACER_CLASS_NAME>::real;						\
		}																					\
	}
