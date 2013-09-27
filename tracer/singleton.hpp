#pragma once

namespace tracer {

/*!
\brief 单例基类

如果想要类 A 实现单例模式, 则需要继承类 Singleton<A> . 如果类 A 的构造函数不是公开的, 则还需要
再加一句 friend class Singleton<A>;
*/
template <class T> 
class Singleton { 
public: 
	static T& Instance() { 
		static T _instance; 
		return _instance; 
	}
protected: 
	Singleton(void) {} 
	virtual ~Singleton(void) {} 
	Singleton(const Singleton<T>&);
	Singleton<T>& operator= (const Singleton<T> &);
}; 

}	// namespace agt