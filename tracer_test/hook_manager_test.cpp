#include "gtest/gtest.h"
#include "hook_manager.h"
#include <windows.h>

namespace tracer {
namespace { 

	std::size_t StrSize(std::string str) {
		return str.size();
	}

	decltype(StrSize) *RealStrSize = StrSize; 
	std::string s2("12345");
	std::size_t FakeStrSize(std::string str) {
		EXPECT_EQ(4, RealStrSize(str));
		return RealStrSize(s2);
	}

}

TEST(HookManagerTest, NormalFuncTest) {
	HookManager hm;
	std::string s1("abcd");

	EXPECT_EQ(s1.size(), StrSize(s1));

	hm.Install(&RealStrSize, FakeStrSize);
	EXPECT_EQ(s2.size(), StrSize(s1));

	hm.Remove(&RealStrSize);
	EXPECT_EQ(s1.size(), StrSize(s1));
}

TEST(HookManagerTest, DtorTest) {
	std::string s1("abcd");
	{
		HookManager hm;

		EXPECT_EQ(s1.size(), StrSize(s1));

		hm.Install(&RealStrSize, FakeStrSize);
		EXPECT_EQ(s2.size(), StrSize(s1));
	}
	EXPECT_EQ(s1.size(), StrSize(s1));
}

namespace {

	decltype(GetCurrentProcessId) *RealGetCurrentProcessId = GetCurrentProcessId;
	DWORD WINAPI FakeGetCurrentProcessId() {
		return 0x1234;
	}

}

TEST(HookManagerTest, APITest) {
	HookManager hm;
	DWORD pid = GetCurrentProcessId();

	hm.Install(&RealGetCurrentProcessId, FakeGetCurrentProcessId);
	EXPECT_EQ(0x1234, GetCurrentProcessId());

	hm.Remove(&RealGetCurrentProcessId);
	EXPECT_EQ(pid, GetCurrentProcessId());
}

}	// namespace tracer