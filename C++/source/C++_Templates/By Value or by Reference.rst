按值传递还是按引用传递
###########################

从一开始，C++ 就提供了按值传递和按引用传递，但并不总是很容易判断使用哪种方式：通常，按引用传递非平凡的对象效率更高
但是更加复杂。C++11 引入了移动语义，因此现在有以下不同的方式按引用传递：

1. X const&（常量左值引用）

    参数指向被传递的对象，并且不能修改它。


2. X&（非常量左值引用）

    参数指向被传递的对象，可以修改它。

3. X&&（右值引用）

    参数指向被传递的对象，携带移动语义，你可以修改它或者窃取它的值。

通常我们推荐在函数模板中使用按值传递，除非你有更好的理由，比如以下几种情况适合按引用传递：

- 拷贝构造被禁止
- 参数被用来返回数据
- 函数模板仅仅用来转发参数
- 有重要的性能提升

按值传递
===================

当使用按值传递时，每个实参原则上都会被拷贝。

调用拷贝构造函数可能会非常昂贵。然而，有多种方式避免昂贵的拷贝操作：实际上，编译器可能会优化掉
拷贝操作，并且使用移动语义来使得对复杂的对象的拷贝操作变得廉价。

.. code-block:: c++

    template<typename T>
    void printV(T arg);

    std::string returnString();
    std::string s = "hi";

    printV(s) // 调用拷贝构造
    printV(std::string("hi")); // 拷贝通常会被优化掉（如果没优化的话，则会调用移动构造）
    printV(returnString()); // 拷贝通常会被优化掉（如果没优化的话，则会调用移动构造）
    printV(std::move(s)); // 调用移动构造

第一次调用传递了一个左值，因此拷贝构造函数会被调用。然而第二个和第三个调用，传递一个 prvalues
（动态创建或者函数返回的临时对象），编译器通常会优化掉拷贝操作。注意从C++17开始这个优化是必须的。
C++之前，如果编译器没有优化掉拷贝操作，至少必须尝试使用移动语义。最后一个调用，传递xvalue（对
一个非常量对象使用std::move），会强制调用移动构造函数。

因此，按值传递仅在传递一个左值时会调用拷贝构造函数，但这是一个很常见的情况。

按值传递参数退化
-------------------

按值传递还有另一个值得一提的属性：按值传递实参时，型别会发生退化。这意味着普通的数组会转换成指针并且
const 和 volatile 修饰词会被移除，函数会转换成函数指针。

.. code-block:: c++

    template<typename T>
    void printV(T arg);

    std::string const c = "hi";
    printV(c); // c 会退化成 std::string

    printV("hi"); // 退化成 char const *

    int a[4];
    printV(a); // 退化成 int *

这个行为继承自 C 语言，它有有点也有缺点。优点是它简化了对字符串字面量的处理，缺点则是在 printV 中
我们无法区分传递的是数组还是指针。

按引用传递
================

按引用传递不会导致拷贝构造被调用；也不会发生传递的实参的型别发生退化。

按常量引用传递
-----------------

为了避免拷贝操作，我们可以使用常量引用。

.. code-block:: c++

    template<typename T>
    void printR(T arg);

    std::string returnString();
    std::string s = "hi";

    printR(s) // 不会发生拷贝操作
    printR(std::string("hi")); // 不会发生拷贝操作
    printR(returnString()); // 不会发生拷贝操作
    printR(std::move(s)); // 不会发生拷贝操作

在底层，传递引用是通过传递实参的地址实现的。从调用者传递一个地址给被调用者是非常高效的。但是，
传递地址会让编译器在编译调用者的代码时产生不确定性：调用者会使用这个地址做什么呢？理论上，被调用
者可以通过这个地址改变这个地址指向的区域的值。也就是说，编译器必须假设该地址缓存的所有值在调用之后
都会失效。重新加载这些值是相当昂贵的。

内联可以缓解这个问题，但是并不是所有的方法都会被内联。

按引用传递不会发生退化
++++++++++++++++++++++++++++++++

当按引用传递实参到形参时，不会发生退化。也就是说数组不会转换为指针，修饰词如 ``const`` 和 
``volatile`` 不会被移除。然而，由于形参声明为 ``T const&``，模板参数 ``T`` 本身不会被
推导为 ``const`` 。

