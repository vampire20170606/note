泛型库
#############

Callables
================

在C++中有几种工作的很好的回调类型，因为它们都能被作为函数调用参数传递，并且能够直接使用 f(...) 语法调用：

- 函数指针
- 重载 operator() 的类类型（有时称为仿函数），包括 lambda 
- 拥有转换成函数指针或者函数引用的转换函数的类类型

这些类型共同地被称为 **函数对象类型** ，拥有该类型的值被称为 **函数对象** 。

C++标准库引入了稍微宽泛一点的概念， **可调用类型** ，包括函数对象类型和成员指针。可调用类型的对象被称为
**可调用对象** ，为了方便起见，我们称之为 **callable** 。

支持函数对象
---------------

.. code-block:: c++

    template<typename Iter, typename Callable>
    void foreach(Iter current, Iter end, Callable op) {
        while(current != end) {
            op(*current); // 对迭代的每个元素调用 op
            ++current;
        }
    }

    void func(int i) {
        std::cout << "func() called for: " << i << "\n";
    }

    class FuncObj {
    public:
        void operator() (int i) const {
            std::cout << "FuncObj::op() called for: " << i << "\n";
        }
    };

    int main()
    {
        std::vector<int> primes = {2, 3, 5, 7, 11, 13, 17, 19};

        foreach(primes.begin(), primes.end(), func); // func 退化为函数指针
        foreach(primes.begin(), primes.end(), &func); // 显示传递函数指针
        foreach(primes.begin(), primes.end(), FuncObj()); // 传递函数对象

        foreach(primes.begin(), primes.end(), 
                [](int i) {
                    std::cout << "lambda called for: " << i << "\n";
                }); // 传递 lambda 
    }

- 当传递函数名称作为函数实参时，实际上，并不是传递函数本身，而是函数的指针或者引用

- 我们也可以显示地通过取地址操作符传递函数指针

- 传递仿函数时，需要传递重载了  operator() 的类类型的对象

    .. code-block:: c++

        op(*current);

        // 等价于
        op.operator()(*current);
    
- 传递 **代理函数** ，重载了转换操作符的类类型

    .. code-block:: c++

        op(*current);

        // 等价于
        (op.operator F())(*current); // F 表示函数指针或者函数引用

- 传递 lambda 表达式，本质和仿函数等价（不带捕获的 lambda 表达式可以隐式转换为函数指针）

处理成员函数和附加实参
-------------------------

从C++17开始，C++标准库提供了 **std::invoke()** 来统一成员方法调用和普通函数调用语法，因此，调用可调用对象对象统一为单一的
形式。

.. code-block:: c++

    template<typename Iter, typename Callable, typename... Args>
    void foreach(Iter current, Iter end, Callable op, Args... args)
    {
        while(current != end) {
            std::invoke(op, // 可调用对象
                args...,  // 附件实参
                *current); // 当前元素
            ++current;
        }
    }

    class MyClass {
    public:
        void memfunc(int i) {
            std::cout << "MyClass::memfunc() called for: " << i << '\n';
        }
    };

    int main()
    {
        std::vector<int> primes = {2, 3, 5, 7, 11, 13, 17, 19};

        foreach(primes.begin(), primes.end(), [](std::string const& prefix, int) {
            std::cout << prefix << i << '\n';
        }, "- value: "); // 附加实参

        MyClass obj;
        foreach(primes.begin(), primes.end(), 
            &MyClass::memfunc, //成员方法
            obj); // this 指针
    }

- 如果可调用对象是成员指针，它使用第一个附加实参作为 this 指针，余下的附加实参作为实参传递给可调用对象

- 除此之外，所有的附加参数作为实参传递给可调用对象

.. note:: 

    可调用对象和附加参数不要使用转发引用，否则在第一次调用时，可调用对象和附加参数已经被移到，从而导致第二次调用时会引发意外行为

包装函数调用
------------------

std::invoke 的一个常见应用是包装一个函数调用（比如，打印调用日志，度量调用的持续时间，准备上下文比如为调用启动一个线程）。

.. code-block:: c++

    template<typename Callable, typename... Args>
    decltype(auto) call(Callable&& op, Args&&... args)
    {
        // 由于可调用对象和实参仅仅是作为调用转发，并且不会多次使用，因而可以使用转发引用
        return std::invoke(std::forward<Callable>(op),
            std::forward<Args>(args)...);
    }

如果你想临时把 std::invoke 返回的值存储到一个变量，然后做完其他事情之后返回该值。你也被使用 decltype(auto) 声明临时变量
的类型：

