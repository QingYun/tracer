#include "gtest/gtest.h"
#include "hook_class_maker.hpp"
#include <windows.h>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

namespace tracer {

#define FUNC_LIST \
	(GetCurrentThreadId)

TRACER_HOOK_CLASS_DECL(Kernel32, TRACER_HOOK_NORMAL, , FUNC_LIST)
TRACER_HOOK_CLASS_DEF(Kernel32, TRACER_HOOK_NORMAL, FUNC_LIST)
#undef FUNC_LIST

TEST(GeneratedHookWrapperTest, APITest) {
	DWORD tid = GetCurrentThreadId();
	int count = 0;

	ASSERT_EQ(NO_ERROR, Kernel32::Hook::GetCurrentThreadId());
	auto before_conn = Kernel32::Before::GetCurrentThreadId::Call(
		[&count] (bool &) mutable {
			count++;
	});
	Kernel32::Before::GetCurrentThreadId::CallOnce(
		[&count] (bool &) mutable {
			count++;
	});

	EXPECT_EQ(tid, GetCurrentThreadId());
	EXPECT_EQ(2, count);
	GetCurrentThreadId();
	EXPECT_EQ(3, count);

	auto after_conn = Kernel32::After::GetCurrentThreadId::Call(
		[] (bool, DWORD &ret) {
			ret = 0x1234;
	});
	Kernel32::After::GetCurrentThreadId::CallOnce(
		[] (bool, DWORD &ret) {
			ret = 0x2345;
	});

	EXPECT_EQ(0x2345, GetCurrentThreadId());
	EXPECT_EQ(4, count);
	EXPECT_EQ(0x1234, GetCurrentThreadId());
	EXPECT_EQ(5, count);

	before_conn.disconnect();
	EXPECT_EQ(0x1234, GetCurrentThreadId());
	EXPECT_EQ(5, count);

	after_conn.disconnect();
	EXPECT_EQ(tid, GetCurrentThreadId());

	ASSERT_EQ(NO_ERROR, Kernel32::Unhook::GetCurrentThreadId());
}

namespace test {
	void NoReturn(int &a) {
		EXPECT_EQ(1, a);
		a = 10;
	}
}

#define FUNC_LIST \
	(NoReturn)

TRACER_HOOK_CLASS_DECL(NoReturnTest, TRACER_HOOK_NORMAL, ::tracer::test, FUNC_LIST)
TRACER_HOOK_CLASS_DEF(NoReturnTest, TRACER_HOOK_NORMAL, FUNC_LIST)
#undef FUNC_LIST

TEST(GeneratedHookWrapperTest, NoRetrunTest) {
	ASSERT_EQ(NO_ERROR, NoReturnTest::Hook::NoReturn());
	NoReturnTest::Before::NoReturn::Call([] (bool&, int &a) {
		EXPECT_EQ(0, a);
		a = 1;
	});
	NoReturnTest::After::NoReturn::Call([] (bool, int &a) {
		EXPECT_EQ(10, a);
		a = 20;
	});
	int val = 0;
	test::NoReturn(val);
	EXPECT_EQ(20, val);
	ASSERT_EQ(NO_ERROR, NoReturnTest::Unhook::NoReturn());
}

#define FUNC_LIST \
	((GetDeviceCaps, 14))

TRACER_HOOK_CLASS_DECL(IDirect3D9, TRACER_HOOK_COM, ::IDirect3D9, FUNC_LIST)
TRACER_HOOK_CLASS_DEF(IDirect3D9, TRACER_HOOK_COM, FUNC_LIST) {
	return Direct3DCreate9(D3D_SDK_VERSION);
}
#undef FUNC_LIST

TEST(GeneratedHookWrapperTest, COMTest) {
	D3DCAPS9 caps;
	LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);

	ASSERT_EQ(NO_ERROR, IDirect3D9::Hook::GetDeviceCaps());
	IDirect3D9::Before::GetDeviceCaps::Call(
		[&caps] (bool& call_ori, LPDIRECT3D9&, UINT &Adapter, 
		D3DDEVTYPE &DeviceType, D3DCAPS9 *&pCaps) {
			EXPECT_EQ(D3DADAPTER_DEFAULT, Adapter);
			EXPECT_EQ(D3DDEVTYPE_HAL, DeviceType);
			EXPECT_EQ(&caps, pCaps);
			pCaps = NULL;
			call_ori = false;
	});
	IDirect3D9::After::GetDeviceCaps::Call(
		[d3d] (bool call_ori, HRESULT &ret, LPDIRECT3D9 &self, 
		UINT&, D3DDEVTYPE&, D3DCAPS9 *&pCaps) {
			EXPECT_EQ(d3d, self);
			EXPECT_EQ(false, call_ori);
			EXPECT_EQ(NULL, pCaps);
			ret = D3D_OK;
	});

	HRESULT hr = d3d->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	EXPECT_HRESULT_SUCCEEDED(hr);
	ASSERT_EQ(NO_ERROR, IDirect3D9::Unhook::GetDeviceCaps());
}

}	// namespace tracer