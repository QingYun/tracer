#include "hook_manager.h"

namespace tracer {

void HookManager::Clear() {
	for (auto itr : hooks_) 
		RemoveImpl(itr.first, itr.second);
	hooks_.clear();
}	

LONG HookManager::Install_( void **target, void *detour ) {
	std::lock_guard<std::mutex> l(lock_);

	auto itr = hooks_.find(target);
	if (itr != hooks_.end())
		return ERROR_INVALID_HANDLE;

	LONG ret = InstallImpl(target, detour);
	if (NO_ERROR == ret)
		hooks_[target] = detour;
	return ret;
}

LONG HookManager::Remove_( void **target ) {
	std::lock_guard<std::mutex> l(lock_);

	auto itr = hooks_.find(target);
	if (itr == hooks_.end())
		return ERROR_INVALID_HANDLE;

	LONG ret = RemoveImpl(target, itr->second);
	if (NO_ERROR == ret)
		hooks_.erase(itr);
	return ret;
}

}	// namespace tracer