.. code-block:: c++

    decltype(auto) ret{std::invoke(std::forward<Callable>(op),
            std::forward<Args>(args)...)};
    
    ...

    return ret;

使用 decltype(auto) 还是会有问题：如果调用返回类型为 void ，初始化 ret 为 decltype(auto) 是不被允许的，因为 void 是不完整类型。
你有以下选择：

- 在返回语句之前声明一个对象，在对象的析构函数中执行期望的操作

    .. code-block:: c++

        struct cleanup {
            ~cleanup() {
                ... // 在返回之前需要执行的操作
            }
        } dummy;
    
        return std::invoke(std::forward<Callable>(op),
            std::forward<Args>(args)...);

- 对 void 和 非 void 采取不同的实现

    .. code-block:: c++

        template<typename Callable, typename... Args>
        decltype(auto) call(Callable&& op, Args&&... args) 
        {
            if constexpr(std::is_same_v<
                std::invoke_result_t<Callable, Args...>, void>) {
                // 返回类型为 void
                std::invoke(std::forward<Callable>(op),
                    std::forward<Args>(args)...);   
                
                ...

                return;
            } else {
                // 返回类型不是 void    
                decltype(auto) ret{std::invoke(std::forward<Callable>(op),
                    std::forward<Args>(args)...)};
    
                ...

                return ret;
            }
        }

实现泛型库的其他实用程序
===============================

std::invoke 只是C++标准库提供的用于实现泛型库的一个有用的实用程序。以下介绍其他的实用程序。

Type Traits
---------------

标准库提供了各种各样的使用程序，它们允许我们评估或者修改类型，这被称作 **type trait** 。它支持各种需要适应实例化类型或者对
实例化类型作出反应的泛型代码。

.. code-block:: c++

    template<typename T>
    class C {

        // 移除 cv 修饰词之后不是 void
        static_assert(!std::is_same_v<std::remove_cv_t<T>, void>, 
            "invalid instantiation of class C for void type");

    public:
        template<typename U>
        void f(V&& v) {
            if constexpr(std::is_reference_v<T>) {
                ... // T 是引用时的处理逻辑
            } 

            if constexpr(std::is_convertible_v<std::decay_t<V>, T>) {
                ... // 如果 V 可以转换为 T 时的处理逻辑
            }

            if constexpr(std::has_virtual_destructor_v<V>) {
                ... // 如果 T 有虚析构时的处理逻辑
            }
        }
    };

std::address_of()
------------------------

std::address_of() 函数模板产生一个对象或者函数的实际地址。即使对象类型重载了 operator& 也能正常工作。

.. code-block:: c++

    template<typename T>
    void f(T&& x)
    {
        auto p = &x; // 如果重载了 operator& 可能会失败
        auto q = std::address_of(x); // 即使重载了 operator& 也能工作
        ...
    }

std::declval()
-----------------------

std::declval<>() 函数模板能被用来作为特定类型的对象引用的占位符。该函数模板没有定义，所以不能被调用。因此，它仅仅能够
被用在不需要求值的表达式的操作数（比如 sizeof 和 decltype 构造）。你可以假设拥有特定类型的对象，而不是创建一个对象。

.. code-block:: c++

    template<typename T1, typename T2,
            typename RT = std::decay_t<decltype(true ? std::declval<T1>() : 
                                                    std::declval<T2>())>>
    RT max(T1 a, T2 b)
    {
        return b < a ? a : b;
    }  

.. note:: 

    std::declval<>() 返回引用型别，左值引用或者右值引用，根据引用折叠规则进行推断

完美转发临时对象
====================

对于函数参数的完美转发可以通过将参数声明为转发引用型别，然后调用 std::forward<>() 。我们也可以对临时变量进行完美转发，使用
auto&& 创建待转发的临时变量，然后调用 std::forward<>() 。

.. code-block:: c++

    template<typename T>
    void foo(T x)
    {
        auto&& val = get(x);
        
        ...

        // 完美转发 get() 的返回值给 set()
        set(std::forward<decltype(val)>(val));
    }

引用作为模板参数
====================

.. code-block:: c++

    template<typename T, T Z = T{}>
    class RefMem {
    private:
        T zero;
    public:
        RefMem() : zero{Z} {}
    };

    int null = 0;

    int main()
    {
        RefMem<int> rm1, rm2;
        rm1 = rm2; // 没问题

        RefMem<int&> rm3; // 错误：Z 的默认值无效
        RefMem<int&, 0> rm4; // 错误：Z 的实参无效

        extern int null;
        RefMem<int&, null> rm5, rm6;
        rm5 = rm6; // 错误： operator= 由于存在引用成员而被删除
    }

