#include "gtest/gtest.h"
#include "trace.hpp"

namespace {

int F1(int a) {
	return a * a;
}

int F2(int a) {
	return a;
}

int F2(int a, int b) {
	return a + b;
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

TEST(TraceTest, NormalFuncTest) {
	TRACER_TRACE(&F1) f1;
	f1.Before().connect([] (bool&, int &a) {
		EXPECT_EQ(10, a);
	});
	auto real = f1.RealFunc();
	f1.After().connect([real] (bool, int &ret, int&) {
		EXPECT_EQ(100, ret);
		ret = real(ret);
	});
	EXPECT_EQ(10000, F1(10));
}

TEST(TraceTest, MemberFuncTest) {
	TRACER_TRACE(&C::Exchange) exchange;
	C a("ObjA"), b("ObjB");
	auto conn = exchange.Before().connect([&a, &b] (bool&, C *&self, const std::string &str) {
		EXPECT_EQ("something else", str);
		EXPECT_EQ(&a, self);
		self = &b;
	});
	EXPECT_EQ("ObjB", a.Exchange("something else"));
	conn.disconnect();
	EXPECT_EQ("something else", b.Exchange(""));
}

TEST(TraceTest, OverloadedFuncTest) {
	TRACER_TRACE(static_cast<int(*)(int)>(&F2)) f2;
	f2.Before().connect([](bool&, int &a) {
		a = 0;
	});
	EXPECT_EQ(0, F2(10));
	EXPECT_EQ(10, F2(5, 5));
}