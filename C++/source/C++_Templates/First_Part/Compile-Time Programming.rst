编译时编程
################

实际上，C++ 拥有多个特性来支持编译时编程：

- C++98 之前，模板就提供了编译时计算的能力，包括使用循环（模板递归）和执行路径选择（模板特化与偏特化）

- 使用偏特化，我们能够在编译时选择依赖于特定约束和需求的类模板实现

- 使用 SFINAE 原则，我们可以选择不同型别或者不同约束的函数模板

- C++11 和 C++14 标准中，编译时计算通过 constexpr 特性得到更好的支持，可以更加直观的进行执行路径选择

- C++17 引入了 ``compile-time if`` , 通过编译时条件或者约束来丢弃语句。即使在非模板代码也有用

模板元编程
===============

模板是在编译时实例化。结果发现 C++ 模板的一些特性与实例化过程结合可以产生一种原始的递归”编程语言“。为此，
模板可以用来”计算一个程序“。

.. code-block:: c++
    :linenos:

    template<unsigned p, unsigned d>
    struct DoIsPrime {
        static constexpr bool value = (p % d != 0) && DoIsPrime<p, d - 1>::value;
    };

    template<unsigned p>
    struct DoIsPrime<p, 2> { // 终止递归
        static constexpr bool value = (p % d != 0);
    };

    template<unsigned p>
    struct IsPrime P {
        static constexpr bool value = DoIsPrime<p, p/2>::value;
    };

    // 避免无限递归
    template<>
    struct IsPrime<0> {
        static constexpr bool value = false;
    };

    template<>
    struct IsPrime<1> {
        static constexpr bool value = false;
    };

    template<>
    struct IsPrime<2> {
        static constexpr bool value = true;
    };

    template<>
    struct IsPrime<3> {
        static constexpr bool value = true;
    };

IsPrime 类模板可以在编译时判断一个数是不是质数。

使用 constexpr 计算
========================

C++11 引入了一个新特性， ``constexpr`` ，这极大地简化了不同形式的编译时计算。特别地，给定合适的输入，
一个 constexpr 函数可以在编译时被评估。然而在 C++11 constexpr 函数有着严格的限制（比如，每个 constexpr
函数的定义被限制为一条 return 语句），大多数的限制在 C++14 被移除。当然，成功地评估一个 constexpr 函数仍然
需要所有的可计算步骤在编译时是可行的和有效的：目前，堆内存分配和抛异常是不能在编译时评估的。

.. code-block:: c++

    constexpr bool doIsPrime(unsigned p, unsigned d) {
        return d != 2 ? (p % d != 0) && doIsPrime(p, d-1) : (p % 2 != 0);
    }

    constexpr bool isPrime(unsigned p) {
        return p < 4 > !(p < 2) : doIsPrime(p, p/2);
    }

由于限制只能使用一条语句，我们只能通过条件操作符作为选择机制，并且我们仍需要递归来迭代所有的元素。但是语法则是普通的
函数代码，但使得比第一个依赖模板实例化的版本更加直观和容易。

C++14 标准之后， constexpr 函数可以利用大多数的控制结构。因此我们可以使用普通的 for 循环。

.. code-block:: c++

    constexpr bool isPrime(unsigned p, unsigned d) {
        for(unsigned d = 2; d <= p/2; ++d) {
            if(p % d == 0) {
                return false;
            }
        }
        return p > 1;
    }

.. code-block:: c++

    constexpr bool b1 = isPrime(9); // 编译评估
    const bool b2 = isPrime(9); // 如果是全局变量，则在编译时评估；在块作用域类，编译可以决定是否在编译时还是运行时评估

    bool fiftySevenIsPrime() {
        return isPrime(57); // 编译器决定编译时还是运行时评估
    }

    int x;
    ...
    std::cout << isPrime(x); //运行时评估

使用偏特化进行执行路径选择
==========================

像 isPrimme() 这样的编译时测试的一个有趣的应用就是使用偏特化在编译时选择不同的实现。


例如，我们可以根据一个模板实参是否为素数来选择不同的实现：

.. code-block:: c++

    // 主模板
    template<std::size_t SZ, bool = isPrime(SZ)>
    struct Helper;

    // 如果 SZ 不是素数时的实现
    template<std::size_t SZ>
    struct Helper<SZ, false>
    {
        ...
    };

    // 如果 SZ 是素数时的实现
    template<std::size_t SZ>
    struct Helper<SZ, true>
    {
        ...
    };

    template<typename T, std::size_t SZ>
    long foo(std::array<T, SZ> const& coll)
    {
        Helper<SZ> h; // 具体是哪个实现取决于 SZ 是否为素数
        ...
    };

由于函数模板不支持偏特化，你不得不选择其它的机制来基于某个约束改变函数的实现。我们可以有以下
几个选择：

- 使用类的静态方法

- 使用 std::enable_if

- 使用 SFINAE 特性

- 使用编译时 if 特性（C++17）

SFINAE（Substitution Failure Is Not An Error，替换失败并不是一种错误）
=============================================================================

在 C++ 中，通过不同的实参型别来重载函数已经很常见。当编译器看到一个被重载的函数时，它必须单独地考虑
每个候选者。评估调用的实参并且选择一个最匹配的候选者。

当一个调用的候选者中存在函数模板时，编译器首先不得不确定候选者的模板实参，然后使用模板实参替换函数参数列表
和返回型别，然后评估它的匹配度。不过，替换过程可能会遇到问题：它可能产生一个没有意义的构造。该无意义的替换
不会导致编译错误，相反，语言规则告诉我们这个候选者只是被简单地忽略掉而已。这条原则就被称为 **SFINAE** 。

