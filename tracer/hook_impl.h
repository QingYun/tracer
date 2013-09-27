/*!
\file
这个文件包含了两个 detours 的简单封装函数, Install 和 Remove
*/
#pragma once
#include <windows.h>

namespace tracer {

LONG InstallImpl(void **target, void *detour);
LONG RemoveImpl(void **target, void *detour);

/*!
\file hook_impl.h
\param [in, out] target 传参时, 这个指针应该指向要被hook的函数; 
返回后, 这个指针指向跳板函数, 调用跳板函数的效果等同于调用未被hook的原始函数
\param [in] detour 指向替换函数的指针
\return 返回`NO_ERROR`表示成功, 否则返回一个错误码:

- **ERROR_INVALID_BLOCK**

	目标函数太小, 以至于放不下用来hook的jmp指令

- **ERROR_INVALID_HANDLE**

	`target`为空

- **ERROR_NOT_ENOUGH_MEMORY**

	没有足够的内存用以完成操作
*/
template<typename T>
LONG Install(T **target, T *detour) {
	return InstallImpl(reinterpret_cast<void**>(target), 
		reinterpret_cast<void*>(detour));
}

/*!
\file hook_impl.h
\param [in, out] target 传参时, 这个指针指向跳板函数; 
返回后, 这个指针指向被解除hook的函数
\param [in] detour 指向替换函数的指针
\return 返回`NO_ERROR`表示成功, 否则返回一个错误码:

- **ERROR_INVALID_BLOCK**

	目标函数太小, 以至于放不下用来hook的jmp指令

- **ERROR_INVALID_HANDLE**

	`target`为空

- **ERROR_NOT_ENOUGH_MEMORY**

	没有足够的内存用以完成操作
*/
template<typename T>
LONG Remove(T **target, T *detour) {
	return RemoveImpl(reinterpret_cast<void**>(target), 
		reinterpret_cast<void*>(detour));
}

}	// namespace tracer