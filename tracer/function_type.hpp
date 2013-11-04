#pragma once
#include <type_traits>

namespace tracer {

/*!
\brief 根据成员函数指针生成对应的普通函数签名.

比如对于`HRESULT (__stdcall C::*)(int, double)`, 
则会生成`HRESULT __stdcall type(C*, int, double)`.
*/
template<typename T>
struct MemberToNormal;

template<typename R, typename C, typename... Args>
struct MemberToNormal<R(C::*)(Args...)> {
	typedef R type(C*, Args...);
};

template<typename R, typename C, typename... Args>
struct MemberToNormal<R(__stdcall C::*)(Args...)> {
	typedef R __stdcall type(C*, Args...);
};

/*!
\brief 在参数列表头部插入一个参数

比如对于`<void __stdcall (int), int&>`, 则会生成`void __stdcall (int&, int)`.  
如果插入的是`void`, 则保持不变
*/
template<typename T1, typename T2>
struct PrependParameter;

template<typename Adding, typename R, typename... Args>
struct PrependParameter<R(Args...), Adding> {
	typedef R type(Adding, Args...);
};

template<typename Adding, typename R, typename... Args>
struct PrependParameter<R __stdcall(Args...), Adding> {
	typedef R __stdcall type(Adding, Args...);
};

template<typename R, typename... Args>
struct PrependParameter<R(Args...), void> {
	typedef R type(Args...);
};

template<typename R, typename... Args>
struct PrependParameter<R __stdcall(Args...), void> {
	typedef R __stdcall type(Args...);
};

/*!
\brief 将所有参数都变为对应的引用类型

比如对于`void(char, int)`, 则会生成`void(char&, int&)`
*/
template<typename T>
struct AllParamsToRef;

template<typename R, typename... Args>
struct AllParamsToRef<R(Args...)> {
	typedef R type(typename std::add_lvalue_reference<Args>::type...);
};

template<typename R, typename... Args>
struct AllParamsToRef<R __stdcall(Args...)> {
	typedef R __stdcall type(typename std::add_lvalue_reference<Args>::type...);
};

/*!
\brief 设置函数签名中返回值的类型

比如对于`<int(char), void>`, 则会生成`void(char)`
*/
template<typename T1, typename T2>
struct SetResultType;

template<typename Setting, typename R, typename... Args> 
struct SetResultType<R(Args...), Setting> {
	typedef Setting type(Args...);
};

template<typename Setting, typename R, typename... Args>
struct SetResultType<R __stdcall(Args...), Setting> {
	typedef Setting __stdcall type(Args...);
};

/*!
\brief 将__stdcall的函数签名变为默认调用约定

比如对于`<int __stdcall (int)>`, 则会生成`int(int)`
*/
template<typename T>
struct RemoveStdcall {
	typedef T type;
};

template<typename R, typename... Args>
struct RemoveStdcall<R __stdcall(Args...)> {
	typedef R type(Args...);
};

/*!
\brief 获取传入的函数签名的返回值类型

比如对于`int(int)`, 则会生成`int`
*/
template<typename T>
struct ResultType;

template<typename R, typename... Args>
struct ResultType<R(Args...)> {
	typedef R type;
};

template<typename R, typename... Args>
struct ResultType<R __stdcall(Args...)> {
	typedef R type;
};

/*!
\brief 将函数签名转换为函数指针

对于普通函数的签名, 用 std::add_pointer 就可以转为函数指针, 
但是这个类模板同时还能保证成员函数指针不会被转换成双重指针.
*/
template<typename T, typename Enable = void> struct FunctionPointer;

template<typename T>
struct FunctionPointer<T, typename std::enable_if<std::is_function<T>::value>::type> {
	typedef typename std::add_pointer<T>::type type;
};

template<typename T>
struct FunctionPointer<T, 
	typename std::enable_if<std::is_member_function_pointer<T>::value>::type> {
	typedef T type;
};

}	// namespace tracer