.. code-block:: c++

    // 计算原始数组的元素数量
    template<typename T, unsigned N>
    std::size_t len(T(&)[N]) {
        return N;
    }

    // 类型内定义了 size_type 的对象的元素数量
    template<typename T>
    typename T::size_type len(T const& t) {
        return t.size();
    }

    // 未能匹配上述模板的型别时匹配该模板 
    std::size_t len(...) {
        return 0;
    }

    int a[10];
    std::cout << len(a); // 没问题：匹配第一个模板
    std::cout << len("hello"); // 没问题：匹配第一个模板

    std::vector<int> v;
    std::cout << len(v); //没问题：匹配第二个模板

    int* p;
    std::cout << len(p); // 没问题：匹配最后一个模板

    std::allocator<int> x;
    std::cout << len(x); // 错误：匹配了第二个模板，但是 allocator 对象没有 size() 方法

对于原始数组和字符串字面量，第一个模板时最匹配的。而第二个模板和第三个模板也是可以匹配的，对于第二个模板，此时
模板参数 T 被推导为 int[10] 和 char const[6]，使用这两个型别替换会导致返回型别 T::size_type 存在潜在的
错误，因此第二个函数模板会被忽略。而第三个模板因为没有第一个模板匹配度高，因此会从候选者中删除。

对于 std::vector<> 型别的对象，只有第二个和第三个模板匹配，但是第二个匹配度更高。

对于指针型别的对象，第二个和第三个模板匹配，但是第二个模板在进行替换时会导致返回型别无意义，因此会被忽略掉。

std::allocator<> 型别的对象匹配第二个和第三个模板，并且由于第二个模板匹配度更高，因而会选用第二个模板并实例化函数体，
由于实例化后的函数体中调用了一个未定义的成员方法 size()，因而会导致编译错误。

.. note:: 

    如果重载解析中的候选者中存在多个函数模板时，编译会对所有的函数模板候选者进行实例化，但是，此时并不会实例化函数的定义，
    仅仅实例化函数的声明，如果声明在实例化过程中导致函数的声明构造无意义的话，则会忽略该函数模板。所有函数模板都实例化之后，
    再从中选择匹配度更高的函数模板。

SFINAE 和重载解析
------------------------

SFINAE 可以用来实现在某个约束下忽略函数模板，因而可以利用该原则实现对函数实现的选择。 std::enable_if 利用了模板偏特化和
SFINAE 原则，因而可以实现对模板的禁用。

带有 decltype 的 SFINAE 表达式
=====================================

找出或者制定合适的表达式在某些条件下通过 SFINAE 原则忽略函数模板并不总是那么容易。

.. code-block:: c++

    template<typename T>
    typename T::size_type len(T const& t) {
        return t.size();
    }

对于拥有 size_type 成员但是没有 size() 成员方法的型别，函数模板 len() 最终实例化之后会导致编译错误。如果我们需要对模板参数
约束既要有 size_type 成员也要有 size() 成员方法，构造合适的表达式则没有那么容易。因而，在这种情况下，一般会有如下通用模式或者
惯用手法：

- 使用尾返回型别语法

- 使用 decltype 和逗号操作符定义返回型别

- 在逗号操作符之前制定所有需要合法的表达式（所有表达式转换为 void 避免逗号操作符被重载）

- 在逗号操作符最后定义实际返回型别的对象

.. code-block:: c++

    template<typename T>
    auto len(T const& t) -> decltype( (void)(t.size()), T::size_type() ) {
        return t.size();
    }    

编译时 if
===============

偏特化、SFINAE 和 std::enable_if 允许我们从整体上使能或者禁用模板。 C++17 另外引入了编译时 if 语句，它允许我们基于编译时
的条件使能或者禁止特定的语句。通过 ``if constexpr(...)`` 语法，编译器使用一个编译时表达式来决定是否采用 ``then`` 部分还是
``else`` 部分。

.. code-block:: c++

    template<typename T, typename... Types>
    void print(T const& firstArg, Types const&... args) {
        std::cout << firstArg << "\n";

        if constexpr(sizeof...(args) > 0) {
            print(args...); // 仅当 sizeof...(args) > 0 时可用
        }
    }

如果 ``sizeof...(args) > 0`` 时，此时会引发模板实例化；反之，递归调用 print() 的语句会被丢弃。请注意，即使是被丢弃的语句，也
应该确保通过模板的第一阶段检查，即检查语法是否正确，不依赖模板参数的名称也能被找到。

.. code-block:: c++

    template<typename T>
    void foo(T t)
    {
        if constexpr(std::is_integral_v<T>) {
            if(t > 0) {
                foo(t-1);
            }
        } else {
            undeclared(t); //错误如果该函数未声明的话
            unsigned(); // 错误如果该函数未声明的话
            static_assert(false, "no integral"); // 总是会导致 assert 错误，即使该语句被丢弃
            static_assert(!std::is_integral_v<T>, "no integral"); // 没问题
        }
    }

``if constexpr`` 能在任何函数中使用，而不仅仅只能在模板中使用。我们仅仅需要一个能产生一个布尔值得编译时表达式。

.. code-block:: c++

    int main()
    {
        if constexpr(std::numeric_limits<char>::is_signed) {
            foo(42); // 没问题
        } else {
            undeclared(42); //错误如果该函数未声明的话
            static_assert(false, "no integral"); // 总是会导致 assert 错误，即使该语句被丢弃
            static_assert(!std::numeric_limits<char>::is_signed, "char is unsigned"); // 没问题
        }
    }
