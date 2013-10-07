#pragma once
#include <windows.h>
#include <mutex>
#include <vector>
#include <DbgHelp.h>
#include "singleton.hpp"

namespace tracer {

class StackWalkIterator;

//! 封装了DbgHelpe.h的一些函数
class DbgHelper : public Singleton<DbgHelper> {
	std::mutex lock_;
	PSYMBOL_INFO sym_buffer_;
	IMAGEHLP_LINE64 line_info_;

private:
	//! 拼接符号文件搜索地址
	std::string GetSymSearchPath_();
	//! 加载模块信息
	void LoadModules_();
	//! 初始化调试符号
	void Init_();
	//! 清理调试符号资源
	bool Cleanup_();
	//! 申请指定大小的符号结构
	void AllocSymBuffer_(std::size_t size);
	DbgHelper();
	~DbgHelper();
	DbgHelper(const DbgHelper&);
	const DbgHelper& operator = (const DbgHelper&);
	friend class Singleton<DbgHelper>;
	friend class StackWalkIterator;

public:
	//! 遍历调用栈
	StackWalkIterator StackWalk(CONTEXT *context = nullptr);
	//! 根据地址获取符号名
	std::string GetSymbolName(DWORD64 addr);
	//! 去除符号名粉碎
	std::string UnDecorateSymbolName(const std::string symbol_name, DWORD flag = UNDNAME_COMPLETE);
	//! 根据地址获取符号的文件名
	std::string GetFileName(DWORD64 addr);
	//! 根据地址获取符号的行号
	DWORD GetLine(DWORD64 addr);
};

//! 遍历调用栈的迭代器
class StackWalkIterator {
	STACKFRAME64 sf_;
	CONTEXT *context_;
	std::mutex &lock_;

	typedef void (StackWalkIterator::*Bool_)() const;
	void CannotCompare_() const {}

public:
	StackWalkIterator(CONTEXT *context, std::mutex &lock);
	StackWalkIterator &operator ++ ();
	StackWalkIterator operator ++ (int);
	const STACKFRAME64 &operator * () const;
	const STACKFRAME64 *operator -> () const;
	operator Bool_() const;
};

}	// namespace tracer