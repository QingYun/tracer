#pragma once

namespace tracer {
	
//! 记录函数调用次数的记录器
template<typename T>
class CallCountRecorder {
	T &tracer_;
	Connection conn_;
	std::size_t count_;

public:
	CallCountRecorder(T &tracer) :
		tracer_(tracer),
		count_(0) {
		conn_ = tracer_.Before().connect_without_params([this] () mutable {
			count_++;
		});
	}
	bool HasBeenCalled() const { return !!count_; }
	std::size_t CallCount() const { return count_; }
};

template<typename T>
CallCountRecorder<T> RecordCallCount(T&& tracer) {
	return CallCountRecorder<T>(std::forward<T>(tracer));
}

}	// namespace tracer