#include "gtest/gtest.h"
#include "trace.hpp"
#include "call_count_recorder.hpp"
#include "arg_recorder.hpp"
#include "ret_val_recorder.hpp"
#include "call_stack_recorder.hpp"
#include "mixin_tracer.hpp"

namespace {

int F1(int a) {
	return a * a;
}

void F2(int a) {
	if (a != 0)
		F2(a - 1);
}

int Fac(int n) {
	if (n == 0)
		return 1;
	return n * Fac(n - 1);
}

struct C {
	void Foo() {}
};

}

void RunFoo() {
	C c;
	c.Foo();
}

TEST(RecorderTest, CallCountTest) {
	TRACER_TRACE(F1) f1;
	auto f1c = tracer::RecordCallCount(f1);
	F1(10);
	EXPECT_EQ(true, f1c.HasBeenCalled());
	F1(20);
	EXPECT_EQ(2, f1c.CallCount());
	EXPECT_EQ(100, f1.RealFunc()(10));
	EXPECT_EQ(2, f1c.CallCount());
}

TEST(RecorderTest, ArgRecorderTest) {
	TRACER_TRACE(F2) f2;
	auto f2a = tracer::RecordArgs(f2);
	int a = 5;
	F2(a);
	for (int i = a; i >= 0; i--) 
		EXPECT_EQ(5 - i, f2a.Arg<0>(i));
}

TEST(RecorderTest, RetValRecorderTest) {
	TRACER_TRACE(Fac) fac;
	auto facr = tracer::RecordRetVal(fac);
	Fac(3);
	int result[] = {1, 1, 2, 6};
	for (int i = 0; i < 4; ++i) 
		EXPECT_EQ(result[i], facr.RetVal(i));
}

TEST(RecorderTest, CallStackRecorderTest) {
	TRACER_TRACE(C::Foo) foo;
	auto fc = tracer::RecordCallStack(foo);

	RunFoo();
	for (auto itr : fc.GetCallStack(0).Entries())
		;//std::cout << itr.File() << " " << itr.Line() << " " << itr.FuncName() << std::endl;
	EXPECT_EQ(true, fc.GetCallStack(0).IsCalledBy("RunFoo"));
	
	void(*f)();
	f = [] () { RunFoo(); };
	f();
	EXPECT_EQ(true, fc.GetCallStack(1).IsCalledBy(f));
}

TEST(RecorderTest, MixinTest) {
	TRACER_TRACE_WITH(
		C::Foo, (tracer::CallCountRecorder)(tracer::CallStackRecorder)) foo;
	EXPECT_EQ(false, foo.HasBeenCalled());

	RunFoo();
	EXPECT_EQ(1, foo.CallCount());
	EXPECT_EQ(true, foo.GetCallStack(0).IsCalledBy(RunFoo));
}