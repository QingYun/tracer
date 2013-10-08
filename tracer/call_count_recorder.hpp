#pragma once

namespace tracer {
	
//! 记录函数调用次数的记录器
template<typename T>
class CallCountRecorder {
	Connection conn_;
	std::size_t count_;

public:
	CallCountRecorder(T &tracer) :
		count_(0) {
		conn_ = tracer.Before().connect_without_params([this] () mutable {
			count_++;
		});
	}
	~CallCountRecorder() { conn_.disconnect(); }

	bool HasBeenCalled() const { return !!count_; }
	std::size_t CallCount() const { return count_; }
};

template<typename T>
CallCountRecorder<T> RecordCallCount(T&& tracer) {
	return CallCountRecorder<T>(std::forward<T>(tracer));
}

}	// namespace tracer