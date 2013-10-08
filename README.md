tracer
======

A simple hook library for testing and debuging

Usage
---

首先, 包含头文件`tracer/tracer.h`.

###Tracers

使用`TRACER_TRACE_NORMAL_FUNC(func_name)`或`TRACER_TRACE_MEMBER_FUNC(class_name::func_name)`来定义一个变量, 这个变量我们称之为一个`tracer`, `func_name`和`class_name::func_name`表示的函数我们称之为原始函数.

这两个宏展开后都是一个类, 可以把它们当做一个类型看待.

    TRACER_TRACE_NORMAL_FUNC(Foo) foo;
    // 等价于
    typedef TRACER_TRACE_NORMAL_FUNC(Foo) FooTracer;
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

- **once(cb)** : 类似于`connect`, 但是这个回调会在被触发一次之后自动断开连接
- **connect_without_params(cb)** : 类似于`connect`, 但是回调的签名应该是`void()`的.

###Recorders

`recorder`是用来记录`tracer`调用情况的类.

####CallCountRecorder

记录调用次数. 

可以使用`CallCountRecorder<decltype(tracer)> recorder(tracer)` 或者 `auto recorder = RecordCallCount(tracer)`创建, 
它有两个公开方法:

- **bool HasBeenCalled()** : 返回一个`bool`值表示原始函数是否被调用过.
- **std::size_t CallCount()** : 返回原始函数被调用的次数.
