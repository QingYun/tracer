/*!
\file 
包含了生成Hook封装类的宏
*/
#pragma once
#include <boost/preprocessor.hpp>
#include <type_traits>
#include "hook_manager.h"
#include "function_type.hpp"
#include "signal.hpp"

#define TRACER_HOOK_NORMAL 0
#define TRACER_HOOK_COM 1
#ifndef TRACER_HOOK_ARG_LIMIT
#define TRACER_HOOK_ARG_LIMIT 15
#endif

#define TRACER_HOOK_FAKE_FUNC_PARAMS(z, n, _)									\
	BOOST_PP_CAT(P, n) BOOST_PP_CAT(p, n)

#define TRACER_HOOK_FULL_NAME(scope, var)										\
	BOOST_PP_CAT(scope, BOOST_PP_CAT(::, var))

//! `TRACER_HOOK_FAKE_FUNCS`的辅助宏, 用来生成有返回值和无返回值的重载
#define TRACER_HOOK_FAKE_FUNCS__(n, cc)											\
	template<typename R															\
		BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>				\
	static auto fake(R cc (BOOST_PP_ENUM_PARAMS(n, P)))	->						\
	typename ::std::enable_if<!::std::is_void<R>::value,						\
		R (cc *)(BOOST_PP_ENUM_PARAMS(n, P))>::type {							\
		return [] (BOOST_PP_ENUM(n, TRACER_HOOK_FAKE_FUNC_PARAMS, _)) {			\
			bool call_ori = true;												\
			R ret;																\
			::tracer::ForwardToFoldedParameters(::std::ref(before_signal),		\
				call_ori BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));		\
			if (call_ori)														\
				ret = real(BOOST_PP_ENUM_PARAMS(n, p));							\
			::tracer::ForwardToFoldedParameters(::std::ref(after_signal),		\
				call_ori, ret BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));	\
			return ret;															\
		};																		\
	}																			\
	template<typename R															\
		BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename P)>				\
	static auto fake(R cc (BOOST_PP_ENUM_PARAMS(n, P)))	->						\
	typename ::std::enable_if<::std::is_void<R>::value,							\
	R (cc *)(BOOST_PP_ENUM_PARAMS(n, P))>::type {								\
		return [] (BOOST_PP_ENUM(n, TRACER_HOOK_FAKE_FUNC_PARAMS, _)) {			\
			bool call_ori = true;												\
			::tracer::ForwardToFoldedParameters(::std::ref(before_signal),		\
				call_ori BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));		\
			if (call_ori)														\
				real(BOOST_PP_ENUM_PARAMS(n, p));								\
			::tracer::ForwardToFoldedParameters(::std::ref(after_signal),		\
				call_ori BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, p));		\
		};																		\
	}

//! `TRACER_HOOK_FAKE_FUNCS`的辅助宏, 用来生成默认调用约定和`__stdcall`的重载
#define TRACER_HOOK_FAKE_FUNCS_(z, n, _)										\
	TRACER_HOOK_FAKE_FUNCS__(n, BOOST_PP_EMPTY())								\
	TRACER_HOOK_FAKE_FUNCS__(n, __stdcall)											

/*!
\brief 生成fake函数

因为在宏中不知道原始函数的签名, 所以没法直接生成和其签名一致的fake函数, 
这个宏生成的实际上是一大堆重载. 所有重载都接收的都是一个参数数量不同的函数指针, 
每个参数数量会有四份重载, 分别对应 默认调用约定 和`__stdcall`以及返回`void`的情况.
每个重载都会返回一个和其签名一致的lambda转换出来的的函数指针.
*/
#define TRACER_HOOK_FAKE_FUNCS													\
	BOOST_PP_REPEAT(TRACER_HOOK_ARG_LIMIT, TRACER_HOOK_FAKE_FUNCS_, _)

