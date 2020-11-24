有技巧的基础知识
#######################

本章将介绍实际使用模板时比较有技巧性的基础知识。

关键字 typename
===================

.. code-block:: c++
    :linenos:

    template<typename T>
    class MyClass {
    public:
        ...
        void foo() {
            typename T::SubType* ptr;
        }
    };

在上面的例子中，第二个 ``typename``  被用来阐明 SubType 是一个定义在类 T 中的一个型别。因此，
ptr 是一个指向 T::SubType 的指针。

如果没有 typename 的话， SubType 将会被假设为一个非型别的成员（比如静态数据成员或者枚举常量）。结果，
表达式 ``T::SubType* ptr`` 将被解释为一个乘法表达式。这不是一个错误，在某些 MyClass 模板的实例中
它可能就是一个合法的表达式。

一般来说， typename 得在依赖模板参数并且表示型别的名称的地方被使用。

零初始化
==================

.. code-block:: c++

    template<typename T>
    void foo()
    {
        T x; // 对于内置型别，未初始化
        T x = T() // C++11 之前的初始化语法
        T x{}; // C++11 之后的初始化语法
    }

使用 this->
====================

如果类模板的基类依赖于模板参数，那么当使用一个名称 x 时，并不总是等价于 this->x 。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Base {
    public:
        void bar();
    };

    template<typename T>
    class Derived： public Base<T> {
    public:
        void foo() {
            bar(); // 调用外部的 bar 或者报错
            this->bar(); // 没问题
            Base<T>::bar(); // 没问题
        }
    };

.. note:: 

    根据经验，我们推荐你在使用依赖模板参数的基类模板中的名称时，总是使用 this-> 或者 Base<T>:: 进行限定。

原始数组和字符串字面量模板
================================

当传递原始数组或者字符串字面量给模板时，你必须小心。首先，如果模板参数被声明为引用时，实参不会退化；按值传递
则会导致退化为指针型别。

我们也可以利用模板推导获取数组的维数。

.. code-block:: c++
    :linenos:

    template<typename T, int N>
    void foo(T (&a)[N]); // void foo(T a[N]) 是不可以的

    int a[10];
    foo(a); // 推断 N == 10

成员模板
============

类成员也可以是一个模板（比如嵌套类和成员方法）。比如我们实现不同模板实参型别的类模板的拷贝赋值运算。

.. code-block:: c++

    template<typename T>
    class Stack {
    ...

    template<typename U>
    Stack& operator= (Stack<U> const&);
    };

    // 注意 Stack<T> 和 Stack<U> 是不同的型别
    template<typename T>
    template<typename U>
    Stack<T>& Stack<T>::operator= (Stack<U> const& op2) {
        Stack<U> tmp(op2);

        elems.clear(); // 会导致不是异常安全
        while(!tmp.empty()) {
            elems.push_front(tmp.top());
            tmp.pop();
        }

        return *this;
    }

    Stack<int> intStack;
    Stack<float> floatStack;

    floatStack = intStack;

成员函数模板特化
---------------------

成员函数模板可以全特化，本质是对某个具体的型别显示实例化成员方法。

.. code-block:: c++
    :linenos:

    class BoolString {
    private:
        std::string value;
    public:
        BoolString(std::string const& s) : value(s) {}
        
        template<typename T = std::string>
        T get() const {
            return value;
        }
    };

    template<>
    inline bool BoolString::get<bool>() const {
        return value == "true" || value == "1" || value == "on";
    }

特殊成员函数模板
---------------------

由编译自动生成的拷贝构造、移动构造、拷贝赋值、移动赋值也可以声明相应的成员模板方法，但是这个模板
方法不会替代由编译器自动生成的构造方法或者赋值运算符重载。也就是说，带模板的构造和赋值运算可以和
预定义的构造和赋值运算符同时存在，优先匹配预定义的构造和赋值运算符，如果模板构造和赋值运算符更加
精确的匹配，则选择带模板的构造和赋值运算符。

.template 构造
-----------------------

有时，当调用一个模板成员时，你必须显示限定模板实参。此时，你必须使用 ``template`` 关键字来确保 ``<`` 是模板实参
列表的开始。

.. code-block:: c++

    template<unsiged long N>
    void printBitSet(std::bitset<N> const& bs) {
        std::cout << bs.template to_string<char, std::char_traits<char>, 
            std::allocator<char>>();
    }

``.template`` （或者 ``->template``、``::template``）仅在模板内部使用，并且仅在某个对象依赖于模板
参数且调用该对象的某个成员方法。
