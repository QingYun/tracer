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

TEST(FunctionTypeTest, FoldParametersTest) {
	StaticAssertTypeEq<decltype(F1), FoldParameters<decltype(F1)>::type>();
	StaticAssertTypeEq<decltype(F4), FoldParameters<decltype(F4)>::type>();
	typedef void Sig(int, int, int, int, int, int, int);
	StaticAssertTypeEq<Sig, FoldParameters<Sig>::type>();
	StaticAssertTypeEq<
		A(char, short, int, long, long long, unsigned char,  
		std::tuple<unsigned short, unsigned int, unsigned long, unsigned long long>), 
		FoldParameters<decltype(F3)>::type>();
}

void F9() {}

int F10(int a, double b) {
	EXPECT_EQ(1, a);
	EXPECT_DOUBLE_EQ(2.2, b);
	return a;
}

void F11(char p1, short p2, int p3, long p4, long long p5, unsigned char p6, 
	std::tuple<unsigned short, unsigned int, unsigned long, unsigned long long> p8) {
		EXPECT_EQ(1, p1);
		EXPECT_EQ(2, p2);
		EXPECT_EQ(3, p3);
		EXPECT_EQ(4, p4);
		EXPECT_EQ(5, p5);
		EXPECT_EQ(6, p6);
		unsigned short a;
		unsigned int b;
		unsigned long c;
		unsigned long long d;
		std::tie(a, b, c, d) = p8;
		EXPECT_EQ(7, a);
		EXPECT_EQ(8, b);
		EXPECT_EQ(9, c);
		EXPECT_EQ(10, d);
}


TEST(FunctionTypeTest, ForwardToFoldedParametersTest) {
	ForwardToFoldedParameters(F9);
	EXPECT_EQ(1, ForwardToFoldedParameters(F10, 1, 2.2));
	ForwardToFoldedParameters(F11, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
}

}	// namespace tracer