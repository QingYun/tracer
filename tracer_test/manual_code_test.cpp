#include "gtest/gtest.h"
#include "hook_manager.h"
#include "signal.hpp"
#include "function_type.hpp"
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")

namespace tracer {
namespace {

template<typename T>
struct SignalKeeper {
	static Signal<typename T::Signature> signal;
};
template<typename T>
Signal<typename T::Signature> SignalKeeper<T>::signal;

template<typename T>
struct RealFuncKeeper {
	template<typename S>
	static S *Pointer_(typename ::std::enable_if<
		!::std::is_member_function_pointer<S>::value>::type*);
	template<typename S>
	static S Pointer_(typename ::std::enable_if<
		::std::is_member_function_pointer<S>::value>::type*);
	typedef decltype(Pointer_<typename T::Signature>(0)) Pointer;
	static Pointer real;
};
template<typename T>
typename RealFuncKeeper<T>::Pointer RealFuncKeeper<T>::real;

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
	template<typename R>
	struct Fake<R (*)()> {							
		static R Func() {				
			bool call_ori = true;												
			R ret;																
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),
				call_ori);		
			if (call_ori)														
				ret = RealFuncKeeper<T>::real();		
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori, ret);	
			return ret;															
		}																		
	};																			
	template<>								
	struct Fake<void (*)()> {							
		static void Func() {			
			bool call_ori = true;												
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),	
				call_ori);		
			if (call_ori)														
				RealFuncKeeper<T>::real();			
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori);		
		}																		
	};
	template<typename R, typename C>
	struct Fake<R (C::*)()> : C {							
		R Func() {				
			bool call_ori = true;												
			R ret;		
			C *self = this;
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),
				call_ori, self);		
			if (call_ori)														
				ret = (self->*RealFuncKeeper<T>::real)();		
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori, ret, self);	
			return ret;															
		}																		
	};
	template<typename R>
	struct Fake<R (__stdcall *)()> {							
		static R __stdcall Func() {				
			bool call_ori = true;												
			R ret;																
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),	
				call_ori);		
			if (call_ori)														
				ret = RealFuncKeeper<T>::real();		
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori, ret);	
			return ret;															
		}																		
	};																			
	template<>								
	struct Fake<void (__stdcall *)()> {							
		static void __stdcall Func() {			
			bool call_ori = true;												
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),	
				call_ori);		
			if (call_ori)														
				RealFuncKeeper<T>::real();			
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori);		
		}																		
	};
	template<typename R, typename P0>
	struct Fake<R (*)(P0)> {							
		static R Func(P0 p0) {				
			bool call_ori = true;												
			R ret;																
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),
				call_ori, p0);		
			if (call_ori)														
				ret = RealFuncKeeper<T>::real(p0);		
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori, ret, p0);	
			return ret;															
		}																		
	};																			
	template<typename P0>								
	struct Fake<void (*)(P0)> {							
		static void Func(P0 p0) {			
			bool call_ori = true;												
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),	
				call_ori, p0);		
			if (call_ori)														
				RealFuncKeeper<T>::real(p0);			
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori, p0);		
		}																		
	};
	template<typename R, typename P0>
	struct Fake<R (__stdcall *)(P0)> {							
		static R __stdcall Func(P0 p0) {				
			bool call_ori = true;												
			R ret;																
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),	
				call_ori, p0);		
			if (call_ori)														
				ret = RealFuncKeeper<T>::real(p0);		
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori, ret, p0);	
			return ret;															
		}																		
	};																			
	template<typename P0>								
	struct Fake<void (__stdcall *)(P0)> {							
		static void __stdcall Func(P0 p0) {			
			bool call_ori = true;												
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::BeforeSignal>::signal),	
				call_ori, p0);		
			if (call_ori)														
				RealFuncKeeper<T>::real(p0);			
			::tracer::ForwardToFoldedParameters(								
				::std::ref(::tracer::SignalKeeper<T::AfterSignal>::signal),		
				call_ori, p0);		
		}																		
	};
};
template<typename T>
typename FakeFuncKeeper<T>::Pointer FakeFuncKeeper<T>::fake = 
	reinterpret_cast<FakeFuncKeeper<T>::Pointer>(
		&FakeFuncKeeper<T>::Fake<FakeFuncKeeper<T>::Pointer>::Func);

template<typename T>
struct CallRecorder;
template<typename R>
struct CallRecorder<R()> {
	class RecordItem {
		R ret_;
		typedef std::tuple<> ArgTuple;
		ArgTuple args_;
	public:
		RecordItem(const R &ret) :
			ret_(ret) {}
		const R &ReturnVal() const { return ret_; }
		const ArgTuple &Args() const { return args_; }
	};
	std::vector<RecordItem> records;
	void operator () (bool call_ori, R &ret) {
		records.emplace_back(ret);
	}
};
template<typename R, typename P>
struct CallRecorder<R(P)> {
	class RecordItem {
		R ret_;
		typedef std::tuple<P> ArgTuple;
		ArgTuple args_;
	public:
		RecordItem(const R &ret, const P &p) :
			ret_(ret),
			args_(p) {}
		const R &ReturnVal()const { return ret_; }
		const ArgTuple &Args() const { return args_; }
		template<std::size_t N>
		const typename std::tuple_element<N, ArgTuple>::type &Arg() const {
			return std::get<N>(args_);
		}
	};
	std::vector<RecordItem> records;
	void operator () (bool call_ori, R &ret, P &p) {
		records.emplace_back(ret, p);
	}
};
template<typename R, typename P1, typename P2>
struct CallRecorder<R(P1, P2)> {
	class RecordItem {
		R ret_;
		typedef std::tuple<P1, P2> ArgTuple;
		ArgTuple args_;
	public:
		RecordItem(const R &ret, const P1 &p1, const P2 &p2) :
			ret_(ret),
			args_(p1, p2) {}
		const R &ReturnVal() const { return ret_; }
		const ArgTuple &Args() const { return args_; }
		template<std::size_t N>
		const std::tuple_element<N, ArgTuple> &Arg() const {
			return std::get<N>(args_);
		}
	};
	std::vector<RecordItem> records;
	void operator () (bool call_ori, R &ret, P1 &p1, P2 &p2) {
		records.emplace_back(ret, p1, p2);
	}
};

}

