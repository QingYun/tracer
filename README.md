Tracer
======

A simple hook library for testing and debuging

Introduction
---

Tracer可以让你在不修改代码的前提下, 在指定函数调用前后以回调的形式插入代码.

Usage
---

首先, 包含头文件`tracer/tracer.h`, 然后配置一下Boost路径.

###Tracers

使用`TRACER_TRACE(func)`来定义一个变量, 这个变量我们称之为一个`tracer`, `func`应该是一个求值为函数指针的表达式, 我们称之为原始函数.

这个宏展开后是一个类:

    TRACER_TRACE(&Foo) foo;
    // 等价于
    typedef TRACER_TRACE(&Foo) FooTracer;
    FooTracer foo;

`tracer`有三个公开的方法:

- **Before()**

  返回一个`tracer::Signal`对象的引用, 它会在调用原始函数前被触发.
  
  `tracer::Signal`派生自`boost.signals2`, 可以使用其所有公开接口, 比如使用`connect(callback)`注册一个回调, 这个回调会在每次信号被触发时被调用. 
  
  回调应该是一个没有返回值的可调用对象, 其第一个参数应该是`bool&`类型的, 表示是否想要调用原始函数, 将其赋值为`false`将不会调用原始函数; 如果原始函数是一个类的非静态成员函数, 则第二个参数应该是类指针的引用, 传入时它的值即为`this`, 在回调中可以修改它, 将其指向其他对象, 这样可以将调用转移到其他对象上; 剩下的参数依次是原始函数参数的引用, 它们都可以在回调中被修改.
  
  比如原始函数的签名为`int(int)`, 则回调的类型应该是`void(bool&, int&)`

- **After()**

  返回一个`tracer::Signal`对象的引用, 它会在调用原始函数后被触发.
  
  回调类型类似于`Before()`的回调, 但是第一个参数应该是`bool`的, 表示是否已经调用了原始函数, 接下来是返回值的引用, 剩下的和`Before()`一样

- **RealFunc()**

  返回一个和原始函数签名一样的函数指针, 调用这个指针可以避免触发信号, 直接调用原始函数.

除了`boost.signals2`原有的接口, `tracer::Signal`还提供了两个新的方法:

- `once(cb)` : 类似于`connect`, 但是这个回调会在被触发一次之后自动断开连接
- `connect_without_params(cb)` : 类似于`connect`, 但是回调的签名应该是`void()`的.

**示例**:

    class C {
    	std::string str_;
    public:
    	C(const std::string &str) :
    		str_(str)
    	{}
    	std::string Get() {
    		return str_;
    	}
    };
    
    int main() {
    	TRACER_TRACE(&C::Get) t;
    	C a("A"), b("B");
    	
    	// 注册一个在C::Get调用前被调用回调, conn是回调的链接管理器
    	auto conn = t.Before().connect([&b] (bool&, C *&self) {
    		// 将所有对C::Get的调用都无条件转移到对象b身上
    		self = &b;
    	});
    	
    	std::string result = a.Get();	// result == "B"
    	conn.disconnect();
    	result = a.Get();				// result == "A"
    }
    
    
传递给宏的可以是任意求值为函数指针的表达式, 因此如果需要追踪有多个重载版本的函数的某一重载版本, 
只需传递`static_cast<func_point_type>(&func_name)`即可, 这个技巧还可以用来处理那些在编译期只知道类型, 地址在运行期才能获取的函数
    
- - -

###Recorders

`recorder`是用来记录`tracer`调用情况的类.

####CallCountRecorder

记录调用次数. 

可以使用`CallCountRecorder<decltype(tracer)> recorder(tracer)` 或者 `auto recorder = RecordCallCount(tracer)`创建, 
它有两个公开方法:

- `bool HasBeenCalled()` : 返回一个`bool`值表示原始函数是否被调用过.
- `std::size_t CallCount()` : 返回原始函数被调用的次数.
 
####ArgRecorder

记录传递给原始函数的所有参数

可以使用`ArgRecorder<decltype(tracer)> recorder(tracer)` 或者 `auto recorder = RecordArgs(tracer)`创建, 它有一个公开方法:

- `nth-param-type Arg<I>(n)` : 返回开始记录后第`n`次调用时的第`I`个参数. 

####RetValRecorder

记录函数的返回值

可以使用`RetValRecorder<decltype(tracer)> recorder(tracer)` 或者 `auto recorder = RecordRetVal(tracer)`创建, 
它有一个公开方法:

- `ret-val-type RetVal(n)` : 返回开始记录后第`n`次调用时的返回值

####CallStackRecorder

记录调用栈

可以使用`CallStackRecorder<decltype(tracer)> recorder(tracer)` 或者 `auto recorder = RecordCallStack(tracer)`创建, 
它有一个公开方法 :

- `CallStack GetCallStack(n)` : 返回开始记录后第`n`次调用时的调用栈.
 
    `CallStack`对象有两个公开方法 : 

    - `vector<CallStackEntry> Entries()` : 返回整个调用栈记录, `Entries()[0]`是原始函数的调用者, `Entries()[1]`是原始函数调用者的调用者, 依此类推
    
        `CallStackEntry`对象有4个公开方法:
        
        - `File()` : 返回函数所在的文件名称
        - `Line()` : 返回函数在文件中的行号
        - `FuncName()` : 返回函数名
        - `FuncAddr()` : 返回函数地址
        
    - `bool IsCalledBy(f)` : `f`可以是字符串形式的函数名或者是函数指针, 如果在调用栈中找到匹配项则返回`true`, 否则返回`false`.

示例:

    struct C {
    	void Foo() {}
    };
    
    void RunFoo() {
    	C c;
    	c.Foo();
    }
    
    int main() {
    	TRACER_TRACE(&C::Foo) foo;
    	auto fc = tracer::RecordCallStack(foo);
    
    	RunFoo();
    	
    	// 输出第一次调用中各个调用者的信息
    	for (auto itr : fc.GetCallStack(0).Entries())
    		std::cout << itr.File() << " " << itr.Line() << " " << itr.FuncName() << std::endl;
    		
    	// 检查第一次调用时调用栈中是否有名为"RunFoo"的调用者
    	assert(true == fc.GetCallStack(0).IsCalledBy("RunFoo"));
    	
    	void(*f)();
    	f = [] () { RunFoo(); };
    	f();
    	// 通过比较函数地址检查第二次调用时是否有调用者f
    	assert(true == fc.GetCallStack(1).IsCalledBy(f));
    }

- - -

###Mixin Tracer

使用`TRACER_TRACE_WITH`可以把 tracer 和 recorder 的功能混合到一起. 

这个宏接受两个参数, 第一个参数是函数名, 第二个宏是要混合的 recorder 列表, 是`(recorder1)(recorder2)`形式的. 
比如要记录`C::Foo`的调用次数和调用栈, 可以这样写

    TRACER_TRACE_WITH(C::Foo, (tracer::CallCountRecorder)(tracer::CallStackRecorder)) foo;
    
`foo`继承了这两个 recorder 的接口, 所以你可以用`foo.Before().connect()`插入调用前的回调, 也可以用`foo.HasBeenCalled()`判断`C::Foo`是否被调用过, 还可以用`foo.GetCallStack()`来获取调用栈.

除了内置的 recorder 以外, 只要是用`Recorder<T>::Recorder(T&)`形式构造的自定义 recorder 也可以用`TRACER_TRACE_WITH`混合.