//! 生成Before signal相关的类型以及signal对象声明
#define TRACER_HOOK_BEFORE_SIGNAL_DECL											\
	typedef																		\
		::tracer::FoldParameters<												\
			::tracer::AllParamsToRef<											\
				::tracer::PrependParameter<										\
					::tracer::SetResultType<Signature, void>::type,				\
					bool														\
				>::type															\
			>::type																\
		>::type BeforeCallbackSignature;										\
	typedef ::std::function<BeforeCallbackSignature> BeforeCallback;			\
	typedef ::tracer::Signal<BeforeCallbackSignature> BeforeSignal;				\
	static BeforeSignal before_signal;

//! 生成After signal相关的类型以及signal对象声明
#define TRACER_HOOK_AFTER_SIGNAL_DECL											\
	typedef																		\
		::tracer::FoldParameters<												\
			::tracer::PrependParameter<											\
				::tracer::AllParamsToRef<										\
					::tracer::PrependParameter<									\
						::tracer::SetResultType<Signature, void>::type,			\
						::tracer::ResultType<Signature>::type					\
					>::type														\
				>::type,														\
				bool															\
			>::type																\
		>::type AfterCallbackSignature;											\
	typedef ::std::function<AfterCallbackSignature> AfterCallback;				\
	typedef ::tracer::Signal<AfterCallbackSignature> AfterSignal;				\
	static AfterSignal after_signal;

//! 生成信息类数据成员定义
#define TRACER_HOOK_INFO_CLASS_DEF(_, class_name, func_name)					\
	class_name::BOOST_PP_CAT(func_name, Info)::OriSignature *					\
		class_name::BOOST_PP_CAT(func_name, Info)::real = nullptr;				\
	class_name::BOOST_PP_CAT(func_name, Info)::BeforeSignal						\
		class_name::BOOST_PP_CAT(func_name, Info)::before_signal;				\
	class_name::BOOST_PP_CAT(func_name, Info)::AfterSignal						\
		class_name::BOOST_PP_CAT(func_name, Info)::after_signal;

//! 生成Unhook实现
#define TRACER_HOOK_UNHOOK_FUNC(z, _, name)										\
	static LONG name() {														\
	return ::tracer::HookManager::Instance().Remove(							\
	&BOOST_PP_CAT(name, Info)::real);											\
	}

//! 生成一个After回调类
#define TRACER_HOOK_AFTER_CLASS(z, _, name)										\
	struct name {																\
		static ::tracer::Connection Call(										\
		BOOST_PP_CAT(name, Info)::AfterCallback cb) {							\
			return BOOST_PP_CAT(name, Info)::after_signal.connect(cb);			\
		}																		\
		static ::tracer::Connection CallOnce(									\
		BOOST_PP_CAT(name, Info)::AfterCallback cb) {							\
			return BOOST_PP_CAT(name, Info)::after_signal.once(cb);				\
		}																		\
	};

//! 生成一个Before回调类
#define TRACER_HOOK_BEFORE_CLASS(z, _, name)									\
	struct name {																\
		static ::tracer::Connection Call(										\
		BOOST_PP_CAT(name, Info)::BeforeCallback cb) {							\
			return BOOST_PP_CAT(name, Info)::before_signal.connect(cb);			\
		}																		\
		static ::tracer::Connection CallOnce(									\
		BOOST_PP_CAT(name, Info)::BeforeCallback cb) {							\
			return BOOST_PP_CAT(name, Info)::before_signal.once(cb);			\
		}																		\
	};