namespace {

int Inc(int a) {
	return a + 1;
}

int Add5(int a) {
	for (int i = 0; i < 5; ++i)
		a = Inc(a);
	return a;
}

}

TEST(ManualCodeTest, NormalFuncTest) {
	struct IncTracer {
		typedef decltype(Inc) Signature;
		struct BeforeSignal {
			typedef														
				FoldParameters<								
					AllParamsToRef<								
						PrependParameter<								
							SetResultType<Signature, void>::type,			
							bool													
						>::type													
					>::type													
				>::type Signature;
		};
		struct AfterSignal {
			typedef													
				FoldParameters<							
					PrependParameter<							
						AllParamsToRef<							
							PrependParameter<							
								SetResultType<Signature, void>::type,		
								ResultType<Signature>::type				
							>::type												
						>::type,											
						bool												
					>::type												
				>::type Signature;						
		};

		Signal<BeforeSignal::Signature> &before_signal;
		Signal<AfterSignal::Signature> &after_signal;
		CallRecorder<Signature> recorder;

		IncTracer() : 
			before_signal(SignalKeeper<BeforeSignal>::signal),
			after_signal(SignalKeeper<AfterSignal>::signal) {
			RealFuncKeeper<IncTracer>::real = tracer::Inc;
			HookManager::Instance().Install(
				&RealFuncKeeper<IncTracer>::real,
				FakeFuncKeeper<IncTracer>::fake
			);
			After().connect(std::ref(recorder));
		}
		~IncTracer() {
			HookManager::Instance().Remove(&RealFuncKeeper<IncTracer>::real);
		}

		Signal<BeforeSignal::Signature> &Before() { return before_signal; }
		Signal<AfterSignal::Signature> &After() { return after_signal; }
		Signature *RealFunc() { return RealFuncKeeper<IncTracer>::real; }
		bool HasBeenCalled() { return !recorder.records.empty(); }
		const decltype(CallRecorder<Signature>::records) &
			CallRecords() { return recorder.records; }
	} Inc;
	EXPECT_EQ(false, Inc.HasBeenCalled());
	EXPECT_EQ(15, Add5(10));
	EXPECT_EQ(true, Inc.HasBeenCalled());
	EXPECT_EQ(5, Inc.CallRecords().size());
	EXPECT_EQ(11, Inc.CallRecords()[1].Arg<0>());
	EXPECT_EQ(12, Inc.CallRecords()[1].ReturnVal());
}

class C {
	std::string name_;
public:
	C(const std::string &name) : 
		name_(name) {}
	std::string GetName() {
		return name_;
	}
};

TEST(ManualCodeTest, MemberFuncTest) {
	struct C_GetNameTracer {
		typedef decltype(&C::GetName) Signature;
		struct BeforeSignal {
			typedef														
				FoldParameters<								
					AllParamsToRef<								
						PrependParameter<								
							SetResultType<
								MemberToNormal<Signature>::type,
								void
							>::type,			
							bool													
						>::type													
					>::type													
				>::type Signature;
		};
		struct AfterSignal {
			typedef													
				FoldParameters<							
					PrependParameter<							
						AllParamsToRef<							
							PrependParameter<							
								SetResultType<
									MemberToNormal<Signature>::type, 
									void
								>::type,		
								ResultType<
									MemberToNormal<Signature>::type
								>::type				
							>::type												
						>::type,											
						bool												
					>::type												
				>::type Signature;						
		};

		Signal<BeforeSignal::Signature> &before_signal;
		Signal<AfterSignal::Signature> &after_signal;

		C_GetNameTracer() : 
			before_signal(SignalKeeper<BeforeSignal>::signal),
			after_signal(SignalKeeper<AfterSignal>::signal) {
				RealFuncKeeper<C_GetNameTracer>::real = &C::GetName;
				HookManager::Instance().Install(
					reinterpret_cast<void**>(&RealFuncKeeper<C_GetNameTracer>::real),
					reinterpret_cast<void*&>(FakeFuncKeeper<C_GetNameTracer>::fake)
					);
		}
		~C_GetNameTracer() {
			HookManager::Instance().Remove(
				reinterpret_cast<void**>(&RealFuncKeeper<C_GetNameTracer>::real));
		}

		Signal<BeforeSignal::Signature> &Before() { return before_signal; }
		Signal<AfterSignal::Signature> &After() { return after_signal; }
		Signature RealFunc() { return RealFuncKeeper<C_GetNameTracer>::real; }
	} GetName;
	C a("a"), b("b");
	GetName.Before().connect([&b] (bool&, C *&self) {
		self = &b;
	});
	EXPECT_EQ("b", a.GetName());
}

}	// namespace tracer