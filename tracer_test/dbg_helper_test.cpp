#include "gtest/gtest.h"
#include "dbg_helper.h"

namespace tracer {

void Foo() {}

TEST(DbgHelperTest, GetSymbolNameTest) {
	EXPECT_NO_THROW(DbgHelper::Instance().GetSymbolName(reinterpret_cast<DWORD64>(Foo)));
}

TEST(DbgHelperTest, StackWalkTest) {
	CONTEXT c;
	c.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&c);
	for (auto itr = DbgHelper::Instance().StackWalk(&c); itr; ++itr) {
		EXPECT_NO_THROW(DbgHelper::Instance().GetSymbolName(itr->AddrPC.Offset));
		EXPECT_NO_THROW(DbgHelper::Instance().GetFileName(itr->AddrPC.Offset)); 
		EXPECT_NO_THROW(DbgHelper::Instance().GetLine(itr->AddrPC.Offset));
	}
}

}	// namespace tracer