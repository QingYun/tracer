#pragma once
#include <boost/signals2.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/at.hpp>

#define TRACER_SIGNAL_ENUM_SLOT_FUNC_ARGS(arity)  \
	BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(BOOST_PP_INC(arity)), _)

#define TRACER_SIGNAL_MAKE_AT_TYPE(index) \
	typename boost::mpl::at<param_types, BOOST_PP_CAT(BOOST_PP_CAT(boost::mpl::int_<, index), > >::type)
#define TRACER_SIGNAL_TYPED_PARAMETER(z,INDEX,DATA) \
	TRACER_SIGNAL_MAKE_AT_TYPE(BOOST_PP_DEC(INDEX)) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,1,DATA),INDEX)
#define TRACER_SIGNAL_ENUM_FUNCTOR_ARGS(arity)  \
	BOOST_PP_COMMA_IF(arity) \
	BOOST_PP_ENUM_SHIFTED(BOOST_PP_INC(arity),TRACER_SIGNAL_TYPED_PARAMETER,(T,param))

#define TRACER_SIGNAL_ENUM_CALLBACK_ARGS(arity) \
	BOOST_PP_ENUM_SHIFTED_PARAMS(BOOST_PP_INC(arity), param)

#define TRACER_SIGNAL_ONCE_WRAP_CLASS_NAME(arity) BOOST_PP_CAT(SignalOnceWrap, arity)

namespace tracer {

template<typename T>
struct DiscardParams;

namespace {
#define BOOST_PP_ITERATION_LIMITS (0, BOOST_SIGNALS2_MAX_ARGS)
#define BOOST_PP_FILENAME_1 "signal.cpp"
#include BOOST_PP_ITERATE()
}

typedef boost::signals2::connection Connection;

template <typename Signature,
	typename Combiner = boost::signals2::optional_last_value<typename boost::function_traits<Signature>::result_type>,
	typename Group = int,
	typename GroupCompare = std::less<Group>,
	typename SlotFunction = boost::function<Signature>,
	typename ExtendedSlotFunction = typename boost::signals2::detail::extended_signature<boost::function_traits<Signature>::arity, Signature>::function_type,
	typename Mutex = boost::signals2::mutex >
class Signal : public boost::signals2::signal<Signature, Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex>
{

public:
	typedef typename boost::signals2::signal<Signature, 
		Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, 
		Mutex>::base_type signal_t;
	typedef typename boost::signals2::signal<Signature, 
		Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, 
		Mutex>::extended_slot_type slot_t;
	typedef signal_once_impl<boost::function_traits<Signature>::arity, Signature, 
		Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex> once_class_t;

	Signal(const Combiner &combiner_arg = Combiner(), 
		const GroupCompare &group_compare = GroupCompare()) :
	boost::signals2::signal<Signature, Combiner, Group, GroupCompare, SlotFunction, 
		ExtendedSlotFunction, Mutex>(combiner_arg, group_compare)
	{}

	boost::signals2::connection once(const typename signal_t::slot_type& in_slot, 
		typename boost::signals2::connect_position at = boost::signals2::at_back)
	{
		return once_class_t().operator()<boost::signals2::connection>
			(this, in_slot, at);
	}
	boost::signals2::connection once(const typename signal_t::group_type& group, 
		const typename signal_t::slot_type& in_slot, 
		typename boost::signals2::connect_position at = boost::signals2::at_back)
	{
		return once_class_t().operator()<boost::signals2::connection>
			(this, group, in_slot, at);
	}
	Connection connect_without_params(std::function<void(void)> cb) {
		return connect(DiscardParams<Signature>(cb));
	}
};

}	// namespace tracer