/*!
\brief 生成用于Hook普通函数的信息类

这个类里包括了Hook这个函数的所有数据, 全部都是静态的, 包括:

- 指向跳板函数的函数指针`real`
- 用来替换原始函数的成员函数`fake`. 实际上不只是成员函数这么简单, 为了不依赖于未知的
函数签名(相对于宏来说), 做了一些hack.
- 储存在原始函数调用前触发的回调的signal对象`before_signal`
- 储存在原始函数调用后触发的回调的signal对象`after_signal`
*/
#define TRACER_HOOK_NORMAL_INFO_CLASS_DECL(_, scope, name)						\
	struct BOOST_PP_CAT(name, Info) {											\
		typedef decltype(TRACER_HOOK_FULL_NAME(scope, name)) OriSignature;		\
		typedef ::tracer::RemoveStdcall<OriSignature>::type Signature;			\
		static OriSignature *real;												\
		TRACER_HOOK_FAKE_FUNCS													\
		TRACER_HOOK_BEFORE_SIGNAL_DECL											\
		TRACER_HOOK_AFTER_SIGNAL_DECL											\
	};

//! 生成针对普通函数的Hook实现
#define TRACER_HOOK_NORMAL_HOOK_FUNC(_, scope, name)							\
	static LONG name() {														\
		if (!BOOST_PP_CAT(name, Info)::real)									\
			BOOST_PP_CAT(name, Info)::real = TRACER_HOOK_FULL_NAME(scope, name);\
		return ::tracer::HookManager::Instance().Install(						\
			&BOOST_PP_CAT(name, Info)::real,									\
			BOOST_PP_CAT(name, Info)::fake(										\
				reinterpret_cast<												\
					BOOST_PP_CAT(name, Info)::OriSignature*>(NULL)));			\
}

/*!
\brief 生成Hook普通函数的封装类的声明
\param [in] class_name 要生成的类的名称
\param [in] scope 函数所在的命名空间, 比如`foo::bar`.
\param [in] func_list 每个元素应为一个函数名
*/
#define TRACER_HOOK_NORMAL_DECL(class_name, scope, func_list)					\
	class class_name {															\
		BOOST_PP_SEQ_FOR_EACH(													\
			TRACER_HOOK_NORMAL_INFO_CLASS_DECL, scope, func_list)				\
	public:																		\
		struct Hook {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_NORMAL_HOOK_FUNC, scope, func_list)					\
		};																		\
		struct Unhook {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_UNHOOK_FUNC, _, func_list)							\
		};																		\
		struct Before {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_BEFORE_CLASS, _, func_list)							\
		};																		\
		struct After {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_AFTER_CLASS, _, func_list)							\
		};																		\
	};																			

//! 生成Hook普通函数的封装类的定义
#define TRACER_HOOK_NORMAL_DEF(class_name, func_list)							\
	BOOST_PP_SEQ_FOR_EACH(TRACER_HOOK_INFO_CLASS_DEF, class_name, func_list)

/*!
\brief 生成Hook COM函数的信息类的定义

和普通函数的信息类相比, COM函数的多了一个`index`成员, 表示这个函数在虚表中的序号
*/
#define TRACER_HOOK_COM_INFO_CLASS_DECL(_, scope, elem)							\
	struct BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2, 0, elem), Info) {				\
		typedef ::tracer::COMToNormal<decltype(&TRACER_HOOK_FULL_NAME(scope,	\
			BOOST_PP_TUPLE_ELEM(2, 0, elem)))>::type OriSignature;				\
		typedef ::tracer::RemoveStdcall<OriSignature>::type Signature;			\
		static OriSignature *real;												\
		static const ::std::size_t index = BOOST_PP_TUPLE_ELEM(2, 1, elem);		\
		TRACER_HOOK_FAKE_FUNCS													\
		TRACER_HOOK_BEFORE_SIGNAL_DECL											\
		TRACER_HOOK_AFTER_SIGNAL_DECL											\
	};

#define TRACER_HOOK_COM_HOOK_FUNC_(name)										\
	static LONG name() {														\
		if (!BOOST_PP_CAT(name, Info)::real)									\
			BOOST_PP_CAT(name, Info)::real = reinterpret_cast<decltype(			\
				BOOST_PP_CAT(name, Info)::real)>(								\
					vmt()[BOOST_PP_CAT(name, Info)::index]);					\
		return ::tracer::HookManager::Instance().Install(						\
			&BOOST_PP_CAT(name, Info)::real,									\
			BOOST_PP_CAT(name, Info)::fake(										\
				reinterpret_cast<												\
					BOOST_PP_CAT(name, Info)::OriSignature*>(NULL)));			\
	}
