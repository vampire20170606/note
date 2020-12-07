函数模板
====================

初识函数模板
-------------------

函数模板针对不同的型别提供了统一的功能行为，也就是说，函数模板代表了一组函数。

定义函数模板
+++++++++++++++++++++++

模板形参声明格式为 ``template< comma-separated-list-of-parameters >`` ，模板形参列表
既可以是型别参数（e.g typename T），也可以是非型别参数（e.g std::size_t size）。

型别参数除了可以使用 typename 之外，还可以使用 class 来定义，比如 ``template<class T>``，
这个是历史遗留导致的， ``class`` 容易使人产生误解，所以应尽量使用 typename。

以下函数模板隐士要求 max 函数模板的实参型别必须支持 operator< 和 拷贝构造，所以，当使用实际的
模板实参进行实例化时，如果实参型别不满足上述要求将会产生编译时错误。

.. code-block:: c++
    :linenos:

    template<typename T>
    T max(T a, T b)
    {
        return b < a ? a : b;
    }

使用函数模板
+++++++++++++++

以下例子将会隐式实例化三个 max 函数的实例：``int max(int, int);`` 、``int max(double, double);`` 和
``int max(string, string);``。

调用 max 时必须使用 ``::`` 进行限定，不然可能会由于 ADL（Argument-Dependent Lookup） 导致对 ``std::string`` 
型别的参数调用 max 存在歧义或者调用到别的 max 函数。

.. code-block:: c++
    :linenos:

    int main()
    {
        int i = 72;
        std::cout << "max(7, i): " << ::max(7, i) << std::endl;

        double f1 = 3.4;
        double f2 = 0.8;
        std::cout << "max(f1, f2): " << ::max(f1, f2) << std::endl;

        std::string s1 = "c++";
        std::string s2 = "template"
        std::cout << "max(s1, s2): " << ::max(s1, s2) << std::endl;
    }

两阶段翻译
+++++++++++++++

当模板实例化时，会进行两阶段的检查。

1. 在模板定义阶段，会检查模板本身是否正确：

    * 语法错误，比如缺少分号。
    * 使用未知名称，并且该名称不依赖模板形参。
    * 不依赖模板形参的 static assertions。

2. 在模板实例化阶段，依赖于模板形参的代码将会进行检查。

模板实参推导
---------------------

型别推导规则参考 **Effective Modern C++** 。

1. 型别推导时的类型转换

    * 如果函数形参为引用类型，则引用同一个模板形参 T 的函数形参必须推导出一模一样的型别。
    * 如果函数形参为值类型，则 CV 饰词和引用修饰符可以忽略，并且允许数组和函数类型退化为指针类型，
      也就是说引用同一个模板形参 T 的函数形参推导出的型别必须满足 **std::decay_t<T1> == std::decay_t<T2>** 。 

2. 默认模板实参的型别推导

    .. code-block:: c++
        :linenos:

        template<typename T>
        void f(T = "");

        f(); // 编译错误： 不能推导 T 的型别

        template<typename T = std::string>
        void f(T = "");

        f(); // OK

多模板参数
----------------

1. 模板形参，声明在尖括号内。

    .. code-block:: c++

        template<typename T> // T 就是模板形参

2. 调用形参， 声明在函数名称后面的小括号内。

    .. code-block:: c++

        T max(T a, T b); // a 和 b就是调用形参

返回类型的模板参数
++++++++++++++++++++++++

.. code-block:: c++
    :linenos:

    template<typename T1, typename T2, typename RT>
    RT max(T1 a, T2 b);

    ::max<int double, double>(4, 7.2); // 必须显示指定所有的模板实参

    template<typename RT, typename T1, typename T2>
    RT max(T1 a, T2 b);

    ::max<double>(4, 7.2); // 只需要指定返回值的模板实参

推导返回型别
+++++++++++++++++++

.. code-block:: c++
    :linenos:

    // C++14
    template<typename T1, typename T2>
    auto max(T1 a, T2 b)
    {
        return b < a ? a : b;
    }

    C++11
    template<typename T1, typename T2>
    auto max(T1 a, T2 b) -> decltype(b < a ? a : b)
    {
        return b < a ? a : b;
    }

    // 利用 decltype 的编译时计算表达式的型别
    template<typename T1, typename T2>
    auto max(T1 a, T2 b) -> decltype(true ? a : b)
    {
        return b < a ? a : b;
    }

    // 防止返回引用，导致空悬引用
    template<typename T1, typename T2>
    auto max(T1 a, T2 b) -> typename std::decay<decltype(true ? a : b)>::type
    {
        return b < a ? a : b;
    }

作为通用型别返回
+++++++++++++++++++++

.. code-block:: c++
    :linenos:

    template<typename T1, typename T2>
    typename std::common_type<T1, T2>::type max(T1 a, T2 b)
    {
        return b < a ? a : b;
    }

默认模板实参
----------------

.. code-block:: c++
    :linenos:

    template<typename T1, typename T2, typename RT =  typename std::common_type<T1, T2>::type>
    RT max(T1 a, T2 b)
    {
        return b < a ? a : b;
    }

    ::max(4, 7.2);
    ::max<int, double, long double>(4, 7.2); // 指定返回型别需要显示指定所有参数型别

    template<typename RT = double, typename T1, typename T2>
    RT max(T1 a, T2 b)
    {
        return b < a ? a : b;
    }

    ::max(4, 7.2);
    ::max<long double>(4, 7.2); // 指定返回型别不需要显示指定所有参数型别

重载函数模板
--------------------

允许多个同名的普通函数和函数模板同时存在，也允许多个同名的函数模板同时存在，在进行重载解析时，
会更偏爱普通函数，如果存在多个函数模板匹配，则会产生歧义，从而导致编译错误。

.. code-block:: c++
    :linenos:
    
    template<typename T>
    T max(T a, T b)
    {
        return b < a ? a : b;
    }

    template<typename T>
    T* max(T* a, T* b)
    {
        return *b < *a ? a : b;
    }

    char const* max(char const* a, char const* b)
    {
        return std::strcmp(b, a) < 0 ? a : b;
    }

    int main()
    {
        int a = 7;
        int b = 42;
        auto m1 = ::max(a, b); // 匹配第一个函数模板

        std::string s1 = "hello";
        std::string s2 = "world";
        auto m2 = ::max(s1, s2); // 匹配第一个函数模板

        int *p1 = &a;
        int *p2 = &b;
        auto m3 = ::max(p1, p2); // 匹配第二个函数模板

        char const* x = "hello";
        char const* y = "world";
        auto m4 = ::max(x, y); // 匹配最后一个普通函数
    }
