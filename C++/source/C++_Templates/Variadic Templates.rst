可变参数模板
#################

从 C++11 开始，模板可以接受可变数量的模板实参。这个特性允许你在需要使用任意数量
任意类型的地方使用模板。

可变参数模板
=====================

模板参数能够被定义为接受不限数量的模板实参。拥有这个能力的模板被称作 **可变参数模板** 。

可变参数模板实例
---------------------

例如，你可以使用不同型别的任意数量的实参调用 print ：

.. code-block:: c++
    :linenos:

    #include<iostream>

    void print() {}

    template<typename T, typename... Types>
    void print(T firstArg, Types... args)
    {
        std::cout << firstArg << '\n';
        print(args...);
    }

    int main()
    {
        print(6.9, 10, "10");
    }

其中 ``typename... Types`` 被称作**模板参数包** ， **Types... args** 中的 **args** 被称作**函数参数包**。
**print(args...)** 中的 **args...** 被称作**解参数包**。

    print(6.9, 10, "10");

的工作过程如下：

1. 该调用被扩展成 print<double, int, char const*>(6.9, 10, "10")

2. **firstArg** 拥有值 6.9，因此 T 被推导为 double 型别，而 args 则包含了10 和
"10" 这两个值，再次调用 print 时扩展为 print<int, char const*>print(10, "10")

3. **firstArg** 拥有值 10，因此 T 被推导为 int 型别，而 args 则只剩 "10" ，再次调用
print 时扩展为 print<char const*>print("10")

4. **firstArg** 拥有值 "10"，因此 T 被推导为 char const* 型别，而 args 为空，再次调用
print 时导致调用非模板没有参数的 print 。

重载可变参数和非可变参数模板
-------------------------------

如果两个函数模板的区别仅在尾部的可变参数包，那么不带尾部参数包的函数模板优先匹配。

.. code-block:: c++
    :linenos:

    #include<iostream>

    template<typename T>
    void print(T arg) 
    {
        std::cout << arg << "\n";
    }

    template<typename T, typename... Types>
    void print(T firstArg, Types... args)
    {
        print(firstArg); // 调用非变参模板
        print(args...); // 对余下的参数调用print，当参数数量大于1时会递归调用本身，否则调用非变参模板
    }

    int main()
    {
        print(6.9, 10, "10");
    }

操作符 sizeof...
--------------------------

C++11 为变参模板引入了一个新形式的 sizeof 操作符： ``sizeof...`` ，它扩展成一个参数包所包含
的元素的数量。

.. code-block:: c++
    :linenos:

    template<typename T, typename... Types>
    void print(T firstArg, Types... args)
    {
        std::cout << sizeof...(Types) << "\n";
        std::cout << sizeof...(args) << "\n";
    }

从上面的例子可以看出， sizeof... 操作符可以应用到模板参数包和函数参数包。

以下的例子是不能按你的预期工作的，因为实例化是编译时的，而 if 是运行时的。

.. code-block:: c++
    :linenos:

    template<typename T, typename... Types>
    void print(T firstArg, Types... args)
    {
        std::cout << firstArg<< "\n";
        if(sizeof...(args) > 0) { // 运行时 if 在编译时不可判定，所以两个分支都会被实例化
            print(args...);
        }
    }

折叠表达式
==================

从 C++17 开始，引入了一个新特性用于计算参数包在某个二元运算符下的计算结果（可以带一个初始值）。

.. code-block:: c++
    
    template<typename... T>
    auto sum(T... args)
    {
        // 等价于 ((s1 + s2) + s3) ...
        return (... + args); // 必须使用括号，否则会被认为是一个参数解包
    }

    // 与上述折叠表达式等价的基于函数模板重载实现
    template<typename T>
    auto sum(T arg)
    {
        return arg;
    }

    template<typename T, typename... Types>
    auto sum(T arg1, T arg2, Types... args)
    {
        return sum((arg1 + arg2), args...);
    }

如果参数包为空的话，折叠表达式通常是不规范的。

.. csv-table:: 折叠表达式（从C++17起）
   :header: "折叠表达式", "Evaluation"
   :widths: 50,  60

   "( ... op pack)", "(((pack1 op pack2) op pack3) ...op packN)"
   "( pack op ...)", "(pack1 op (... (packN-1 op packN)))"
   "(init op ... op pack)", "((((init op pack1) op pack2) op pack3) ...op packN)"
   "( pack op ... op init )",  "(pack1 op (... (packN-1 op (packN op init))))"

.. note:: 

    ``operator&&`` 在输入空参数包时返回 ``true``；
    ``operator||`` 在输入空参数包时返回 ``false``；
    ``operator,`` 在输入空参数包时返回 ``void()``。

.. code-block:: c++

    template<typename... Types>
    auto foo(Types... args)
    {
        return (... && args); // 返回 true
    }

    template<typename... Types>
    auto foo(Types... args)
    {
        return (... || args); // 返回 false
    }

    template<typename... Types>
    auto foo(Types... args)
    {
        return (... , args); // 返回 void()
    }

