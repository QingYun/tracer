#pragma once
#include <string>
#include <vector>
#include <type_traits>
#include "signal.hpp"
#include "dbg_helper.h"

namespace tracer {

//! 记录调用栈
template<typename T>
class CallStackRecorder {
public:
	class CallStackEntry {
		std::string func_name_, file_;
		DWORD line_;
		DWORD64 func_addr_;

	public:
		CallStackEntry(std::string &&func_name, DWORD64 func_addr, 
			std::string &&file, DWORD line) :
			func_name_(std::move(func_name)), 
			func_addr_(func_addr),
			file_(std::move(file)),
			line_(line)
		{}
		DWORD Line() const { return line_; }
		DWORD64 FuncAddr() const { return func_addr_; }
		const std::string &File() const { return file_; }
		const std::string &FuncName() const { return func_name_; }
	};

	class CallStack {
		std::vector<CallStackEntry> entries_;

	public:
		void Push(std::string &&func, DWORD64 func_addr, 
			std::string &&file, DWORD line) {
			entries_.emplace_back(std::move(func), func_addr, std::move(file), line);
		}
		//! 返回整个调用栈, 第一个元素是被记录函数的调用者
		const std::vector<CallStackEntry> &Entries() const {
			return entries_;
		}
		//! 判断被记录函数是否被某个函数直接或间接调用过
		template<typename T>
		auto IsCalledBy(T func) const -> 
			typename std::enable_if<(
				std::is_member_function_pointer<T>::value || 
				std::is_function<typename std::remove_pointer<T>::type>::value), bool>::type{
			DWORD64 addr = SkipJump_(reinterpret_cast<PBYTE&>(func));
			for (auto itr : entries_)
				if (addr == itr.FuncAddr())
					return true;
			return false;
		}
		bool IsCalledBy(const std::string &func) const {
			for (auto itr : entries_)
				if (func == itr.FuncName())
					return true;
			return false;
		}
		
	private:
		DWORD64 SkipJump_(PBYTE addr) const {
			if (addr == NULL)
				return 0;

			// jmp [imm32]
			if (addr[0] == 0xff && addr[1] == 0x25) {
				PBYTE target = *reinterpret_cast<PBYTE*>(&addr[2]);
				if (IsImported_(addr, target))
					addr = *reinterpret_cast<PBYTE*>(&target);
			}

			// jmp imm8
			if (addr[0] == 0xeb) {
				addr = addr + 2 + *reinterpret_cast<PBYTE>(&addr[1]);

				// jmp [imm32]
				if (addr[0] == 0xff && addr[1] == 0x25) {
					PBYTE target = *reinterpret_cast<PBYTE*>(&addr[2]);
					if (IsImported_(addr, target))
						addr = *reinterpret_cast<PBYTE*>(&target);
				} else if (addr[0] == 0xe9) 	// jmp imm32
					addr = addr + 5 + *reinterpret_cast<int*>(&addr[1]);
			} 
			
			// jmp imm32
			if (addr[0] == 0xe9) 	
				addr = addr + 5 + *reinterpret_cast<int*>(&addr[1]);

			return reinterpret_cast<DWORD64>(addr);
		}

		bool IsImported_(PBYTE pbCode, PBYTE pbAddress) const 
		{
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery((PVOID)pbCode, &mbi, sizeof(mbi));
			__try {
				PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)mbi.AllocationBase;
				if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
					return false;
				}

				PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((PBYTE)pDosHeader +
					pDosHeader->e_lfanew);
				if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) {
					return false;
				}

				if (pbAddress >= ((PBYTE)pDosHeader +
					pNtHeader->OptionalHeader
					.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress) &&
					pbAddress < ((PBYTE)pDosHeader +
					pNtHeader->OptionalHeader
					.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress +
					pNtHeader->OptionalHeader
					.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size)) {
						return true;
				}
				return false;
			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				return false;
			}
		}
	};

private:
	std::vector<CallStack> call_stacks_;
	Connection conn_;

public:
	CallStackRecorder(T &tracer) {
		conn_ = tracer.Before().connect_without_params([this] () {
			CONTEXT c;
			c.ContextFlags = CONTEXT_FULL;
			RtlCaptureContext(&c);
			CallStack call_stack;

			auto itr = DbgHelper::Instance().StackWalk(&c);
			for (int i = 0; i < 19; ++i) ++itr;

			DWORD64 displacement;
			for (; itr; ++itr) {
				std::string func_name(
					DbgHelper::Instance().GetSymbolName(itr->AddrPC.Offset, &displacement));
				call_stack.Push(
					std::move(func_name), itr->AddrPC.Offset - displacement,  
					DbgHelper::Instance().GetFileName(itr->AddrPC.Offset), 
					DbgHelper::Instance().GetLine(itr->AddrPC.Offset));
			}
			call_stacks_.push_back(std::move(call_stack));
		});
	}
	~CallStackRecorder() {
		conn_.disconnect();
	}
	const CallStack & GetCallStack(std::size_t n) const {
		return call_stacks_.at(n);
	}
};

template<typename T>
CallStackRecorder<T> RecordCallStack(T &tracer) {
	return CallStackRecorder<T>(tracer);
}

}	// namespace tracer