.. code-block:: c++

    template<typename T>
    void printR(T const& arg);

    std::string const c = "hi";
    printR(c); // T 推导为 std::string

    printR("hi"); // T 推导为 char[3]

    int arr[4];
    printR(arr); // T 推导为 int[4]

因此，在 printR 函数模板内，使用 T 声明的局部对象不是 const 的。

按非常量引用传递
=================

如果你想通过传递的实参返回数据，你不得不使用非常量引用（除非你更喜欢指针传参）。

.. code-block:: c++

    template<typename T>
    void outR(T& arg);

使用临时对象（prvalue）或者使用std::move移动过的对象（xrvalue）调用 outR 函数模板是不被允许的。

 .. code-block:: c++

    std::string returnString();
    std::string s = "hi";

    outR(s); //没问题： T 被推导为 std::string
    outR(std::string("hi")); // 错误：不允许 prvalue
    outR(returnString()); // 错误：不允许 prvalue
    outR(std::move(s)); // 错误：不允许 xvalue

如果你传递一个常量的实参，形参型别将会被推导为常量引用，也就是说传递右值突然被允许了。

.. code-block:: c++

    std::string const returnConstString();
    std::string const c = "hi";

    outR(c); // 没问题：T 被推导为 std::string const
    outR(returnConstString); // 没问题:  T 被推导为 std::string const
    outR(std::move(c)); // 没问题： T 被推导为 std::string const
    outR("hi"); // 没问题： T 被推导为 char const[3]

在上述情况下，在函数模板内任何试图修改被传递的实参的行为都会导致错误。

如果你想禁用常量对象传递给非常量引用，你可以使用以下方式：

- 使用静态断言触发编译错误

    .. code-block:: c++

        template<typename T>
        void outR(T& arg) {
            static_assert(!std::is_const<T>::value, 
                "out parameter of outR<T>(T&) is const");
            ...
        }

- 使用 enable_if 表达式禁用模板

    .. code-block:: c++

        template<typename T, 
            typename = std::enable_if_t<!std::is_const<T>::value>>
        void outR(T&arg) {
            ...
        }
    
  或者使用 concepts

    .. code-block:: c++

        template<typename T>
        requires !std::is_const_v<T>
        void outR(T& arg) {
            ...
        }


按转发引用传递
==================

使用引用传递实参的一个原因是能够完美转发参数。但是当使用转发引用时，会应用一些特殊的规则。

.. code-block:: c++

    template<typename T>
    void passR(T&& arg);

    std::string returnString();
    std::string s = "hi";

    passR(s); // 没问题： T 被推导为 std::string &
    passR(std::string("hi")); // 没问题： T 被推导为 std::string
    passR(returnString()); // 没问题： T 被推导为 std::string
    passR(std::move(s)); // 没问题： T 被推导为 std::string

    int arr[4];
    passR(arr); // 没问题： T 被推导为 int (&)[4]

    std::string const c = "hi";
    passR(c); // 没问题： T 被推导为 std::string const

从上面的例子可以看出，转发引用可以保留传递实参的值语义，同时也保留了实参的常量属性。但是请注意，
当传递左值，模板参数会被推导为左值引用，在函数模板内部使用模板参数声明局部变量却未初始化时，会
导致编译错误。

.. code-block:: c++

    template<typename T>
    void passR(T&& arg) {
        T x; // 当传递一个左值时， x 的型别是左值引用，需要进行初始化
    }

使用 std::ref() 和 std::cref()
======================================

从 C++11 开始，你可以让调用方决定是按值传递还是按引用传递。当声明模板声明为按值传递时，调用方
可以使用 std::ref() 和 std::cref() 来按引用传递实参。

.. code-block:: c++

    template<typename T>
    void printT(T arr);

    std::string s = "hello";
    printT(s); // 按值传递
    printT(std::cref(s)); // 就像按引用传递一样

std::ref() 和 std::cref() 返回 std::reference_wrapper<> 型别的对象，它内部存储了原始
对象的指针，同时提供了到原始型别的隐式转换。因此原始型别支持的操作，reference_wrapper 也可以
支持。

注意，将 reference_wrapper 传递给泛型代码时，不会发生型别转换（模板参数推导不会进行隐式转换），
因此 reference_wrapper 包裹的对象的型别支持的操作在该泛型代码中不支持。

