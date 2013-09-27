#include "hook_impl.h"
#include "detours/detours.h"
#include <TlHelp32.h>

namespace tracer {
namespace {

/*!
DetourUpdateThread 的作用是在事务提交之前挂起指定线程, 
这个函数会枚举所有线程并一一调用 DetourUpdateThread 将其挂起.
因为事务提交时会有代码修改的操作, 如果不挂起所有线程, 则有可能有
线程因为刚好执行到修改了一半的代码而崩溃.
*/
void DetourUpdateAllThreads()
{
	HANDLE snap_handle = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());
	if (INVALID_HANDLE_VALUE != snap_handle)
	{
		THREADENTRY32 te = {sizeof(te)};
		BOOL ok = Thread32First(snap_handle, &te);
		for (; ok; ok = Thread32Next(snap_handle, &te))
		{
			if (GetCurrentProcessId() == te.th32OwnerProcessID && GetCurrentThreadId() != te.th32ThreadID)
			{
				HANDLE thread_handle = OpenThread(THREAD_ALL_ACCESS, 
					FALSE, te.th32ThreadID);
				if (thread_handle)
				{
					DetourUpdateThread(thread_handle);
					CloseHandle(thread_handle);
				}
			}
		}
		CloseHandle(snap_handle);
	}
}

}

LONG InstallImpl( void **target, void *detour ) {
	DetourTransactionBegin();
	DetourUpdateAllThreads();
	DetourAttach(target, detour);
	return DetourTransactionCommit();
}

LONG RemoveImpl( void **target, void *detour ) {
	DetourTransactionBegin();
	DetourUpdateAllThreads();
	DetourDetach(target, detour);
	return DetourTransactionCommit();
}

}	// namespace tracer