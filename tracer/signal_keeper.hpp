#pragma once
#include "signal.hpp"

namespace tracer {

/*!
\brief 持有所有替换函数中用到的 Signal 对象

这个类相当于一个全局的关联容器, 用类型做 key, 取出关联到类型的 Signal 对象.
*/
template<typename T>
struct SignalKeeper {
	static Signal<typename T::Signature> signal;
};
template<typename T>
Signal<typename T::Signature> SignalKeeper<T>::signal;

}	// namespace tracer