.. code-block:: c++

    template<typename T>
    class D {};

    void foo(D<int>& l, D<int>& r) {
        std::cout << "match D<int>\n";
    }

    template<typename T>
    void foo(D<T>& l, D<T>& r) {
        std::cout << "match D<T>\n";
    }

    template<typename T>
    void dispatch(std::)

    D<int> d1;
    // 匹配第一个普通函数, reference_wrapper<D<int>> 隐式转换为 D<int>
    foo(std::ref(d1), std::ref(d1)); 

    D<float> d2;
    // 错误，无法推导 T 的型别（不会发生隐式转换之后再进行型别推导）
    foo(std::ref(d2), std::ref(d2)); 

    // 没问题：显示指定模板实参，禁止模板实参推导
    foo<float>(std::ref(d2), std::ref(d2));

    template<typename T>
    void dispatch(std::reference_wrapper<T> l, std::reference_wrapper<T> r) {
        foo(l.get(), r.get());
    }

    // 对 reference_wrapper 包一层，显示使用底层型别
    dispatch(std::ref(d2), std::ref(d2)); 

那么，你现在是否能够解释以下代码。

.. code-block:: c++

    template<typename T1, typename T2>
    bool isLess(T1 arg1, T2 arg2) {
        return arg1 < arg2;
    }

    std::string s = "hello";
    isLess(std::ref(s), std::string("world")); // 错误

    template<typename T>
    class X {
    public:

        friend bool operator<(X &l, X &r) {
            return true;
        }
    };

    X<int> a;
    X<int> b;
    isLess(std::ref(a), b); // 没问题

    template<typename T>
    class X {
    public:

        template<typename U>
        friend bool operator<(X<U> &l, X<U> &r) {
            return true;
        }
    };

    X<int> a;
    X<int> b;
    isLess(std::ref(a), b); // 错误

处理字符串字面量与原始数组
=============================

到目前为止，我们已经看到当使用字符串字面量和原始数组时对模板参数产生的不同效果。

- 按值传递会发生退化，因此他们会退化成指向原始型别的指针
- 按引用传递不会发生退化，实参的型别变成数组的引用

每种方式都有优缺点。数组退化成指针时，你将无法区分传递的实参时指针型别还是数组型别；如果不发生退化，
不同大小的字符串字面量将会被认为是不同的型别。

.. code-block:: c++

    template<typename T>
    void foo(T const& arg1, T const& arg2);

    foo("hi", "guy"); // 编译错误

    template<typename T>
    void foo(T arg1, T arg2) {
        if(ar1 == arg2) { 
            ...
        }
    }

    foo("hi", "guy"); // 指针的比较，潜藏的bug

字符串字面量和原始数组的特殊实现
---------------------------------

你可能想在你的实现中区分传递的实参型别是指针还是数组，这必然要求传递的数组实参不会发生退化。为了区分这些情况，
你不得不检测是否传递的是数组。总的来说，有以下两种选择：

- 将模板参数声明为仅对数组型别有效

    .. code-block:: c++

        template<typename T, std::size_t L1, std::size_t L2>
        void foo(T (&args)[L1], T (&arg2)[L2]) {
            T* pa = arg1;
            T* pb = arg2;
            if(compareArrays(pa, L1, pb, L2)) {
                ...
            }
        }

    你可能需要多个实现来支持不同的数组声明格式。

- 你可以使用 type traits 来检测传递的实参是否为数组

    .. code-block:: c++

        template<typename T, typename = std::enable_if_t<std::is_array_v<T>>>
        void foo(T&& arg1, T&& arg2) {
            ...
        }

处理返回值
==============

对于返回值，你也会在按值返回和按引用返回之间做决定。然而，返回引用可能是麻烦的来源，因为你引用了一个不在你控制范围之内的东西。
在一些情况下，返回引用是常见的编程实践。

- 返回容器或者 string 中的元素（比如通过 operator[] 或者 front()）
- 授予对类成员的写访问
- 返回对象实现链式调用（strems 的 operator<< 和 operator>> 以及类对象的 operator=）

此外，返回常量引用来允许对类成员的读访问也是比较常用的。

注意，如果没用恰当使用的话，上述情况也可能会招致麻烦。

.. code-block:: c++

    auto s = std::make_shared<std::string>("hello");
    auto& c = (*s)[0];
    s.reset();
    std::cout << c; // 悬挂引用，会导致运行时错误

因此，我们应该确保函数模板通过传值的方式返回结果。然而，使用模板参数 T 并不会确保它不是一个引用，它可能被隐式推导为引用型别。

