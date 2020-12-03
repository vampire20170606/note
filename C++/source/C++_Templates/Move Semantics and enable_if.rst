移动语义和enable_if<>
##########################

C++11 引入的最重要的特性之一就是**移动语义** 。这个特性可以优化对临时对象的重量级拷贝操作，使得
临时对象的资源可以被移到到目标对象之中。

移动语义对模板的设计有着重要的影响，并且引入了一些特殊的规则来支持泛型代码中的移动语义。

完美转发
=============

假设你写的泛型代码需要支持转发传递实参的基础性质：

- 可修改的对象在转发之后依旧可以修改
- 常量对象在转发之后依旧是只读对象
- 可移动对象在转发之后依旧是可移动对象

如果不使用模板实现上述功能，我们必须对上述三种情况进行编程。

.. code-block:: c++

    #include<iostream>

    class X {
        ...
    };

    void g(X& ) {
        std::cout << "g() for ref\n";
    }

    void g(X const& ) {
        std::cout << "g() for const ref\n";
    }

    void g(X&& ) {
        std::cout << "g() for rvalue ref\n";
    }

    // f 转发参数给函数 g
    void f(X& value) {
        g(value); // call g(X& )
    }

    void f(X const& value) {
        g(value); // call g(X const& )
    }

    void f(X&& value) {
        g(std::move(value)); // call g(X&& )
    }

如果使用以下模板来实现上述功能，则可能会丢失可移动属性：

.. code-block:: c++

    template<typename T>
    void f(T& value)
    {
        g(value);
    }

C++11 因此引入了特殊的规则来 **完美转发** 参数属性。实现此目的的惯用代码模式如下：

.. code-block:: c++

    template<typename T>
    void f(T&& value)
    {
        g(std::forward<T>(value));
    }

特殊成员函数模板
===================

成员函数模板也可以被用作特殊成员函数。

.. code-block:: c++

    class Person {
    private:
        std::string name;
    public:
        template<typename T>
        explicit Person(T&& n) : name(std::forward<T>(n)) {}

        Person(Person const & p) : name(p.name) {}
        Person(Person&& p) : name(std::move(p.name)) {}
    };

    std::string s = "name";
    Person p1(s); // call template ctor
    Person p2("tmp"); // call template ctor

    Person p3(p1); // ERROR， call template ctor, but no ctor for string with Person
    Person p4(std::move(p1)); // call move ctor

    Person const pc("ctmp"); // call template ctor
    Person pc1(pc); // call copy ctor

``Person p3(p1)`` 之所以会产生编译错误，是因为根据函数的重载解析规则，模板构造函数比拷贝构造函数更匹配，
因为拷贝构造函数需要一个到 ``const`` 的转换操作。

提供一个非常量的拷贝构造并不能完全解决这个问题，在派生类中，成员模板可能还是一个更好的匹配。其实我们真正需要的是
在实参的型别为 Person 或者可以转换为 Person ，我们可以使用 enable_if 来实现这个功能。

.. code-block:: c++

    class ConPerson : public Person {
    public:
        ConPerson(ConPerson const& p) : Person(p) {} // call Person's template ctor
    };

使用 enable_if 禁用模板
============================

从 c++11 开始，C++标准库提供了 std::enable_if 辅助模板来实现在某个编译时条件下忽略函数模板。

.. code-block:: c++

    template<typename T>
    typename std::enable_if<(sizeof(T) > 4)>::type
    foo() { }

std::enable_if 的行为如下所述：

- 如果表达式的值为 true，它的型别成员 type 会产生一个型别：

    * 如果未指定第二个模板实参，那么 type 就是 void；
    * 否则， type 就是第二个模板实参。

- 如果表达式的值为 false，那么 type 就是未定义的。根据模板的 SFINAE 特性，带有 enable_if 表达式的函数模板会被忽略。

在声明的中间使用 enable_if 表达式是相当不灵活的 。因此普遍的方法是使用一个额外的带默认值的函数模板实参。

.. code-block:: c++

    template<typename T, typename = std::enable_if_t<(sizeof(T) > 4)>>
    void foo() { }

使用 enable_if<>
=======================

使用 enable_if 可以解决 Person 类模板的构造模板的问题，如果构造方法的实参的型别是 std::string 或者可以转换为 std::string 时，
则启用该构造模板。

.. code-block:: c++

    template<typename T, typename = std::enable_if_t<std::is_convertible_v<T, std::string>>
    Person(T&& n);


禁用特殊成员方法
------------------

我们不能使用 enable_if_t<> 禁用预定义的拷贝/移动构造和赋值操作符。因为成员方法模板不被当作特殊成员方法，因此，当拷贝构造被需要时，成员方法
模板会被忽略。

.. code-block:: c++

    class C {
    public:
        C() = default;
        template<typename T>
        C(T const& ) {
            std::cout << "template copy ctor\n";
        }
    };

    C x;
    C y{x}; // 调用编译器自动生成的拷贝构造

删除编译器自动生成的拷贝构造会导致 ``C y{x};`` 产生编译错误。

这里有一个比较有技巧性的解决方案，我们可以把拷贝构造函数的参数声明为 ``const volatile`` 并且标记为 delete。这样可以阻止另外的拷贝构造方法被隐示声明。
有了它，非 volatile 型别的参数将会优先匹配模板构造函数。

.. code-block:: c++

    class C {
    public:
        C() = default;
        C(C const volatile& ) = delete;
        template<typename T>
        C(T const& ) {
            std::cout << "template copy ctor\n";
        }
    };

    C x;
    C y{x}; // 调用模板构造函数

使用 Concepts 简化 enable_if 表达式
=======================================

Concepts 从语言特性上支持对模板的需求/条件进行形式化描述，而 enable_if 表达式则是利用了模板实例化时的特性，因而 Concepts 会使得
模板更加容易理解。

.. code-block:: c++

    // C++20
    template<typename T>
    requires std::is_convertible_v<T, std::string>
    explicit Person(T&& n) : name(std::forward<T>(n)) {}

    // 将该需求形式化为一个通用的 concept
    template<typename T>
    concept ConvertibleToString = std::is_convertible_v<T, std::string>;

    template<typename T>
    requires ConvertibleToString<T>
    explicit Person(T&& n) : name(std::forward<T>(n)) {}

    // 直接在模板参数中使用 concept
    template<ConvertibleToString T>
    explicit Person(T&& n) : name(std::forward<T>(n)) {}
