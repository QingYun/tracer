#include "dbg_helper.h"
#include <memory>
#include <sstream>
#include <TlHelp32.h>
#pragma comment(lib, "DbgHelp.lib")

namespace {

std::string GetLastErrMsg() {
	DWORD err = GetLastError();
	LPVOID msg_buf;
	std::string ret;
	std::ostringstream ss;
	ss << err;
	ret.append("[").append(ss.str()).append("] ");
	if (0 == FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		reinterpret_cast<char*>(&msg_buf), 0, NULL))
		return ret;
	ret.append(reinterpret_cast<char*>(msg_buf));
	LocalFree(msg_buf);
	return ret;
}

}

namespace tracer {

std::string DbgHelper::GetSymSearchPath_() {
	std::string path(".;");
	const std::size_t buf_size = 4096;
	char buf[buf_size];

	if (GetCurrentDirectoryA(buf_size, buf) > 0) 
		path.append(buf).append(";");

	if (GetModuleFileNameA(NULL, buf, buf_size) > 0) {
		for (char *p = (buf + strlen(buf) - 1); p >= buf; --p) {
			if ((*p == '\\') || (*p == '/') || (*p == ';')) {
				*p = '\0';
				break;
			}
		}
		if (strlen(buf) > 0)
			path.append(buf).append(";");
	}

	if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", buf, buf_size) > 0)
		path.append(buf).append(";");

	if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", buf, buf_size) > 0)
		path.append(buf).append(";");

	if (GetEnvironmentVariableA("SYSTEMROOT", buf, buf_size) > 0)
		path.append(buf).append(";").append(buf).append("\\system32").append(";");

	return path;	
}

void DbgHelper::LoadModules_() {
	MODULEENTRY32 me = {sizeof(me)};
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

	if (INVALID_HANDLE_VALUE == snap)
		throw std::runtime_error("CreateToolhelp32Snapshot fail " + GetLastErrMsg());

	bool keep_going = !!Module32First(snap, &me);
	while (keep_going) {
		if ((0 == SymLoadModuleEx(GetCurrentProcess(), NULL, me.szExePath, me.szModule, 
				reinterpret_cast<DWORD64>(me.modBaseAddr), me.modBaseSize, NULL, 0)) 
			&& (ERROR_SUCCESS != GetLastError()))
			throw std::runtime_error("SymLoadModuleEx fail " + GetLastErrMsg());
		keep_going = !!Module32Next(snap, &me);
	}
	CloseHandle(snap);
}

void DbgHelper::Init_() {
	std::string path = GetSymSearchPath_();
	std::lock_guard<std::mutex> l(lock_);

	if (FALSE == SymInitialize(GetCurrentProcess(), path.c_str(), FALSE))
		throw std::runtime_error("SymInitialize fail " + GetLastErrMsg());

	DWORD options = SymGetOptions();
	options |= SYMOPT_LOAD_LINES;
	options |= SYMOPT_FAIL_CRITICAL_ERRORS;
	SymSetOptions(options);

	LoadModules_();
}

bool DbgHelper::Cleanup_() {
	std::lock_guard<std::mutex> l(lock_);
	return !!SymCleanup(GetCurrentProcess());
}

void DbgHelper::AllocSymBuffer_( std::size_t size ) {
	if (sym_buffer_)
		delete sym_buffer_;
	sym_buffer_ = static_cast<PSYMBOL_INFO>(malloc(sizeof(SYMBOL_INFO) + size));
	sym_buffer_->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym_buffer_->MaxNameLen = size;
}

DbgHelper::DbgHelper() {
	Init_();
	sym_buffer_ = nullptr;
	AllocSymBuffer_(1024);

	ZeroMemory(&line_info_, sizeof(line_info_));
	line_info_.SizeOfStruct = sizeof(line_info_);
}

DbgHelper::~DbgHelper() {
	Cleanup_();
}

/*!
传入有效的`context`时, 相当于`begin()`; 不写参数或者传`nullptr`时相当于`end()`
\param [in] context 要遍历调用栈的线程上下文
\return 遍历调用栈的迭代器
*/
tracer::StackWalkIterator DbgHelper::StackWalk( CONTEXT *context /*= nullptr*/ ) {
	return StackWalkIterator(context, lock_);
}

/*!
\param [in] addr 要查找的符号的地址
\param [out] displacement 传入的地址与符号起始地址间的距离
\return 符号名称
*/
std::string DbgHelper::GetSymbolName( DWORD64 addr, DWORD64 *displacement /*= nullptr*/ ) {
	std::lock_guard<std::mutex> l(lock_);
	while (true) {
		if (!SymFromAddr(GetCurrentProcess(), addr, displacement, sym_buffer_))
			throw std::runtime_error("SymFromAddr failed " + GetLastErrMsg());
		if (sym_buffer_->NameLen < sym_buffer_->MaxNameLen)
			break;
		AllocSymBuffer_(sym_buffer_->MaxNameLen * 2);
	}
	return sym_buffer_->Name;
}

std::string DbgHelper::UnDecorateSymbolName( const std::string symbol_name, DWORD flag /*= UNDNAME_COMPLETE*/ ) {
	std::lock_guard<std::mutex> l(lock_);
	std::size_t size = 1024;
	std::unique_ptr<char> und_name(new char[size]);
	DWORD need_size;
	while (true) {
		need_size = ::UnDecorateSymbolName(symbol_name.c_str(), und_name.get(), size, flag);
		if (need_size == 0)
			throw std::runtime_error("UnDecorateSymbolName failed " + GetLastErrMsg());
		else if (need_size >= size) {
			size = need_size + 1;
			und_name.reset(new char[size]);
		} else 
			break;
	}
	std::string ret(und_name.get());
	return ret;
}

std::string DbgHelper::GetFileName( DWORD64 addr ) {
	std::lock_guard<std::mutex> l(lock_);
	DWORD displacement;
	if (!SymGetLineFromAddr64(GetCurrentProcess(), addr, &displacement, &line_info_))
		return "<unknown>";
	return line_info_.FileName;
}

DWORD DbgHelper::GetLine( DWORD64 addr ) {
	std::lock_guard<std::mutex> l(lock_);
	DWORD displacement;
	if (!SymGetLineFromAddr64(GetCurrentProcess(), addr, &displacement, &line_info_))
		return 0;
	return line_info_.LineNumber;
}

StackWalkIterator::StackWalkIterator( CONTEXT *context, std::mutex &lock ) :
	context_(context),
	lock_(lock) {
	ZeroMemory(&sf_, sizeof(sf_));
	if (context_) {
		std::lock_guard<std::mutex> l(lock_);
		sf_.AddrPC.Offset = context_->Eip;
		sf_.AddrPC.Mode = AddrModeFlat;
		sf_.AddrFrame.Offset = context_->Ebp;
		sf_.AddrFrame.Mode = AddrModeFlat;
		sf_.AddrStack.Offset = context_->Esp;
		sf_.AddrStack.Mode = AddrModeFlat;
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), 
			&sf_, context_, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
			throw std::runtime_error("StackWalk64 failed " + GetLastErrMsg());
	}
}

StackWalkIterator & StackWalkIterator::operator++() {
	if (context_) {
		std::lock_guard<std::mutex> l(lock_);
		if (!StackWalk64(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), GetCurrentThread(), 
			&sf_, context_, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL)) 
			context_ = nullptr;
	}
	return *this;
}

tracer::StackWalkIterator StackWalkIterator::operator++( int ) {
	StackWalkIterator t = *this;
	++(*this);
	return t;
}

const STACKFRAME64 & StackWalkIterator::operator*() const {
	return sf_;
}

const STACKFRAME64 * StackWalkIterator::operator->() const {
	return &sf_;
}

StackWalkIterator::operator Bool_ () const {
	return context_ ? &StackWalkIterator::CannotCompare_ : 0;
}


}	// namespace tracer