当使用引用类型显示实例化模板时，事情会变得相当棘手：

- 默认初始化不再工作

- 你不能再仅仅通过传递 0 来初始化

- 最令人吃惊的是，赋值操作符不再可用，因为拥有非静态引用成员的类的默认赋值操作符是被删除的

非类型模板参数使用引用类型也是很棘手的，并且很危险：

.. code-block:: c++

    template<typename T, int& SZ>
    class Arr {
    private:
        std::vector<T> elems;
    public:
        Arr() : elems(SZ) { } // 使用当前的 SZ 初始化 vector

        void print() const {
            for(int i = 0; i < SZ; ++i) {
                std::cout << elems[i] << ' ';
            }
        }
    };

    int size = 10;

    int main()
    {
        Arr<int&, size> y; // 错误，std::vector 元素类型不可以是引用类型

        Arr<int, size> x; // vector 初始化为 10 个元素
        x.print(); // 没问题

        size += 100;
        x.print(); // 运行时错误
    }

由于这个原因，C++标准有时会有令人吃惊的规范和约束：

- 为了在模板参数被实例化为引用类型时，赋值操作符仍然有效， std::pair<>  和 std::tuple<> 会实现赋值
  操作符，而不是使用默认行为

    .. code-block:: c++

        namespace std {
            template<typename T1, typename T2>
            struct pair {
                T1 first;
                T2 second;

                ...

                // 默认拷贝/移动构造对引用成员仍然有效
                pair(pair const&) = default;
                pair(pair&&) = default;

                ...
                // 赋值操作符对于引用类型必须显示定义
                pair& operator=(pair const& p);
                pair& operator=(pair&& p) noexcept(...);
            };
        }

- 由于可能的副作用的复杂性，使用引用类型实例化 C++17 标准库中的类模板 std::optional<> 和 std::variant<> 是不合规范的

一般而言，引用类型与其他类型很不一样，并且有几个独特的语言规则。这会对调用参数的声明以及定义 type traits 的方式产生巨大影响。

延迟评估
===============

当实现模板时，有时问题是能否处理不完整类型。

.. code-block:: c++

    template<typename T>
    class Cont {
    private:
        T* elems;
    public:
        ...
    };

到目前为止， Cont 能处理不完整类型。对于类引用自身类型作为元素类型是非常有用的：

.. code-block:: c++

    struct Node {
        std::string value;
        Cont<Node> next; // 仅在 Cont 接受不完整类型时才有效
    };

然而，仅仅使用某些 traits ，你可能会失去处理不完整类型的能力。

.. code-block:: c++

    template<typename T>
    class Cont {
    private:
        T* elems;
    
    public:
        ...

        typename std::conditional<std::is_move_constructible<T>::value, T&&, T&>::type
        foo();
    };

std::is_move_constructible 要求模板实参是完整类型，使用 struct Node 实例化时会失败。

我们可以通过将 foo() 替换成成员模板来处理这个问题，这样 std::is_move_constructible<> 的评估被延迟到
foo() 的实例化点：

.. code-block:: c++

    template<typename T>
    class Cont {
    private:
        T* elems;
    
    public:
        ...

        template<typename D = T>
        typename std::conditional<std::is_move_constructible<D>::value, D&&, D&>::type
        foo();
    };

编写泛型库时要考虑的事情
=============================

让我们列出一些在实现泛型库时要考虑的事情：

- 在模板中使用转发引用转发值。如果值不依赖与模板参数，使用 auto&&

- 当参数被声明为转发引用时，准备好当传递左值时，模板参数拥有引用类型

- 当需要依赖于模板参数的对象的地址时，使用 std::address_of ，这样可以避免当绑定的类型重载 operator& 出现令你吃惊的行为

- 对于成员函数模板，确保它们不会比预定义的拷贝/移动构造或者赋值操作符更匹配

- 当模板参数可能是字符串字面量并且不是按值传递时，考虑使用 std::decay

- 如果你有 out 或者 inout 参数易拉与模板参数，准备好处理 const 模板实参可能会被指定的情形

- 准备好处理模板参数被指定为引用类型时产生的副作用。特别地，你可能想确保返回类型不会变成引用

- 准备好处理不完整类型来支持递归数据结构

- 重载所有的数组类型，而不仅仅是 T[SZ]