//! 生成针对COM函数的Hook实现
#define TRACER_HOOK_COM_HOOK_FUNC(z, _, elem)									\
	TRACER_HOOK_COM_HOOK_FUNC_(BOOST_PP_TUPLE_ELEM(2, 0, elem))

#define TRACER_HOOK_COM_UNHOOK_FUNC(z, _, elem)									\
	TRACER_HOOK_UNHOOK_FUNC(z, _, BOOST_PP_TUPLE_ELEM(2, 0, elem))

#define TRACER_HOOK_COM_BEFORE_CLASS(z, _, elem)								\
	TRACER_HOOK_BEFORE_CLASS(z, _, BOOST_PP_TUPLE_ELEM(2, 0, elem))

#define TRACER_HOOK_COM_AFTER_CLASS(z, _, elem)									\
	TRACER_HOOK_AFTER_CLASS(z, _, BOOST_PP_TUPLE_ELEM(2, 0, elem))

/*!
\brief 生成Hook COM函数的封装类声明

Hook COM函数的封装类会有一个额外的静态方法`vmt()`, 返回接口的虚表指针.
\param [in] class_name 要生成的类的名称
\param [in] scope 应该精确到类, 比如`::IDirect3D9`. 
\param [in] func_list 其中每个元素应该是一个有两个元素的tuple, 其中第一个元素是函数名, 
第二个元素是其在虚表中的序号, 比如`((func1, 0))((func2, 1))((func3, 2))`
*/
#define TRACER_HOOK_COM_DECL(class_name, scope, func_list)						\
	class class_name {															\
		static void **vmt_;														\
		static scope *GetInterfacePointer_();									\
		BOOST_PP_SEQ_FOR_EACH(													\
			TRACER_HOOK_COM_INFO_CLASS_DECL, scope, func_list)					\
	public:																		\
		static void **vmt() {													\
			if (!vmt_)															\
				vmt_ = *reinterpret_cast<void***>(GetInterfacePointer_());		\
			return vmt_;														\
		}																		\
		struct Hook {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_COM_HOOK_FUNC, _, func_list)						\
		};																		\
		struct Unhook {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_COM_UNHOOK_FUNC, _, func_list)						\
		};																		\
		struct Before {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_COM_BEFORE_CLASS, _, func_list)						\
		};																		\
		struct After {															\
			BOOST_PP_SEQ_FOR_EACH(												\
				TRACER_HOOK_COM_AFTER_CLASS, _, func_list)						\
		};																		\
	};																			

#define TRACER_HOOK_COM_INFO_CLASS_DEF(_, class_name, elem)						\
	TRACER_HOOK_INFO_CLASS_DEF(_, class_name, BOOST_PP_TUPLE_ELEM(2, 0, elem))

/*!
\brief 生成Hook COM函数的封装类的定义

这个宏后面应该跟一个函数体, 函数体中应返回要用到的接口指针, 比如

	TRACER_HOOK_COM_DEF(IDirect3D9Hook, ((GetDeviceCaps, 14))) { 
		return Direct3DCreate9(D3D_SDK_VERSION);
	}
*/
#define TRACER_HOOK_COM_DEF(class_name, func_list)								\
	BOOST_PP_SEQ_FOR_EACH(TRACER_HOOK_COM_INFO_CLASS_DEF, class_name, func_list)\
	void **class_name::vmt_ = nullptr;											\
	::std::result_of<decltype(&class_name::GetInterfacePointer_)()>::type		\
	class_name::GetInterfacePointer_()

