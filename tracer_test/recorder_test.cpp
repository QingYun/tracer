#include "gtest/gtest.h"
#include "trace_normal_func.hpp"
#include "trace_member_func.hpp"
#include "call_count_recorder.hpp"
#include "arg_recorder.hpp"

namespace {

int F1(int a) {
	return a * a;
}

void F2(int a) {
	if (a != 0)
		F2(a - 1);
}

}

TEST(RecorderTest, CallCountTest) {
	TRACER_TRACE_NORMAL_FUNC(F1) f1;
	auto f1c = tracer::RecordCallCount(f1);
	F1(10);
	EXPECT_EQ(true, f1c.HasBeenCalled());
	F1(20);
	EXPECT_EQ(2, f1c.CallCount());
	EXPECT_EQ(100, f1.RealFunc()(10));
	EXPECT_EQ(2, f1c.CallCount());
}

TEST(RecorderTest, ArgRecorderTest) {
	TRACER_TRACE_NORMAL_FUNC(F2) f2;
	auto f2a = tracer::RecordArgs(f2);
	int a = 5;
	F2(a);
	for (int i = a; i >= 0; i--) 
		EXPECT_EQ(5 - i, f2a.Arg<0>(i));
}