.. code-block:: c++

    template<typename T>
    T retR(T&& p) {
        return T{...}; // 传递左值时会导致按引用传递 
    }

即使 T 是通过按值传递推导出来的模板参数，当显示指定模板参数为引用型别时，它也有可能是引用型别。

    .. code-block:: c++

        template<typename T>
        T retV(T p) {
            return T{...};
        }

    int x;
    retV<int&>(x); // T 被实例化为 int&

为了安全，你有以下两个选择：

- 使用 type traits std::remove_reference<> 将 T 转换为非引用型别

    .. code-block:: c++

        template<typename T>
        typename std::remove_reference<T>::type retV(T p) {
            return T{...};
        }

- 让编译器自动推导返回型别，只需将返回型别声明为 auto （从C++14开始），因为 auto 会导致退化

    .. code-block:: c++

        template<typename T>
        auto retV(T p) {
            return T{...};
        }

推荐的模板参数声明
===================      

我们有不同的方式声明依赖于模板参数的函数形参:

- 声明按值传递实参

    这个方法简单，它会导致字符串字面和原始数组发生退化，但它不会为大型对象提供最好的性能。不过调用者还是可以通过
    std::ref() 和 std::cref() 来按引用传递实参,但是调用这个必须小心这样做是合法的。

- 声明按引用传递实参

    这个方法通常为有点大的对象提供了更好的性能，尤其当传递

        * 已存在的对象（左值）给左值引用
        * 临时对象（prvalue）或者标记为可移动的对象（xvalue）给右值引用
        * 或者两者都给转发引用
    
    因为按引用传递时不会发生退化，当处理字符串字面量和数组时你需要小心。对于转发引用， 你不得不当心使用这个方法模板参数会被隐式
    推导为引用型别。

一般性建议
-------------

对于函数模板，我们推荐以下几点：

1. 通常情况下，将参数声明为按值传递。这个方法简单并且常常有用，即使对于字符串字面量也有用。对于小对象和临时对象以及可移动对象，它
的性能比较好。调用者也可以使用 std::ref() 和 std::cref() 来传递大对象来避免昂贵的拷贝。

2. 如果有更好的理由，可以选择其他方式：

    - 如果你需要 out/inout 参数，按非常量引用传递实参。不过，你可能要考虑禁用意外地接受常量对象。
    - 如果模板是用来转发实参的，使用转发引用。使用 std::decay<> 或者 std::common_type<> 协调不同型别的字符串字面量和原始数组
    - 如果性能是关键因素，并且拷贝实参非常昂贵，使用常量引用。

3. 不要对性能做直觉上的假设，进行实际的度量。

不要过度泛型
----------------

通常，函数模板并不是使用任意型别的实参。相反，会对实参型别有一定的约束。比如，你知道只有某些型别的 vector 才允许被传递。在这种情况下，
应该使用以下方式声明模板：

.. code-block:: c++

    template<typename T>
    void printVector(std::vector<T> const& v);

std::make_pair 的例子

std::make_pair<>() 是一个演示决定参数传递机制陷阱的好例子。它的声明随着不同的 C++ 版本而改变。

- 第一个 C++ 标准， C++98， std::make_pair<>() 被声明为按引用传递来避免不必要的拷贝：

    .. code-block:: c++

        template<typename T1, typename T2>
        pair<T1, T2> make_pair(T1 const& a, T2 const& b) 
        {
            return pair<T1, T2>(a, b);
        }

传递字符串字面量和原始数组会导致重大的问题（不能使用数组对象初始化数组对象）。

- 结果，C++03标准时，该函数模板被声明为按值传递：

    .. code-block:: c++

        template<typename T1, typename T2>
        pair<T1, T2> make_pair(T1 a, T2 b) 
        {
            return pair<T1, T2>(a, b);
        }    

- 不过， C++11标准时，为了支持移动语义，实参型别变成了转发引用：

    .. code-block:: c++

        template<typename T1, typename T2>
        constexpr pair<typename decay<T1>::type, typename decay<T2>::type> 
        make_pair(T1&& a, T2&& b) 
        {
            return pair<typename decay<T1>::type, typename decay<T2>::type>(forward(a), forward(b));
        }    

C++ 标准库现在在许多地方使用相似的方式使用转发引用传递实参，通常和 std::decay<> 组合使用。
