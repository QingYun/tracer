#define TRACER_SIGNAL_NUM_ARGS BOOST_PP_ITERATION()

template<typename Signature>
class TRACER_SIGNAL_ONCE_WRAP_CLASS_NAME(TRACER_SIGNAL_NUM_ARGS)
{
	boost::function<Signature> real_proc_;
public:
	TRACER_SIGNAL_ONCE_WRAP_CLASS_NAME(TRACER_SIGNAL_NUM_ARGS)
		(const boost::function<Signature>& real_proc) : real_proc_(real_proc) {}
	typedef typename boost::function_types::parameter_types<Signature>::type param_types;
	typename boost::function_types::result_type<Signature>::type operator() 
		(const boost::signals2::connection &conn 
		TRACER_SIGNAL_ENUM_FUNCTOR_ARGS(TRACER_SIGNAL_NUM_ARGS))
	{
		conn.disconnect();
		return real_proc_(TRACER_SIGNAL_ENUM_CALLBACK_ARGS(TRACER_SIGNAL_NUM_ARGS));
	}
};

template <unsigned arity,typename Signature,typename Combiner,typename Group,typename GroupCompare,
	typename SlotFunction,typename ExtendedSlotFunction,typename Mutex>
class signal_once_impl;
template <typename Signature,typename Combiner,typename Group,typename GroupCompare,typename SlotFunction,
	typename ExtendedSlotFunction,typename Mutex>
class signal_once_impl<TRACER_SIGNAL_NUM_ARGS, Signature, 
	Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, Mutex>
{
public:
	typedef boost::signals2::signal<Signature, 
		Combiner, Group, GroupCompare, SlotFunction, ExtendedSlotFunction, 
		Mutex> signal_t;
	typedef typename signal_t::extended_slot_type ext_slot_t;
	typedef typename boost::signals2::connect_position conn_pos_t;
	typedef TRACER_SIGNAL_ONCE_WRAP_CLASS_NAME(TRACER_SIGNAL_NUM_ARGS)<Signature> wrap_t;

	template<typename R, typename SlotType>
	R operator()(signal_t *host, const SlotType& in_slot, conn_pos_t at)
	{
		return host->connect_extended(ext_slot_t(&wrap_t::operator(), wrap_t(in_slot), 
			TRACER_SIGNAL_ENUM_SLOT_FUNC_ARGS(TRACER_SIGNAL_NUM_ARGS)), at);
	}

	template<typename R, typename GroupType, typename SlotType>
	R operator()(signal_t *host, const GroupType& group, const SlotType& in_slot, conn_pos_t at)
	{
		return host->connect_extended(group, ext_slot_t(&wrap_t::operator(), wrap_t(in_slot), 
			TRACER_SIGNAL_ENUM_SLOT_FUNC_ARGS(TRACER_SIGNAL_NUM_ARGS)), at);
	}
};