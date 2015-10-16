Tracer
======

A C++ library that enables you to decorate a function at runtime.

Introduction
---

Tracer can insert callbacks before or after a call to a given function without the need of modifying its code. [中文版](https://github.com/QingYun/tracer/blob/master/README.cn.md)

Usage
---

Include `tracer/tracer.h` and add the path to Boost into the include directories option.

###Tracers

Use `TRACER_TRACE(func)` as a type name to define a variable, which could be called a `tracer`, `func` is a function pointer and we call it the original function.

This `TRACER_TRACE` macro will expand to a class:

    TRACER_TRACE(&Foo) foo;
    
    // equals to 
    typedef TRACER_TRACE(&Foo) FooTracer;
    FooTracer foo;

A `tracer` has three public methods:

- **Before()**

  Returns a reference to a `tracer::Signal` object, this signal will be triggered just before the original function being called.
  
  `tracer::Signal` derives from `boost.signals2`, so you can use all its public methods, like using `connect(callback)` to register a new callback which will be called every time when this signal is triggered. 
  
  The callback should not return anything so its signature is like `void(bool&, ...)`. The first parameter is always a `bool&`, a flag with a default value of `true`, indicates if the original function should be called, so if you don't want the original function to be called, just simply assign `false` to it; remained parameters are references to the arguments to the original function, in the same order. If the original function is a non-static function member, then there will be a extra parameter as the second parameter, wich is the reference to `this`
  
  Examples:
  
    - `int(int)` => `void(bool&, int&)`
    - `std::string (C::*)(std::string)` => `void(bool&, C*&, std::string&)`

- **After()**

  Returns a reference to a `tracer::Signal` object which will be triggered after the original function is called
  
  The callback signature is almost the same as the one for `Before()`, the only difference is the first parameter becomes a `bool` instead of a `bool&` to indicates whether the original function has been called and the second parameter will be a reference to its return value.

- **RealFunc()**

  Returns a function pointer that has the same signature as the original function and invocations to this function pointer will be directly directed to the original function without triggering callbacks.

Besides methods inherited from `boost.signals2`, there are two new methods in `tracer::Signal`:

- `once(cb)` : like `connect` but the callback will be disconnected automatically after the first invocation.
- `connect_without_params(cb)` : like `connect` but always receives a `void()` function. It's very handy when you have no interests in parameters.

**example code**:

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
    	
    	// conn is for managing connection state
    	auto conn = t.Before().connect([&b] (bool&, C *&self) {
    		// forward all the calls to b
    		self = &b;
    	});
    	
    	std::string result = a.Get();	// result == "B"
    	conn.disconnect();
    	result = a.Get();				// result == "A"
    }
    
    
It doesn't matter whether the argument passed to the `TRACER_TRACE` macro is just a variable or a complex expression, as long as it evaluates to a function pointer. That means if you want to trace a certain function with a set of overloaded functions, you can just convert it to the signature you want, like `static_cast<func_point_type>(&func_name)`. It also very useful when you try to trace a function you only got its signature at compile-time and you don't know its address until run-time, such as functions in a COM component. 
    
- - -

###Recorders

Some helper classes to record calling information of a traced function.

####CallCountRecorder

Record how many times a function is called. 

You can use `CallCountRecorder<decltype(tracer)> recorder(tracer)` or `auto recorder = RecordCallCount(tracer)` to create one.

It has two public methods:

- `bool HasBeenCalled()` : Returns a `bool` value indicates whether this function is called at least once.
- `std::size_t CallCount()` : Returns the exact times.
 
####ArgRecorder

Record all the arguments passed.

You can use `ArgRecorder<decltype(tracer)> recorder(tracer)` or `auto recorder = RecordArgs(tracer)` to create one.

It has one public method:

- `nth-param-type Arg<I>(n)` : Returns the `I`th argument in the `n`th call. 

####RetValRecorder

Record the return value.

`RetValRecorder<decltype(tracer)> recorder(tracer)` or `auto recorder = RecordRetVal(tracer)`

- `ret-val-type RetVal(n)` : Returns the return value of the `n`th call.

####CallStackRecorder

Record the call stack

`CallStackRecorder<decltype(tracer)> recorder(tracer)` or `auto recorder = RecordCallStack(tracer)`

- `CallStack GetCallStack(n)` : Returns the call stack of the `n`th call.
 
    `CallStack` has two public methods : 

    - `vector<CallStackEntry> Entries()` : Returns all the records, `Entries()[0]` is the direct caller of the original function, `Entries()[1]` is the caller's caller and so on.
    
        `CallStackEntry` has four public methods:
        
        - `File()` : Returns the name of the file.
        - `Line()` : Returns the line number
        - `FuncName()` : The name of the function
        - `FuncAddr()` : The address of the function
        
    - `bool IsCalledBy(f)` : Returns a `bool` value indicates whether the function `f` appears in the recorded call stack. `f` could be a function name as a string or a function pointer

Example:

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
    	
    	// print the information of the direct caller
    	for (auto itr : fc.GetCallStack(0).Entries())
    		std::cout << itr.File() << " " << itr.Line() << " " << itr.FuncName() << std::endl;

        // check caller by name
    	assert(true == fc.GetCallStack(0).IsCalledBy("RunFoo"));
    	
    	void(*f)();
    	f = [] () { RunFoo(); };
    	f();
    	// or by function pointer
    	assert(true == fc.GetCallStack(1).IsCalledBy(f));
    }

- - -

###Mixin

The macro `TRACER_TRACE_WITH` could mix the functions of tracer and recorder. It expects two arguments, a function pointer just like the one you would pass into `TRACER_TRACE`, and a list of recorder in the form of `(RecorderType1)(RecorderType2)`. 

To record how many times the function `C::Foo` is called as well as its call stacks, you write:

    TRACER_TRACE_WITH(C::Foo, (tracer::CallCountRecorder)(tracer::CallStackRecorder)) foo;
    
Then, `foo` will inherit all the public methods of `CallCountRecorder` and `CallStackRecorder`. That means you can use `foo.Before().connect()` to insert a callback; use `foo.HasBeenCalled()` check whether it is called and `foo.GetCallStack()` to see the call stacks.

As you may expect, you can also mix your own recorder into a tracer as long as it has a constructor with the signature of `Recorder<TracerType>::Recorder(TracerType&)`