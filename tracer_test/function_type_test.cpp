#include "gtest/gtest.h"
#include <windows.h>
#include "function_type.hpp"

namespace tracer {

using namespace testing;

struct A {
	HRESULT WINAPI F0();
	HRESULT WINAPI F1(int);
	HRESULT WINAPI F2(char, short, int, long, long long, 
		unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long);
};

TEST(FunctionTypeTest, COMToNormalTest) {
	StaticAssertTypeEq<HRESULT __stdcall(A*), MemberToNormal<decltype(&A::F0)>::type>();
	StaticAssertTypeEq<HRESULT __stdcall(A*, int), MemberToNormal<decltype(&A::F1)>::type>();
	StaticAssertTypeEq<HRESULT __stdcall(A*, char, short, int, long, long long, 
		unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long), 
		MemberToNormal<decltype(&A::F2)>::type>();
}

void WINAPI F1();
int F2(double);
A F3(char, short, int, long, long long, 
	unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long);

TEST(FunctionTypeTest, PrependParameterTest) {
	StaticAssertTypeEq<void __stdcall(int), PrependParameter<decltype(F1), int>::type>();
	StaticAssertTypeEq<int(char&&, double), PrependParameter<decltype(F2), char&&>::type>();
	StaticAssertTypeEq<A(A*, char, short, int, long, long long, 
		unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long), PrependParameter<decltype(F3), A*>::type>();
	StaticAssertTypeEq<int(double), PrependParameter<decltype(F2), void>::type>();
}

void F4(char*, short&);
int WINAPI F5(int&&);

TEST(FunctionTypeTest, AllParamsToRefTest) {
	StaticAssertTypeEq<decltype(F1), AllParamsToRef<decltype(F1)>::type>();
	StaticAssertTypeEq<int(double&), AllParamsToRef<decltype(F2)>::type>();
	StaticAssertTypeEq<void(char*&, short&), AllParamsToRef<decltype(F4)>::type>();
	StaticAssertTypeEq<int __stdcall(int&), AllParamsToRef<decltype(F5)>::type>();
}

TEST(FunctionTypeTest, SetResultTypeTest) {
	StaticAssertTypeEq<int __stdcall(), SetResultType<decltype(F1), int>::type>();
	StaticAssertTypeEq<void (double), SetResultType<decltype(F2), void>::type>();
	StaticAssertTypeEq<char& (char*, short&), SetResultType<decltype(F4), char&>::type>();
	StaticAssertTypeEq<int&& __stdcall(int&&), SetResultType<decltype(F5), int&&>::type>();
}

TEST(FunctionTypeTest, RemoveStdcallTest) {
	StaticAssertTypeEq<void(), RemoveStdcall<decltype(F1)>::type>();
	StaticAssertTypeEq<int(int&&), RemoveStdcall<decltype(F5)>::type>();
}

char &F6();
int&& WINAPI F7();
A *F8();

TEST(FunctionTypeTest, ResultTypeTest) {
	StaticAssertTypeEq<void, ResultType<decltype(F1)>::type>();
	StaticAssertTypeEq<A, ResultType<decltype(F3)>::type>();
	StaticAssertTypeEq<char&, ResultType<decltype(F6)>::type>();
	StaticAssertTypeEq<int&&, ResultType<decltype(F7)>::type>();
	StaticAssertTypeEq<A*, ResultType<decltype(F8)>::type>();
}

}	// namespace tracer