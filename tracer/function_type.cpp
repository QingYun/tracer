#define I BOOST_PP_ITERATION()

template<typename C BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct COMToNormal<HRESULT(__stdcall C::*)(BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef HRESULT __stdcall type(C* BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename Adding, typename R 
	BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct PrependParameter<R (BOOST_PP_ENUM_PARAMS(I, P)), Adding> {
	typedef R type(Adding BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename Adding, typename R 
	BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct PrependParameter<R __stdcall (BOOST_PP_ENUM_PARAMS(I, P)), Adding> {
	typedef R __stdcall type(Adding BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct PrependParameter<R (BOOST_PP_ENUM_PARAMS(I, P)), void> {
	typedef R type(BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct PrependParameter<R __stdcall (BOOST_PP_ENUM_PARAMS(I, P)), void> {
	typedef R __stdcall type(BOOST_PP_ENUM_PARAMS(I, P));
};

#define M(z, n, P) BOOST_PP_CAT(P, n)&
template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct AllParamsToRef<R (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R type(BOOST_PP_ENUM(I, M, P));
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct AllParamsToRef<R __stdcall(BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R __stdcall type(BOOST_PP_ENUM(I, M, P));
};
#undef M

template<typename Setting, typename 
	R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct SetResultType<R (BOOST_PP_ENUM_PARAMS(I, P)), Setting> {
	typedef Setting type(BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename Setting, typename 
	R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct SetResultType<R __stdcall (BOOST_PP_ENUM_PARAMS(I, P)), Setting> {
	typedef Setting __stdcall type(BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct RemoveStdcall<R (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R type(BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct RemoveStdcall<R __stdcall (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R type(BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct ResultType<R (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R type;
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct ResultType<R __stdcall (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R type;
};

#if I < 8

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct FoldParameters<R (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R type(BOOST_PP_ENUM_PARAMS(I, P));
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct FoldParameters<R __stdcall (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R __stdcall type(BOOST_PP_ENUM_PARAMS(I, P));
};

#else

#define M(z, n, P) BOOST_PP_CAT(P, BOOST_PP_ADD(n, 6))

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct FoldParameters<R (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R type(BOOST_PP_ENUM_PARAMS(6, P), ::std::tuple<
		BOOST_PP_ENUM(BOOST_PP_SUB(I, 6), M, P)>);
};

template<typename R BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
struct FoldParameters<R __stdcall (BOOST_PP_ENUM_PARAMS(I, P))> {
	typedef R __stdcall type(BOOST_PP_ENUM_PARAMS(6, P), ::std::tuple<
		BOOST_PP_ENUM(BOOST_PP_SUB(I, 6), M, P)>);
};

#undef M

#endif


#if I < 8

#define PARAM(z, n, _) BOOST_PP_CAT(P, n) && BOOST_PP_CAT(p, n)

template<typename F BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
auto ForwardToFoldedParameters(F func BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM(I, PARAM, _)) ->
	typename ::std::result_of<F(BOOST_PP_ENUM_PARAMS(I, P))>::type {
		return func(BOOST_PP_ENUM_PARAMS(I, p));
}

#undef PARAM

#else

#define PARAM(z, n, _) BOOST_PP_CAT(P, n) && BOOST_PP_CAT(p, n)
#define M(z, n, P) BOOST_PP_CAT(P, BOOST_PP_ADD(n, 6))

template<typename F BOOST_PP_COMMA_IF(I) BOOST_PP_ENUM_PARAMS(I, typename P)>
auto ForwardToFoldedParameters(F func, BOOST_PP_ENUM(I, PARAM, _)) ->
	typename ::std::result_of<F(BOOST_PP_ENUM_PARAMS(6, P), ::std::tuple<
	BOOST_PP_ENUM(BOOST_PP_SUB(I, 6), M, P)>)>::type {
		return func(BOOST_PP_ENUM_PARAMS(6, p), ::std::tie(
			BOOST_PP_ENUM(BOOST_PP_SUB(I, 6), M, p)));
}

#undef PARAM
#undef M

#endif