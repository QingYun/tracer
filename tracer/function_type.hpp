#pragma once
#include <boost/preprocessor.hpp>

#define TRACER_FUNCTION_TYPE_LIMIT 15
namespace tracer {

/*!
\brief 根据成员函数指针生成对应的普通函数签名.

比如对于`HRESULT (__stdcall C::*)(int, double)`, 
则会生成`HRESULT __stdcall type(C*, int, double)`.
*/
template<typename T>
struct MemberToNormal;

/*!
\brief 在参数列表头部插入一个参数

比如对于`<void __stdcall (int), int&>`, 则会生成`void __stdcall (int&, int)`.  
如果插入的是`void`, 则保持不变
*/
template<typename T1, typename T2>
struct PrependParameter;

/*!
\brief 将所有参数都变为对应的引用类型

比如对于`void(char, int)`, 则会生成`void(char&, int&)`
*/
template<typename T>
struct AllParamsToRef;

/*!
\brief 设置函数签名中返回值的类型

比如对于`<int(char), void>`, 则会生成`void(char)`
*/
template<typename T1, typename T2>
struct SetResultType;

/*!
\brief 将__stdcall的函数签名变为默认调用约定

比如对于`<int __stdcall (int)>`, 则会生成`int(int)`
*/
template<typename T>
struct RemoveStdcall;

/*!
\brief 获取传入的函数签名的返回值类型

比如对于`int(int)`, 则会生成`int`
*/
template<typename T>
struct ResultType;

/*!
\brief 折叠参数

如果参数数量大于7个, 则会将第6个以后的参数并入一个`tuple`作为第7个参数.  
比如对于`int(int, int, int, int, int, char, short, int, long, long long)`, 则会生成
`int(int, int, int, int, int, char, std::tuple<short, int, long, long long>)`
*/
template<typename T>
struct FoldParameters;

/*!
\brief 转发为折叠参数的调用

接收一个见过参数折叠的可调用对象`func`和需要传给它的参数:

- 如果`func`需要的参数少于7个, 则不做任何修改, 直接将参数转发给它.
- 如果`func`需要的参数多于7个(多出来的部分并为一个`tuple`作为第7个参数), 则将前6个参数直接转发
给`func`, 剩下的参数并为`tuple`传给`func`

\sa FoldParameters
*/
void ForwardToFoldedParameters();

#define BOOST_PP_ITERATION_LIMITS (0, TRACER_FUNCTION_TYPE_LIMIT)
#define BOOST_PP_FILENAME_1 "function_type.cpp"
#include BOOST_PP_ITERATE()

}	// namespace tracer