下面的例子使用 ``(np ->* ... ->* paths)`` 从 np 开始按照 paths 指定的路径进行遍历。

.. literalinclude:: codes/basics/foldtraverse.cpp
    :language: c++
    :linenos:

同样，我们可以利用折叠表达式来简化打印变参模板元素的实现：

.. code-block:: c++

    template<typename... Types>
    void print(Types const&... args)
    {
        (std::cout << ... << args) << "\n";
    }

可变参数模板的应用
====================

变参模板在实现泛型库的时候扮演着重要角色，比如C++标准库。比如：

* 传递参数构造 shared_ptr

    auto sp = std::make_shared<std::complex<float>>(4.2, 7.5);

* 传递参数给线程

    std::thread t(foo, 42, "hello");

* 传递参数来构造一个新的元素插入 vector 中

    std::vector<Customer> v;
    v.emplace_back("tim", "jone", 1962);

通常，这些参数都使用万能引用来进行完美转发：

.. code-block:: c++

    namespace std {
        template<typename T, typename... Args>
        shared_ptr<T> make_shared(Args&&... args);
        
        class thread {
            public:
                template<typename F, typename... Args>
                explicit thread(F&& f, Args&&... args);
                ....
        };

        template<typename T, typename Allocator = allocator<T>>
        class vector {
            public:
                template<typename... Args>
        }
    }

.. note:: 

    变参函数模板参数和普通参数一样，当进行值传递的时候回返回退化。

可变参数类模板和可变参数表达式
=================================

可变参数表达式
----------------

可变参数表达式就相当于对可变参数包中的每个元素应用同一操作，可以是二元操作，也可以是一元
操作，还可以是编译时表达式。

.. code-block:: c++
    :linenos:

    template<typename... Types>
    void printElement(Types const&... args)
    {
        (std::cout << ... << args) << "\n"; // C++17 的折叠表达式
    }

    template<typename T = int, typename... Types>
    void print(Types... args)
    {
        // C++11 支持
        // 使用一元表达式
        printElement(!args...); 
        printElement(-args...);

        // 使用二元表达式
        printElement(args + args...);
        printElement(args * args...);

        printElement(args + 1...); // 错误， 字面量和省略号之间要有界限符号
        printElement(args + 1 ...); // 没问题
        printElement((args + 1) ...); // 没问题
        printElement(1 + args...); // 没问题

        // C++17 支持，使用了折叠表达式
        std::cout << std::boolalpha << (... && std::is_same<T, Types>::value ) << "\n";
    }

可变参数索引
--------------

下面的例子通过可变数量的索引来访问容器中的相关元素。

.. code-block:: c++

    template<typename C, typename... Idx>
    void printElem(C const& con, Idx... idx)
    {
        printElement(con[idx]...);  // con[idx] 本质上还是一个参数包
    }

    std::vector<std::string> col = {"hello", "world", "everyont"};
    printElem(col, 2, 0, 1);

    template<int... Idx, typename... C>
    void printElem(C const&... con)
    {
        print11(con[Idx]...); 
    }

    std::vector<int> a1 {1, 20};
    std::vector<std::string> a2 {"1", "20", "hello"};
    printElem<1, 2>(a1, a2);

可变参数类模板
-------------------

可变参数也可以用在类模板上。

.. code-block:: c++

    template<typename... Elements>
    class Tuple;

    Tuple<int, char, std::string> t; // 异构容器

    template<std::size_t... Index>
    class Indices;

    template<typename T, std::size_t... Idx>
    void printIdx(T t, Indices<Idx...>)
    {
        print(std::get<Idx>(t)...);
    }

    auto t = std::make_tuple(12, "hello", 0.9);
    printIdx(t, Indices<0, 1, 2>());

变参推导指引
--------------

以下是 std::array 推导指引，对于 ``std::array a{10, 11, 23};``， 首先推导出
T 为 第一个元素的型别， U... 为余下的元素的型别，然后通过 enable_if_t 来约束所有
的型别应该一致。所以上述初始化推导出的型别为 ``std::array<int, 3> a{10, 11, 23}`` 。

.. code-block:: c++

    namespace std {
        template<typename T, typename... U> array(T, U...)
            -> array<enable_if_t<(is_same_v<T, U> && ...), T>, 
                (1 + sizeof...(U))>
    }

可变参数基类和using
----------------------

从下面的例子可知，我们可以派生自可变数量的基类，同时还可以使用using指令引入基类参数包中的声明。

.. code-block:: c++

    class Customer {
    private:
        std::string name;

    public:
        Customer(std::string const& n) : name(n) {}
        std::string getName() const { return name; }
    };

    struct CustomerEq {
        bool operator() (Customer const& lhs, Customer const& rhs) {
            return lhs.getName() == rhs.getName();
        }
    };

    struct CustomerHash {
        bool operator() (Customer const& c) {
            return std::hash<std::string>()(c.getName());
        }
    };

    template<typename... Bases>
    struct Overloader : Bases...
    {
        using Bases::operator()... ; // Ok since C++17
    };

    // using 不会导致发生实例化
    using CustomerOP = Overloader<CustomerHash, CustomerEq>;
