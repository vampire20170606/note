非类型模板参数
##################

对于函数模板和类模板，模板参数不必是型别，它们也可以是普通的值。

非类型的类模板参数
=====================

前面实现的 Stack 类模板使用 STL 容器来进行内存管理，我们也可以通过栈空间来实现内存管理，就像在函数内
声明一个局部数组一样，当函数结束的时候，该变量所占用的内存空间也被释放了。通过对数组的大小进行参数化，从
而将分配空间大小的权利交给了用户。

.. literalinclude:: codes/stacknontype.hpp
    :language: c++    
    :linenos:
    
如果你想使用这个模板，你除了要指定第一个型别模板参数外，还需要指定第二非型别的模板参数。

.. code-block:: c++
    :linenos:

    int main()
    {
        Stack<int, 20> int20Stack;
        Stack<int, 40> int40Stack;
        Stack<std::string, 40> stringStack;
        
        int20Stack.push(10);
        int20Stack.pop();
        
        stringStack.push("hello");
        stringStack.pop();
    }

每个模板实例是不同的型别，所以 int20Stack 和 int40Stack 是不同的型别。非型别模板参数
也可以指定默认实参。

.. code-block:: c++

    template<typename T, std::size_t MaxArraySize = 100>
    class Stack {
        ...
    };

非类型函数模板参数
=====================

同样也可以给函数模板定义非型别的模板参数。

.. code-block:: c++

    template<int Val, typename T>
    T addValue(T x)
    {
        return x + Val;
    }

你可以把这个函数模板的实例传递给 C++ 标准库的 transform 函数的第四个参数：

.. code-block:: c++

    std::transform(source.begin(), source.end(), dest.begin(), addValue<5, int>);

.. note:: 

    addValue 必须显示指定模板实参，因为此时它是作为transform函数的参数，只有在函数
    直接调用时才会发生型别推导。

.. code-block:: c++

    // 使用非型别参数推导型别
    template<auto Val, typename T = decltype(Val)>
    T foo();

    // 非型别参数的型别和型别参数一样
    template<typename T, T Val = T{}>
    T bar();

非类型模板参数的限制
========================

一般来说，非类型模板参数只能是整型常量值（包括枚举常量）、对象/函数/成员的指针、对象或者函数
的左值引用，或者空指针（nullptr）。

浮点数和类型别的对象是不允许作为非型别的模板参数的。

.. code-block:: c++
    :linenos:

    template<double Val>
    void foo(); // 错误：浮点值不允许作为非型别模板参数

    struct A {};
    template<A a>
    class foo {}; // 错误：类型别的对象不允许作为非型别模板参数

当传递指针或者引用的模板实参时，对象不能是字符串字面量，临时对象，或者数据成员或者其它
的子对象。

.. code-block:: c++
    :linenos:

    struct A {
        int a;
    };

    template<auto& Val>
    void foo()

    foo<"hello">(); // 错误
    foo<A{}>(); // 错误

    int a; // 局部变量
    foo<a>() // 错误

    A m;
    foo<m.a>(); // 错误

每个C++标准也会对对象的存储类型有所限制：

- C++98，对象只能是外部链接
- C++11/C++14，增加对内部链接的支持
- C++17，增加对局部静态变量支持

.. code-block:: c++
    :linenos:

    extern char const S03[] = "hi"; // external linkage
    char const s11[] = "hi"; // internal linkage
    static char const s11_1[] = "hi"; // internal linkage

    int main()
    {
        static char const s17[] = "hi"; // no linkage

        Message<s03> m03; // ok, all version
        Message<s11> m11; // ok, since C++11
        Message<s11_1> m11_1; // ok, since C++11
        Message<s17> m17; // ok, since C++17

    }

避免无效表达式
-----------------

当传递非型别模板实参时，你可以传递任何编译时表达式。然而，如果使用的表达式包含 ``>`` 时，
你需要把整个表达式放在小括号中，来避免嵌套的 ``>`` 终止实参列表：

.. code-block:: c++

    template<int I, bool B>
    class C;

    C<42, sizeof(int) > 4> c; // 错误
    C<42, (sizeof(int) > 4)> c; // ok
 
模板参数型别 auto
============================

从 C++17 开始，你可以使用 auto 来定义非型别模板参数允许的任意型别的非型别模板参数。
即泛化非型别的模板参数，然后由型别推导机制来推导非型别模板参数的实际型别。但是请注意，
此处推导出来的型别必须满足非型别模板参数的限制。

.. code-block:: c++

    template<typename T, auto MaxArraySize>
    class Stack;

    Stack<int, 20u> int40Stack; // decltype(MaxArraySize) == unsigned
    Stack<int, 40> int40Stack; // decltype(MaxArraySize) == int

    template<decltype(auto) N>
    class C;

    // 使用括号会使得decltype推导出引用型别
    int i;
    C<(i)> x; // decltype(N) == int&

.. code-block:: c++
    :linenos:

    template<decltype(auto) N>
    class C {
    public:
        C() {
            std::cout << N <<"\n";
        }
    };

    int i;
    C<i> x0; // 错误，非常量
    C<(i)> x1; // ok，全局变量的引用是常量
    const int ci = 10;
    C<ci> x2; // ok
    constexpr int cei = 10;
    C<cei> x3; // ok

    int main()
    {
        const int local = 10;
        C<local> x4; // ok
        C<(local)> x5;
    }


.. note:: 

    非型别的模板参数一定是常量表达式
