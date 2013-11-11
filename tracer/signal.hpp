#pragma once
#include <boost/signals2.hpp>

namespace tracer {

template<typename T>
struct DiscardParams;

typedef boost::signals2::connection Connection;

template<typename... Args> class Signal;

//! 扩展了boost.signals2
template <typename R, typename... Args, typename... Other>
class Signal<R(Args...), Other...> : public boost::signals2::signal<R(Args...), Other...> {
public:
	typedef boost::signals2::signal<R(Args...), Other...> signal_t;

	template<typename... Args>
	Signal(Args&&... args) :
		signal_t(std::forward<Args>(args)...)
	{}

	//! 插入一个只会被调用一次的插槽
	boost::signals2::connection once(const typename signal_t::slot_type& in_slot,
		typename boost::signals2::connect_position at = boost::signals2::at_back)
	{
		return connect_extended([in_slot](const Connection &conn, Args... args) {
			conn.disconnect();
			return in_slot(args...);
		}, at);
	}
	boost::signals2::connection once(const typename signal_t::group_type& group,
		const typename signal_t::slot_type& in_slot,
		typename boost::signals2::connect_position at = boost::signals2::at_back)
	{
		return connect_extended(group, [in_slot](const Connection &conn, Args... args) {
			conn.disconnect();
			return in_slot(args...);
		}, at);
	}

	//! 插入一个不接收任何参数的插槽
	Connection connect_without_params(boost::function<void(void)> &&cb) {
		return connect(DiscardParams(std::forward<boost::function<void(void)>>(cb)));
	}

private:
	class DiscardParams {
		boost::function<void(void)> cb_;
	public:
		DiscardParams(const boost::function<void(void)> &cb) :
			cb_(cb)
		{}
		DiscardParams(boost::function<void(void)> &&cb) :
			cb_(std::move(cb))
		{}
		R operator() (Args&&...) {
			cb_();
			return R();
		}
	};
};

}	// namespace tracer