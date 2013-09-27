#pragma once
#include <unordered_map>
#include <mutex>
#include "hook_impl.h"
#include "singleton.hpp"
#include "../google/gtest/gtest_prod.h"

namespace tracer {

/*!
\brief 钩子管理器

这个类是 InstallImpl 和 RemoveImpl 的包装, 会记录所有已安装的钩子并且在析构时移除他们.
*/
class HookManager : public Singleton<HookManager> {
	/*!
	key是指向跳板函数的指针的指针, value是替换函数的指针
	*/
	std::unordered_map<void**, void*> hooks_;
	std::mutex lock_;

private:
	HookManager() {}
	HookManager(const HookManager&);
	HookManager &operator = (const HookManager &);
	~HookManager() { Clear(); }
	friend class Singleton<HookManager>;

	LONG Install_(void **target, void *detour);
	LONG Remove_(void **target);

public:
	/*!
	\brief 安装钩子
	
	参数返回值同 InstallImpl , 唯一的不同是返回`ERROR_INVALID_HANDLE`还有可能是因为
	`target`已经装有钩子了
	*/
	template<typename T>
	LONG Install(T **target, T *detour) {
		return Install_(reinterpret_cast<void**>(target), 
			reinterpret_cast<void*>(detour));
	}

	/*!
	\brief 移除钩子
	\param [in, out] target 传参时应指向跳板函数, 返回后指向原始函数
	\return 同 RemoveImpl , 唯一的不同是返回`ERROR_INVALID_HANDLE`还有可能是因为
	`target`上没有安装钩子
	*/
	template<typename T>
	LONG Remove(T **target) {
		return Remove_(reinterpret_cast<void**>(target));
	}

	//! 移除所有钩子
	void Clear();

	FRIEND_TEST(HookManagerTest, NormalFuncTest);
	FRIEND_TEST(HookManagerTest, DtorTest);
	FRIEND_TEST(HookManagerTest, APITest);
};

}	// namespace tracer