//! 遇到未知的category时会展开这个宏
#define TRACER_HOOK_ERROR_DECL(a, b, c) static_assert(false, "未知Hook类型! 请检查TRACER_HOOK_CLASS_DECL的第二个参数");

/*!
\brief 生成Hook封装类声明

对于被Hook的函数的不同种类, 生成的类之间略有差别, 但是最少有四个公开的嵌套类:

- Hook: 对于每个被Hook的函数, 这个类中都有一个同名的静态函数, 调用它即可挂上钩子
- Unhook: 对于每个被Hook的函数, 这个类中都有一个同名的静态函数, 调用它即可卸载钩子
- Before: 对于每个被Hook的函数, 这个类中都有一个公开的同名嵌套类, 类中有两个静态函数
	- Call: 接收一个回调, 回调将会在原始函数调用之前被调用; 返回一个
	`tracer::Connection`对象, 用来管理回调的连接情况. 
	- CallOnce: 和`Call`类似, 但是回调被调用一次后就会自动断开连接了. 
- After: 对于每个被Hook的函数, 这个类中都有一个公开的同名嵌套类, 类中有两个静态函数
	- Call: 接收一个回调, 回调将会在原始函数调用之后被调用; 返回一个
	`tracer::Connection`对象, 用来管理回调的连接情况. 
	- CallOnce: 和`Call`类似, 但是回调被调用一次后就会自动断开连接了. 

\param [in] class_name 要生成的封装类的名称
\param [in] category 要Hook的函数类型:

- `TRACER_HOOK_NORMAL`表示要Hook的都是普通函数, 使用 TRACER_HOOK_NORMAL_DECL 生成声明
- `TRACER_HOOK_COM`表示要Hook的都是COM函数, 使用 TRACER_HOOK_COM_DECL 生成声明

\param [in] scope 函数所在作用域, 具体要求视种类而定
\param [in] func_list 要Hook的函数列表, 是一个SEQ结构(`(a)(b)(c)`), 元素内容视种类而定
\sa TRACER_HOOK_NORMAL_DECL, TRACER_HOOK_COM_DECL
*/
#define TRACER_HOOK_CLASS_DECL(class_name, category, scope, func_list)			\
	BOOST_PP_IIF(BOOST_PP_EQUAL(category, TRACER_HOOK_NORMAL),					\
		TRACER_HOOK_NORMAL_DECL,												\
		BOOST_PP_IIF(BOOST_PP_EQUAL(category, TRACER_HOOK_COM),					\
			TRACER_HOOK_COM_DECL,												\
			TRACER_HOOK_ERROR_DECL))(class_name, scope, func_list)

//! 遇到未知的category时会展开这个宏
#define TRACER_HOOK_ERROR_DEF(a, b) static_assert(false, "未知Hook类型! 请检查TRACER_HOOK_CLASS_DECL的第二个参数");

/*!
\brief 生成Hook封装类定义
\param [in] class_name 封装类的名称
\param [in] category Hook函数的种类:

- `TRACER_HOOK_NORMAL`表示要Hook的都是普通函数, 使用 TRACER_HOOK_NORMAL_DEF 生成定义
- `TRACER_HOOK_COM`表示要Hook的都是COM函数, 使用 TRACER_HOOK_COM_DEF 生成定义

\param [in] func_list 传给`TRACER_HOOK_CLASS_DECL`的函数列表
\sa TRACER_HOOK_NORMAL_DEF, TRACER_HOOK_COM_DEF
*/
#define TRACER_HOOK_CLASS_DEF(class_name, category, func_list)					\
	BOOST_PP_IIF(BOOST_PP_EQUAL(category, TRACER_HOOK_NORMAL),					\
		TRACER_HOOK_NORMAL_DEF,													\
		BOOST_PP_IIF(BOOST_PP_EQUAL(category, TRACER_HOOK_COM),					\
			TRACER_HOOK_COM_DEF,												\
			TRACER_HOOK_ERROR_DEF))(class_name, func_list)