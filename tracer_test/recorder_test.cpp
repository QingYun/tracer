#include "gtest/gtest.h"
#include "trace_normal_func.hpp"
#include "trace_member_func.hpp"
#include "call_count_recorder.hpp"

namespace {

int F1(int a) {
	return a * a;
}

class C {
	std::string str_;
public:
	C(const std::string &str) :
		str_(str) 
	{}
	std::string Exchange(const std::string &str) {
		std::string ret(std::move(str_));
		str_ = str;
		return ret;
	}
};

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