#pragma once

namespace tracer {

/*!
\brief 保存指向跳板函数的指针.

模板参数`T`应该有一个名为`Signature`的typedef, 它可以是普通函数的签名或者是成员函数指针, 
`RealFuncKeeper<>`的公开静态数据成员`real`是此类型的函数指针.
*/
template<typename T>
struct RealFuncKeeper {
	template<typename S>
	static S *Pointer_(typename ::std::enable_if<
		!::std::is_member_function_pointer<S>::value>::type*);
	template<typename S>
	static S Pointer_(typename ::std::enable_if<
		::std::is_member_function_pointer<S>::value>::type*);
	typedef decltype(Pointer_<typename T::Signature>(0)) Pointer;
	static Pointer real;
};
template<typename T>
typename RealFuncKeeper<T>::Pointer RealFuncKeeper<T>::real;

}	// namespace tracer