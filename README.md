tracer
======

A simple hook library for testing and debuging

Usage
---

First, `#include "tracer/tracer.h"`.

Then you can use `TRACER_TRACE_NORMAL_FUNC(func_name) var;` or `TRACER_TRACE_MEMBER_FUNC(class_name::method) var;` to make 
a object, a hook will be installed to the target function in the constructor. The object has three public method:

- **Before()** 

  Returns a reference to a `tracer::Signal` object which will be triggered before the actual call to the original function. 
  
  `tracer::Signal` is derived from `boost.signals2` and you can use all its public interfaces. The slot callback should 
  take a `bool&` variable as its first parameter which indicates whether to call the original function. If you are tracing 
  a member function, the second parameter should be a reference to `this` and you can change it to transfer the call to 
  other objects. The remaining parameters should be references to the original function parameters so you can modify the 
  parameters passed to the original function.
  
- **After()**

  Returns a reference to a `tracer::Signal` object which will be triggered after the actual call to the original function. 
  
  The callback should take a `bool` variable as its first parameter that indicates whether the original function has been 
  called. The second parameter should be a reference to the return value and you can modify it in the callback. The 
  remaining parameters should be a reference to `this` for member function and references to the original function parameters.
  
- **RealFunc()**
 
  Returns a function pointer, you can use it as the original function without tracing.
