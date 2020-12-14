深入模板基础
###################

参数化声明
===============

C++目前支持四种基本的模板类型：类模板，函数模板，变量模板，以及别名模板。每种模板类型可以出现在命名空间
作用域，也可以出现在类作用域。在类作用域时，它们变成嵌套类模板，成员方法模板，静态数据成员模板，以及成员
别名模板。

C++17 引入了另一个构造， **deduction guides** 。它们不被称作模板，但是语法会让人想起函数模板。

.. code-block:: c++
    :linenos:

    // 命名空间作用域下的类模板
    template<typename T>
    class Data {
    public:
        static constexpr bool copyable = true;
        ...
    };

    // 命名空间作用域的函数模板
    template<typename T>
    void log(T x) {
        ...
    }

    // 命名空间作用域的变量模板（从C++14开始）
    template<typename T>
    T zero = 0;

    // 命名空间作用域的变量模板（从C++14开始）
    template<typename T>
    bool dataCopyable = Data<T>::copyable;

    // 命名空间作用域别名模板
    template<typename T>
    using DataList = Data<T*>;

    class Collection {
    public:

        // 在类内的成员类模板定义
        template<typename T>
        class Node {
            ...
        };

        // 类内的成员函数模板（隐式内联）
        template<typename T>
        T* alloc() {
            ...
        }

        // 类内变量模板（从C++14开始）
        template<typename T>
        static T zero = 0;

        // 类内成员别名模板
        template<typename T>
        using NodePtr = Node<T*> *;
    };

从C++17开始，静态数据成员模板和变量模板也能是”内联的“，意味着它们的定义能够跨编译单元重复。

.. code-block:: c++

    // header.hpp，使用 inline 使得 a 的定义能够被多个编译单元包含   
    struct MyClass {
        static inline int a = 18;
    };

    // 与上面的定义等价
    struct MyClass {
        static int a;
    };

    inline int MyClass::a = 18;

成员模板使用以下方式进行类外定义（别名模板除外）：

.. code-block:: c++

    clas List {
    public:
        List() = default(); // 因为定义了模板构造函数

        template<typename U>
        class Handle; // 类内成员类模板声明

        template<typename U>
        List(List<U> const&); // 成员方法模板声明

        template<typename U>
        static U zero; // 成员变量模板（从C++14开始）
    };

    template<typename T>
    template<typename U>
    class List<T>::Handle { // 类外成员类模板定义
        ...
    };

    template<typename T>
    template<typename U>
    List<T>::List(List<U> const& b) { // 类外成员方法模板定义
        ...
    };

        template<typename T>
    template<typename U>
    U List<T>::zero = 0; // 类外静态成员变量模板定义

**Union 模板**

Union 模板也是合理的（它们被认为是一种类模板）：

.. code-block:: c++

    template<typename T>
    union AllocChunk {
        T object;
        unsigned char bytes[sizeof(T)];
    };

**默认调用实参**

函数模板也能像普通函数一样使用默认调用参数：

.. code-block:: c++

    template<typename T>
    void report_top(Stack<T> const&, int number = 10);

    template<typename T>
    void fill(Array<T>&, T const& = T{});

当 fill() 被调用时，如果提供了第二个调用参数，默认实参不会被实例化。这确保了如果默认调用参数不能为某
个特定的类型实例化时，也不会产生错误：

.. code-block:: c++

    class Value {
    public:
        explicit Value(int); // 没有默认构造函数
    };

    void init(Array<Value>& array
    {
        Value zero(0);

        fill(array, zero); // 没问题，默认构造函数不会被使用
        fill(array); // 错误：未定义的默认构造函数
    }

**类模板的非模板成员**

在一个类里面，除了声明四种基本的模板之外，你也能通过类模板的模板参数来参数化普通类成员，它们有时候被错误地认为成员模板。

.. code-block:: c++

    template<int I>
    class CupBoard {
        class Shelf;
        void open();
        enum Wood : unsigned char;
        static double totalWeight; 
    };

虚成员函数
--------------------

成员函数模板不能被声明为虚函数。这种约束是被强加的，因为虚函数调用机制的实现通常使用固定大小的表，每个虚函数一个入口。然而成员函数模板
的实例化数量不是固定的，除非整个程序被翻译完成。因此，支持虚函数成员模板需要C++编译器和链接器支持一个全新机制。

相反，类模板的普通成员可以是虚方法，因为，当类模板实例化时，它们的数量是固定的。

.. code-block:: c++

    template<typename T>
    class Dynamic {
    public:
        virtual ~Dynamic(); // 没问题：Dynamic<T> 的每个实例一个析构函数

        template<typename T2>
        virtual void copy(T2 const&); // 错误：Dynamic<T> 的每个实例有未知数量的 copy() 的实例
    };

模板链接类型
---------------

每个模板必须有一个名称，并且该名称在它的作用域必须是唯一的，除了能被重载的函数模板。特别注意，与类类型不一样，类模板不能与不同种类的实体
共享同一个名称：

.. code-block:: c++

    int C;
    ...
    class C; // 没问题：类名和非类名不在相同的”空间“

    int X;
    ...
    template<typename T>
    class X; // 错误：与变量 X 冲突

    struct S;
    ...
    template<typename T>
    class S; // 错误：与 struct S 冲突

模板名称有链接类型，但是它们不能有 C 链接类型（名称链接类型跟名称mangle相关）。

.. code-block:: c++

    extern "c++" template<typename T>
    void normal(); // 这是默认的链接类型，可以省略

    extern "C" template<typename T>
    void invalid(); // 错误：模板不能有 C 链接类型

    extern "java" template<typename T>
    void javaLink(); // 非标准的链接类型，可能会某些编译器某天会支持兼容 java 泛型

模板通常拥有 extern 链接类型。例外的情况是命名空间作用域的带有 static 指示符的函数模板，未命名命名空间内的直接或者间接模板成员（拥有内部链接），
未命名类的成员模板（拥有 no linkage）：

.. code-block:: c++

    template<typename T>
    void external(); // 不同文件中，在相同作用的同名声明指向相同的实体

    template<typename T>
    static void internal(); // 不同文件中，相同作用域的同名声明指向不同的模板

    template<typename T>
    static void internal(); // 前一个模板重新声明

    namespace {
        template<typename>
        void otherinternal(); // 另一个文件，出现在类似的匿名命名空间的同名模板，是不相关的
    }

    namespace {
        template<typename>
        void otherinternal(); // 前一个模板重新声明
    }

    struct {
        template<typename T>
        void f(T) {} // no linkage：不能被重新声明
    } x;

目前，模板不能声明在函数作用域或者局部类作用域，但是泛型lambda，它等价于包含成员函数模板的闭包类型，并且能出现在局部作用域，实际上暗示了
一类局部成员函数模板。

模板实例的链接类型就是模板的链接类型。就变量模板而言，这会带来一个有趣的结果。

.. code-block:: c++

    template<typename T> T zero = T{};

zero 的所有实例都拥有 external 链接类型，即使 zero<int const> 也是这样。这可能违反了我们的直觉：

.. code-block:: c++

    int const zero_int = 0;

声明为常量类型时，变量是 internal 链接类型。类似地，所有以下模板的实例拥有 external 链接类型。尽管所有这些实例都拥有 int const 类型。

.. code-block:: c++

    template<typename T> int const max_volume = 11;

主模板
-----------

模板的一般声明被称作 **主模板** 。该模板声明不会再模板名称之后的尖括号内添加模板实参：

.. code-block:: c++

    template<typename T> class Box; // 没问题： 主模板
    template<typename T> class Box<T>; // 错误：没有特化

    template<typename T> void translate(T) // 没问题：主模板
    template<typename T> void translate<T>(T); // 错误：函数模板不支持特化

    template<typename T> constexpr T zero = T{}; // 没问题：主模板
    template<typename T> constexpr T zero<T> = T{}; // 错误：没有特化

非主模板出现在声明类模板或者变量模板的偏特化时。函数模板必须总是主模板。

模板参数
=============

这里有三种基本的模板参数：

1. 类型参数

2. 非类型参数

3. 模板的模板参数

这些基本的模板参数都能被用作模板参数包的基础元素。

模板参数在模板声明的参数化句子的开端。该声明不必有名称：

.. code-block:: c++

    template<typename, int>
    class X; // X<> 被一个类型和整数参数化

仅在后面被引用时，才需要模板参数名称。模板参数名称也可以在后续的参数声明中被引用。

.. code-block:: c++

    template<typename T, T Root, template<T> class Buf>
    class Structure;

型别参数
--------------

型别参数要么通过关键字 ``typename`` 要么通过关键字 ``class`` 引入：它们完全等价。关键字紧接一个简单的标识符，标识符
后面跟着一个逗号表示下一个参数声明，一个闭包的尖括号（>）标识参数化句子的结束，或者等号（=）不表示默认模板实参的开始。

在一个模板声明中，型别参数很像类型别名。例如，即使模板参数 T 将被一个类类型替换，也不能使用 **class T** 的详细名称格式：

.. code-block:: c++

    template<typename Allocator>
    class List {
        class Allocator* allocptr; // 错误：使用 "Allocator* allocptr"
        friend class Allocator; // 错误：使用 "friend Allocator"
        ...
    };

非类型参数
----------------

非类型模板参数代表能在编译时或者链接时确定的常量。这样的参数的类型必须是以下几种之一：

- 整数类型或者枚举类型

- 指针类型

- 成员指针类型

- 左值引用类型（对象引用和函数引用都可接受）

- std::nullptr_t

- 包含 auto 或者 decltype(auto) 的型别（从C++17开始，类型占位符）

函数和数组类型也可以被指定，但他们会隐式的退化为指针类型：

.. code-block:: c++

    template<int buf[5]> class Lexer; // buf 实际上是 int *
    template<int * buf> class Lexer; // 没问题：这是重新声明

    template<int fun()> struct FuncWrap; // fun 实际上是函数指针
    template<int (*)()> struct FuncWrap; //没问题：这是重新声明

非类型模板参数很像变量声明，只是它们不能拥有像 static, mutable 等非类型指示符。它们可以拥有 const 和 volatile 限定符，
但是如果该限定符出现在参数类型的最外层，它们会被忽略：

.. code-block:: c++

    template<int const length> class Buffer; // const 是没用的
    template<int length> class Buffer; // 与前面的声明一样

最后，在表达式中使用时，非引用的非类型参数总是右值。它们不能取地址，也不能作为被赋值的一方。另一方面，非类型参数的左值引用可以用来表示
左值：

.. code-block:: c++

    template<int& Counter>
    struct LocalIncrement {
        LocalIncrement() { Counter = Counter + 1; } // 可以作为左值，出现在等号左边
        ~LocalIncrement() { Counter = Counter - 1; }
    }

模板的模板参数
==================

模板的模板参数是类模板和别名模板的占位符。它们很像类模板，但是不能使用关键字 struct 和 union：

.. code-block:: c++

    template<template<typename X> class C> // 没问题
    void f(C<int> *p);

    template<template<typename X> struct C> // 错误：此处 struct 是无效的
    void f(C<int> *p);

    template<template<typename X> union C> // 没问题：此处 union 是无效的
    void f(C<int> *p);

C++17允许使用 typename 代替 class ：作出这个改变的动机是基于以下事实，模板的模板参数不仅能被类模板替换，也能
被别名模板替换。

.. code-block:: c++

    // since C++17
    template<template<typename X> typename C>
    void f(C<int> *p); 

模板的模板参数的参数也可以有默认模板实参。使用模板的模板参数时，如果未指定相关参数的话，就会使用默认实参：

.. code-block:: c++

    template<template<typename T, typename A = MyAllocator> class Container>
    class Adaptation {
        Container<int> storage; // 等价于 Container<int, MyAllocator>
        ...
    };

T 和 A 是模板的模板参数 Container 的模板参数。这些名称仅能用在模板的模板参数的其他参数声明中。

.. code-block:: c++

    template<template<typename T, T*> class Buf> // 没问题
    class Lexer {
        static T* storage; // 没问题：模板的模板参数的模板参数不能在这里使用
        ...
    };

然而，通常模板的模板参数的模板参数的名称不会被其他参数使用，因此经常会省略它们的名称。

.. code-block:: c++

    template<template<typename, typename = MyAllocator> class Container>
    class Adaptation {
        Container<int> storage; // 等价于 Container<int, MyAllocator>
        ...
    };

模板参数包
------------------

从C++11开始，任何类型的模板参数都能通过在模板参数名称前引入一个省略号（...）变成一个模板参数包，如果模板参数
没有名称的话，省略号就在模板参数名称出现的地方：

.. code-block:: c++

    template<typename... Types> // 声明一个名叫 Types 的模板参数包
    class Tuple;

模板参数包表现得像它潜在的模板参数，但有个至关重要的区别：普通的模板参数精确匹配一个模板实参，模板参数包却能匹配任意数量的模板实参:

.. code-block:: c++

    using IntTuple = Tuple<int>; // 没问题：一个模板实参
    using IntCharTuple = Tuple<int, char>; // 没问题：两个模板实参
    using IntTriple = Tuple<int, int, int>; // 没问题：三个模板实参
    using EmptyTuple = Tuple<>; // 没问题：零个模板实参

类似地，非类型和模板的模板参数的模板参数包也能接受任意数量的非类型或者模板的模板参数：

.. code-block:: c++

    template<typename T, unsigned... Dimensions>
    class MutiArray; // 没问题，声明非类型的模板参数包

    using TransformMatrix = MutiArray<double, 3, 3>; // 没问题： 3x3 的矩阵

    template<typename T, template<typename, typename>class... Containers>
    void testContainers(); // 没问题：声明模板的模板参数的模板参数包

C++17引入了非类型模板参数推导：

.. code-block:: c++

    // since C++17
    template<typename T, auto... Dimensions>
    class MutiArray; 
        
    template<typename T, decltype(auto)... Dimensions>
    class MutiArray; 

主类模板，变量模板和别名模板至多有一个模板参数包，并且模板参数包必须是最后一个模板参数。函数
模板有一个更弱的限制：允许多个模板参数包，只要在模板参数包之后的每个模板参数要么有一个默认值，
要么能被推导出来：

.. code-block:: c++

    template<typename... Types, typename Last>
    clas LastType; // 错误：模板参数包不是最后一个模板参数

    template<typename... TestTypes, typename T>
    void runTests(T value); // 模板参数包后紧接着一个可推导的模板参数

    template<unsigned...> struct Tensor;
    template<unsigned... Dims1, unsigned... Dims2>
    auto compose(Tensor<Dims1...>, Tensor<Dims2...>); // 没问题： tensor dimensions 能被推导

类模板和变量模板的偏特化可以有多个模板参数包,这是因为偏特化是通过推导过程被选择的，这与函数模板选择过程等同。

.. code-block:: c++

    template<typename...> TypeList;
    template<typename X, typename Y> struct Zip;
    template<typename... Xs, typename... Ys>
    struct Zip<Types<Xs...>, TypeList<Ys...>>; // 没问题：偏特化使用推导来确定 Xs 和 Ys 的替换

也许并不令人惊讶，一个类型参数包不能在它自己的参数子句中展开（类模板不支持多个模板参数包）：

.. code-block:: c++

    template<typename... Ts, Ts... vals> 
    struct StaticValues {}; // 错误： Ts 不能被展开成它自己的参数列表

然而，嵌套模板可以创造类似情况

.. code-block:: c++

    template<typename... Ts> struct ArgList {
        template<Ts... vals> struct Vals {};
    };

    ArgList<int, char, char char>::Vals<3, 'x', 'y'> tada;

包含一个模板参数包的模板被称为 **可变参数模板** ，因为它接受可变数量的模板实参。

默认模板实参
--------------

任何一种不是模板参数包的模板参数都能装备默认模板实参，尽管它必须在种类上匹配相应的模板参数（比如，一个类型的模板参数不能有一个非类型的默认实参）。
默认实参不能依赖它自己的参数，因为直到默认实参之后为止，参数的名称不在作用域内。然而，它可以依赖前面的参数。

.. code-block:: c++

    template<typename T, typename Allocator = allocator<T>>
    class List;

仅当后续的参数也提供了默认实参时，类模板，变量模板或者别名模板的模板参数才可以有默认模板实参（类似于函数的默认参数）。后续的默认值通常在同一个模板声明
中提供，但他们也可以在该模板的前一个声明中被声明。

.. code-block:: c++

    template<typename T1, typename T2, typename T3, typename T4 = char, typename T5 = char>
    class QuintTuple; // 没问题

    template<typename T1, typename T2, typename T3 = char, typename T4, typename T5>
    class QuintTuple; // 没问题：T4 和 T5 已经有默认值

    template<typename T1 = char, typename T2, typename T3, typename T4, typename T5>
    class QuintTuple; // 没问题：T2 没有默认值

函数模板的模板参数的默认模板实参不需要后续的模板参数有默认模板实参：

.. code-block:: c++

    template<typename R = void, typename T>
    R* addressof(T& value); // 没问题：如果没有显示指定的话，R 就是 void。

默认模板实参不能重复。

.. code-block:: c++

    template<typename T = void>
    class Value;

    template<typename T = void>
    class Value; // 错误：重复的默认实参

许多语境下不允许默认模板实参：

- 偏特化

    .. code-block:: c++

        template<typename T>
        class C;

        ...

        template<typename T = int>
        class C<T*>; // 错误

- 参数包

    .. code-block:: c++

        template<typename... Ts = int> struct X; // 错误

- 类模板的类外成员定义

    .. code-block:: c++

        template<typename T>
        struct X {
            T f();
        };

        template<typename T = int> T X<T>::f() {} // 错误

- 友元类模板声明（友元类不能在友元声明时定义它）

    .. code-block:: c++

        struct S {
            template<typename = void> friend struct F; // 错误
        };

        struct S {
            friend struct F {}; // 错误
        };

- 友元函数模板声明，除非它是个定义并且不在编译单元内其它地方声明

    .. code-block:: c++

        struct S {
            template<typename = void> friend void f(); // 错误：不是一个定义
            template<typename = void> friend void g() { } // 目前为止没问题
        };

        // 自己测试发现不会报错
        template<typename> void g(); 

模板实参
=============

当实例化一个模板时，模板参数被模板实参替换。实参能被几种不同的机制确定：

- 显示模板实参：模板名称后面紧接着用尖括号括起来的显示模板实参。产生的名称称作 **template-id** 

- 注入类名：在带模板参数 P1，P2，... 的类模板 X 的作用域内，模板名称（X）等价于 template-id X<P1, P2, ...> 

- 默认模板实参：如果有默认模板实参，显示模板实参能够从模板实例中删除。然而，对于类模板和变量模板，即使所有的模板参数
  有默认值，必须提供尖括号（即使为空）

- 模板实参推导：未显示指定的函数模板实参可以从函数调用实参推导出来。如果所有的模板实参能够被推导出来，则不需要在函数模板的名称后
  指定尖括号。C++17 也引入了从变量初始化器或者函数符号类型转换推导类模板实参的能力。

函数模板实参
----------------

函数模板的模板实参能被显示指定，从模板被使用的方式推导出来，或者提供默认模板实参：

.. code-block:: c++

    template<typename T>
    T max(T a, T b)
    {
        return b < a ? a : b;
    }

    int main()
    {
        ::max<double>(1.0, -3.0); // 显示指定模板实参
        ::max(1.0, -3.0); // 模板实参被隐式推导为 double
        ::max<int>(1.0, -3.0); // 显示的 <int> 抑制了推导，因此类型是 int
    }

一些模板实参不能被推导，因为与之相关的模板参数未出现在函数参数类型中或者一些其他原因。不能推导的模板参数通常放在模板参数列表的开头，这样
就能显示指定这些模板参数，并且允许其他模板实参能被推导出来。

.. code-block:: c++

    template<typename DstT, typename SrcT>
    DstT implicit_cast(SrcT const& x) // SrcT 能被推导出来，但 DstT 不能被推导
    {
        return x;
    }

    int main()
    {
        double value = implicit_cast<double>(-1);
    }

如果把上述例子的模板参数的顺序反过来，调用 implicit_cast 不得不显示指定两个模板实参。

另外，不能被推导的实参不能有效地放在模板参数包或者偏特化之后，因为将会没有办法显示指定或者推导它们。

.. code-block:: c++

    template<typename... Ts, int N>
    void f(double (&)[N+1], Ts...  ps); // 无用的声明，因为 N 不能被显示指定或者被推导（不会产生编译错误）

    template<typename... Ts, int N>
    void f(double (&)[N], Ts...  ps); // 没问题，因为此时 N 可以被推导

    template<typename T>
    class MyClass;

    // 偏特化包含不能被推导的模板实参
    template<int N>
    class MyClass<double(&)[N+1]> {
        ...
    };

因为函数模板能够被重载，显示地为函数模板提供所有的实参不足以确认单个函数：在某些情况下，它发现了一组函数：

.. code-block:: c++

    template<typename Func, typename T>
    void apply(Func funcPtr, T x) {
        funcPtr(x);
    }

    template<typename T> void single(T);

    template<typename T> void muti(T);
    template<typename T> void muti(T*);

    int main()
    {
        apply(&single<int>, 3); // 没问题
        apply(&muti<int>, 7); // 错误：muti<int> 并不唯一
    }

使用模板实参替换函数模板的模板参数可能会导致尝试构造一个无效的C++类型或者表达式，这个遵循 SFINAE 原则。

.. code-block:: c++

    template<typename T> RT1 test(typename T::X const*);
    template<typename T> RT2 test(...);

    test<int>; // 仅确认第二个模板，第一个模板会导致无效的类型，因此会被忽略

类型实参
------------

模板类型实参时模板类型参数的”值“。任何类型（包括 void，函数类型，引用类型等）通常都能被用作模板实参，但是使用它们替换
时必须产生有效的构造：

.. code-block:: c++

    template<typename T>
    void clear(T p)
    {
        *p = 0; // 需要 operator* 能被应用到 T
    }

    int main()
    {
        int a;
        clear(a); // 错误：int 不支持 operator *
    }

非类型模板实参
-----------------

非类型模板实参是非类型模板参数被替换的值。该值必须是以下几种东西之一：

-  另一个有适当类型的非类型模板实参

- 编译时整型（或者枚举）常量值。仅当相关的模板参数的类型与值的类型匹配，或者值的类型可以隐式转换并且不会产生窄化。
  比如，char 类型的值可以提供给 int 类型的参数，但是 500 给 8-bit 的 char 类型参数是无效的

- 外部变量或者函数名称之前使用内置的 operator& 。对于函数或者数组，可以省略 & 。这样的模板实参匹配指针类型的非类型模板参数。
  C++17允许任意产生函数或者变量指针的常量表达式

- 开头不带 & 操作符的上述实参对于引用类型的非类型模板实参是有效实参。C++17允许函数或者变量glvalue（使用std::move之后的值）

- 成员指针常量；换言之，形如 &C::m , C 是类类型，且 m 是非静态成员（数据或者函数）。该参数仅匹配成员指针的非类型参数

- 空指针常量是指针或者成员指针的非类型参数的有效实参

C++17之前，当匹配指针或者引用参数时，用户定义的转换操作（单参数构造函数和转换操作符）和派生类到基类的隐式转换不被考虑。

.. code-block:: c++

    template<typename T, T nontypeParam>
    class C;

    C<int, 33>* c1; // 整型

    int a;
    C<int *, &a>* c2; // 外部变量的地址

    void f();
    void f(int);
    C<void (*)(int), f>* c3; // 重载解析规则选择 f(int)

    template<typename T> void templ_func();
    C<void(), &templ_func<double>>* c4; // 函数模板实例是函数

    struct X {
        static bool b;
        int n;
        constexpr operator int() const { return 42; }
    }

    C<bool&, X::b>* c5; // 静态类成员是可接受的变量/函数名称

    C<int X::*, &X::n>* c6; // 成员指针常量

    C<long, X{}>* c7; // 没问题， X 首先通过 constexpr 的转换函数转成 int,然后通过标准转换转换成 long

    constexpr int foo() {
        return 10;
    }

    C<int, foo()>* c8; // 没问题，foo() 是个常量表达式函数
    
    int fm = 10;
    constexpr int* bar() {
        return &fm;
    }

    C<int*, bar()>* c9; //C++17 支持指针和引用的常量表达式

模板实参的一般约束是，编译或者链接器必须能在程序构建时表达它们的值。直到程序运行时才知道的值（比如，临时变量的地址）与
模板在程序构建时实例化的概念是不兼容的。

即使如此，有些常量目前不是有效的：

- 浮点数

- 字符串常量

字符串常量的一个问题是两个相同的字面量可能存储在不同的地址。一个可选的表示常量字符串模板实例的方式（但是比较麻烦）需要引入
一个额外的变量保存字符串：

.. code-block:: c++

    template<char const* str>
    class Message {
        ...
    };

    extern char const hello[] = "Hello World!";

    char const hello11[] = "Hello World!";

    void foo()
    {
        static char const hello17[] = "Hello World!";

        Message<hello> msg03; // 所有C++标准都没问题
        Message<hello11> msg11; // 从C++11开始支持
        Message<hello17> msg17; // 从C++17开始支持
    }

声明为引用或者指针类型的非类型模板参数，在所有的C++版本都可以使用 external linkage 的常量表达式，C++11 可以使用 internal linkage 的
常量表达式，C++17可以使用任何 linkage 的常量表达式。

这里有一些其他的无效例子（没那么吃惊）：

.. code-block:: c++

    template<typename T, T nontypeParam>
    class C;

    struct Base {
        int i;
    } base;

    struct Derived : public Base {
    } derived;

    C<Base *, &derived>* err1; // 错误：派生类到基类的转换不会被考虑

    C<int&, base.i>* err2; // 错误：变量的字段不被认为是变量

    int a[10];
    C<int *, &a[1]>* err3; // 错误：数组元素的地址不被接受
    C<int *, &a[0]>* ok; // 没问题

模板的模板实参
--------------------

模板的模板实参一般必须是一个类模板或者别名模板，并且它们的模板参数完全匹配它替换的模板的模板参数。在C++17之前，模板的模板实参的默认实参会被
忽略（但如果模板的模板参数有默认实参的话，会在模板实例时被考虑）。C++17放松了这个匹配规则，仅需要模板的模板参数至少和相关的模板的模板实参一
样特殊（使用 clang 进行测试时，发现使用C++17并不能支持该特性）。

.. code-block:: c++

    #include<list>
    // template<typename T, typename Allocator = allocator<T>>
    // class list;

    template<typename T, template<typename> class Cont> // Cont期望一个参数
    class Rel {
        Cont<T> cont; 
    };

    Rel<int, std::list> rel; // 错误：std::list 有超过一个模板参数

    // 改成以下形式可以通过编译，但是 std::list 默认实参并不会传递
    template<typename T, template<typename, typename> class Cont> 
    class Rel {
        Cont<T, std::allocator<T>> cont;
    };

    // 给模板的模板参数添加默认实参
    template<typename T, template<typename T1, typename = std::allocator<T1>> class Cont> 
    class Rel {
        Cont<T> cont;
    };

带可变参数的模板的模板参数提供一个解决方案，模板的模板参数包能匹配零个或者多个模板的模板实参中的同一种类的模板参数。

.. code-block:: c++

    template<typename T, template<typename...> class Cont>
    class Rel {
        Cont<T> cont; 
    };

.. code-block:: c++

    // template<typename Key, typename T,
    //        typename Compare = less<Key>,
    //        typename Allocator = allocator<pair<Key const, T>>>
    // class map;

    // template<typename T, size_t N>
    // class array;

    template<template<typename...> class TT>
    class AlmostAnyTmpl {
        ...
    };

    AlmostAnyTmpl<std::vector> withVector; // 两个类型参数
    AlmostAnyTmpl<std::map> withMap; // 四个参数
    AlmostAnyTmpl<std::array> withArray; // 错误：类型模板参数包不能匹配非类型模板参数

实参的等价性
--------------

当实参的值一模一样时，两组模板实参是等价的。对于类型实参，类型别名不会影响：最终被比较的类型是类型别名声明潜在的类型。
对于整型非类型实参，比较的是实参的值，怎么表示该值是没关系的。

.. code-block:: c++

    template<typename T, int I>
    class Mix;

    using Int = int;
    Mix<int, 3 * 3>* p1;
    Mix<Int, 4 + 5>* p2; // p2 和 p1 类型相同

在模板依赖的上下文中，模板实参的“值”并不总是能确定，并且等价性规则变得有点更复杂：

.. code-block:: c++

    template<int N> struct I {};

    template<int M, int N> void f(I<M+N>);
    template<int N, int M> void f(I<N+M>);
    template<int M, int N> void f(I<N+M>);

前两个 f<>() 是等价地（将第二个的 M 重命名为 N， N 重命名为 M，两个函数模板就变成一模一样了）。第三个函数模板
与前两个函数是功能上等价，因为函数模板参数无论传递什么值，都会产生相同的结果。 

由函数模板生成的函数不会与普通函数等价，即使它们有相同的类型和相同的名称。这对类成员产生了两个重要的推论：

- 由成员函数模板生成的函数不会重载虚函数

- 由构造模板生成的构造函数不会是拷贝/移动构造。类似地，由赋值操作符模板生成的赋值操作符不会是拷贝赋值或者移动赋值操作符。

可变参数模板
==================

可变参数模板是至少包含一个模板参数包的模板。可变参数模板对于泛化为任意数量的实参的模板行为特别有用。 **Tuple** 类模板就是
可变参数模板，因为元组能拥有任意数量的元素。

当可变参数模板的模板实参被确定时，可变参数模板的每个模板参数包将匹配一连串的零个或者多个模板实参。这一连串的模板实参被称
为 **实参包** 。它们不被称作模板，但是语法会让人想起函数模板。

.. code-block:: c++

    template<typename... Types>
    class Tuple {
        ...
    };

    int main()
    {
        Tuple<> t0; // Types 包含一个空列表
        Tuple<int> t1; // Types 包含 int
        Tuple<int, float> t2; // Types 包含 int 和float
    }

sizeof... 可以计算实参包中实参的数量：

.. code-block:: c++

    template<typename... Types>
    class Tuple {
    public:
        static constexpr std::size_t length = sizeof...(Types);
    };

    int a1[Tuple<int>::length]; // 包含一个整数的数组
    int a3[Tuple<short, int, long>::length]; // 包含三个整数的数组

包扩展
------------

sizeof... 是 **包扩展** 一个例子。包扩展是一个将实参包扩展成单独的实参的构造。尽管 sizeof... 仅执行该扩展来计算
单独的实参的数量，但其他形式的参数包（那些出现在C++要求一个列表的地方），可以扩展列表里面的多个元素：

.. code-block:: c++

    template<typename... Types>
    class MyTuple : public Tuple<Types...> {
        ...
    };

    MyTuple<int, float> t2; // 继承自 Tuple<int, float>

模板实参 Types... 是一个包扩展，它产生了一连串的模板实参，对应于为 Types 替换的参数包中的每一个实参。      

一个理解包扩展的直观方式就是从从句法扩展的角度思考，模板参数包被精确地数量正确的（非包）模板参数替换，并且包扩展被写成单独
的实参，每个非包的模板参数一次。比如， MyTuple 如果被扩展成两个参数时将会看起来像下面这样：

.. code-block:: c++

    template<typename T1, typename T2>
    class MyTuple : public Tuple<T1, T2> {
        ...
    };

使用三个参数扩展：

.. code-block:: c++

    template<typename T1, typename T2, T3>
    class MyTuple : public Tuple<T1, T2, T3> {
        ...
    };

每个包扩展都有模式，它表示对实参包中的每个实参都会被重复的类型和表达式，通常出现在表示包扩展的省略号之前。我们先前的例子仅有平凡
的模式（参数包的名称），但是模式可以相当复杂。比如，我们可以定义一个新类型 PtrTuple ，它派生自实参类型的指针的 Tuple ：

.. code-block:: c++

    template<typename... Types>
    class PtrTuple : public Tuple<Types*...> {
        ...
    };

    PtrTuple<int, float> t2; // 继承自 Tuple<int*, float*>

包扩展 Types*... 的模式就是 Types* 。重复替换成该模式将产生一连串的模板类型实参，它们是替换 Types 的实参包中的类型的指针形式。
PtrTuple 的包扩展的句法解释如下所示：

.. code-block:: c++
    
    template<typename T1, typename T2, typename T3>
    class PtrTuple : public Tuple<T1*, T2 *, T3*> {
        ...
    };

包扩展能存在于在哪些地方
----------------------------

到目前为止，我们的例子聚焦于使用包扩展来产生一连串的模板实参。实际上，包扩展本质上可以被用在语言提供逗号分隔列表语法的任何地方，包括：

- 基类列表

    .. code-block:: c++

        template<class... Mixins>
        class X : public Mixins... ; // 展开成 class X : public ArgTy1, public ArgTy2, ...


- 在构造方法中的基类初始化列表（ A(int p1, int p2, int p3, ...) : B(p1), C(p2), ... ）

    .. code-block:: c++

        template<class... Mixins>
        class X : public Mixins... {
        public:
            // 展开成 X(const ArgTy1& a1, const ArgTy2& a2, ...) : ArgTy1(a1), ArgTy2(a2), ...
            X(const Mixins&... mixins) : Mixins(mixins)... { } 
        };

- 调用实参列表（模式就是实参表达式）
    
    .. code-block:: c++

        f(&args...); // 展开成 f(&a1, &a2, &a3, ...)
        f(n, ++args...); // 展开成 f(n, ++a1, ++a2, ++a3, ...);
        f(const_cast<const Args*>(&args)...); // 展开成 f(const_cast<const ArgTy1*>(&a1), const_cast<const ArgTy2*>(&a2), ...)
        f(h(args...) + args...); // 展开成 f(h(a1, a2, ...) + a1, h(a1, a2, ...) + a2, ...)
    
- 初始化列表（比如，花括号初始化）

    .. code-block:: c++

        // 圆括号初始化
        Class c1(&args...); // 调用 Class::Class(&a1, &a2, &a3, ...)
        Class c2 = Class(n, ++args...); // 调用 Class::Class(n, ++a1, ++a2, ++a3, ...)
        ::new((void *)p) U(std::forward<Args>(args)...); // 展开成 ::new((void *)p) U(std::forward<ArgTy1>(a1), std::forward<ArgTy2>(a2), ...);
        
        // 花括号初始化
        template<typename... Ts> 
        void func(Ts... args) {
            constexpr int size = sizeof...(args) + 2;
            int res[size] = {1, args..., 2}; // 展开成 int res[size] = {1, a1, a2, ..., 2}
            int dummy[sizeof...(Ts)] = { (std::cout << args, 0)... }; // 展开成 { (std::cout << a1, 0), (std::cout << a2, 0), ... }
        }

- 类模板，函数模板和别名模板的模板参数列表

    .. code-block:: c++

        template<typename... T> 
        struct value_holder
        {
            template<T... Values> // 展开成非类型模板形参列表，template<ArgTy1, ArgTy2, ArgTy3, ...>
            struct apply { };
        };

- 模板实参列表

    .. code-block:: c++

        template<class A, class B, class...C> 
        void func(A arg1, B arg2, C...arg3)
        {
            container<A, B, C...> t1;  // 展开成 container<A, B, ArgTy1, ArgTy2, ...> 
            container<C...,A,B> t2;  // 展开成 container<ArgTy1, ArgTy2, ArgTy3, ..., A, B> 
            container<A,C...,B> t3;  // 展开成 container<A, ArgTy1, ArgTy2, ArgTy3, ..., B> 
        }        

- 可以被函数抛出的异常列表（C++11 和 C++14 已经被弃用，C++17 被禁止）

    .. code-block:: c++

        template<class...X> 
        void func(int arg) throw(X...) // 展开成 throw(ArgTy1, ArgTy2, ...)
        {
            ...
        }

- 如果属性本身支持包扩展，也可以出现在属性中（然而C++标准还未定义这样的属性）

    .. code-block:: c++

        void [[attributes...]] f(); // 展开成 void [[attr1], [attr2], ...]

- 指定声明的对齐值

    .. code-block:: c++

        template <typename... Ts>
        struct C
        {
            uint8_t i4 alignas (Ts...); // 展开成 alignas(ArgTy1, ArgTy2, ArgTy3, ...)
        };

- 指定 lambda 表达式的捕获列表

    .. code-block:: c++

        template<class... Args>
        void f(Args... args) {
            auto lm = [&, args...] { return g(args...); }; // 展开成 [&, a1, a2, ...]
            lm();
        }

- 函数类型的参数列表

    .. code-block:: c++

    template<typename ...Ts> void f(Ts...) {}
    f('a', 1);  // 展开成 void f(char, int)
    f(0.1);     // 展开成 void f(double)
    
    template<typename ...Ts, int... N> void g(Ts (&...arr)[N]) {}
    int n[1];
    g("a", n); // 展开成 void g(char const(&)[2], int(&)[1]）

- using 声明（C++17 开始支持）

    .. code-block:: c++

        template <typename... bases>
        struct X : bases... {
            using bases::g...; // 展开成 using B::g, D::g, ...
        };

        X<B, D> x; 

我们已经提及 sizeof... 作为包扩展机制实际上不会产生列表。C++17添加了折叠表达式，另一个不产生逗号分隔列表的机制  。

某些包扩展语境被包含仅仅是为了完整性，因此我们将我们精力集中于在实际使用中趋于有用的包扩展语境。由于所有语境下的包扩展遵循相同的原则和
语法，因此你应该能从此处给的例子推断出更深奥的包扩展使用语境。

在基类列表中的包括会展开成一些直接基类。该扩展对于通过 **mixins** 聚集外部提供的数据和功能非常有用，也就是一些类旨在“混入”一个类继承
层次来提供新的行为。

.. code-block:: c++

    template<typename... Mixins>
    class Point : public Mixins... { // 基类包展开
        double x, y, z;
    public:
        Point() : Mixins()... { } // 基类初始化包展开

        template<typename Visitor>
        void visitMixins(Visitor visitor) {
            visitor(static_cast<Mixins&>(*this)...); // 函数调用实参包展开
        }
    };

    struct Color { char red, green, blue; };
    struct Label { std::string name; };
    Point<Color, Label> p; // 继承自 Color 和 Label

函数参数包
-----------------

**函数参数包** 是一个函数参数，它匹配零个或者多个函数调用实参。与函数模板参数包一样，函数参数包也是使用在函数参数
名称之前（或者函数参数名称的位置）的省略号（...）引入的，与模板参数包也一样的是，函数参数包无论何时被使用，都必须
通过包扩展来展开。模板参数包和函数参数包统称为 **参数包** 。

与模板参数包不一样的是，函数参数包总是包扩展，因此它们的声明类型必须包含至少一个参数包。

.. code-block:: c++

    template<typename... Mixins>
    class Point : public Mixins...
    {
        double x, y, z;
    public:
        Point(Mixins... mixins)  // mixins 是函数参数包
            : Mixins(mixins)... {} // 使用提供的 mixin 值初始化每个基类
    };

    struct Color { char red, green, blue; };
    struct Label { std::string name; };
    Point<Color, Label> p( {0x7F, 0, 0x7F}, {"center"} ); 

函数模板的函数参数包可能依赖声明在模板中的模板参数包，它允许模板在接受任意数量的调用实参的同时而不会丢失类型信息：

.. code-block:: c++

    template<typename... Types>
    void print(Types... values);

    int main()
    {
        std::string welcome("Welcome to ");
        print(welcome, "C++ ", 2001, '\n'); // 调用 print<std::string, char const*, int, char>
    }

当使用一些实参调用函数模板 print() 时，放置在实参包中的实参类型将替代模板类型参数包 **Types** ，同时，放置在
实参包中的实际参数值将替代函数参数包 **values** .

print() 的实际实现使用递归模板实例化产生。

出现在参数列表最后的未命名函数参数包和C风格的“vararg”参数存在语法歧义：

.. code-block:: c++

    template<typename T> void c_style(int, T...);
    template<typename T...> void pack(int, T...);

在第一种情况下，“T...”被当作“T, ....”：一个类型T的未命名参数后面跟着一个C风格的vararg参数。在第二种情况下，“T...”
构造被当作一个函数参数包，因为T是一个有效展开模式。可以通过在省略号之前添加逗号（这保证了省略号被当作C风格的vararg参数）
或者在省略号后面添加标识符（这使得它成为一个命名函数参数包）。注意在泛型lambda中，如果省略号之前包含 **auto** （中间
不会包含逗号）， 尾部的省略号被当作表示参数包。

多重和嵌套包扩展
----------------------

包扩展的模板可以任意地复杂，并且可能包括多个、不同的参数包。当实例化包含多重参数包的包扩展时，所有参数包必须拥有相同的长度。
最终的类型或者值序列将由逐个元素组成，替换每个参数包的第一个实参成指定模式，紧接着每个参数包的第二个实参，等等。

.. code-block:: c++

    template<typename F, typename... Types>
    void forwardCopy(F f, Types const&... values)
    {
        f(Types(values)...); // 依次调用每个实参的拷贝构造函数
    }

调用参数命名了两个参数包， **Types** 和 **values** 。当实例化该模板时，Types和values参数包的逐元素扩展产生了一系列的对象构造，
它构建了values的第 *i* 个值的拷贝，并且转换成Types的第 *i* 个类型。三个实参的 forwardCopy 看起来像下面这个样子：

.. code-block:: c++

    template<typename F, typename T1, typename T2, typename T3>
    void forwardCopy(F f, T1 const&v1, T2 const& v2, T3 const& v3)
    {
        f(T1(v1), T2(v2), T3(v3));
    }

包扩展本身也可以被嵌套。在这种情况下，一个参数包的每次出现被离它最近且包围它的包扩展所“展开”（并且仅被这个包扩展展开）。

.. code-block:: c++

    template<typename... OuterTypes>
    class Nested {
        template<typename... InnerTypes>
        void f(InterTypes const&... innerValues) {
            g(OuterTypes(InnerTypes(innerValues)...)...);
        }
    };

调用函数g()，模式为 **InnerTypes(innerValues)** 的包扩展是最内层的包扩展，它扩展了 InnerTypes 和 innerValues，并且
产生了一组函数调用实参来初始化由 OuterTypes 表示的对象。外层的包扩展的模式包含了内层的包扩展，产生了一组调用函数 g() 的调用
实参，由通过内层的扩展产生的一系列的函数调用实参创建的 OuterTypes 中的每个类型的对象组成调用实参。针对两个实参的 OuterTypes
和 三个实参的 InnerTypes 和 innerValues 的语法解释如下：

.. code-block:: c++

    template<typename O1, typename O2>
    class Nested {
        template<typename T1, typename T2, typename T3>
        void f(T1 const& iv1, T2 const& iv2, T3 const& iv3) {
            g( O1(T1(iv1), T2(iv2), T3(iv3)), 
               O2((T1(iv1), T2(iv2), T3(iv3)))
            )
        }
    };

零长度包扩展
------------------

包扩展的语法解释是理解可变参数模板实例在不同数量的实参下的行为的一个重要工具。然而，在实参包长度为零时，语法解释通常会失败。

.. code-block:: c++

    template<>
    class Point : {
        Point() : {}
    };

上述代码是不合语法的，因为模板参数列表现在是空的，并且空的基类列表和基类初始化列表每个都有一个孤立的冒号。

包扩展实际上是语义构造，并且任何大小的实参包的替换都不会影响包扩展（或者包裹它的变参模板）如何被解析。更确切地讲，当一个包
扩展展开成一个空列表时，程序（从语义上）表现得就好像列表不存在似的。 Point<> 实例最终没有积累，并且它的默认构造函数没有基
类初始化，然而在其他方面都是符合语法规则的。即使当零长度包扩展的语法解释是定义明确的（但可能不同）代码，仍保留语义规则：

.. code-block:: c++

    template<typename T, typename... Types>
    void g(Types... values) {
        T v(values...);
    }

变参函数模板 g() 创建了一个从一系列的给定的值直接初始化的值 v 。如果 values 为空， v 的声明，从语法上看，像一个
函数声明（ ``T v()`` ）。然而替换成包扩展是语义上的，并且不会影响解析产生的实体的种类，所以 v 是通过零个实参初始
化，也就是说，值初始化。

类模板成员和类模板内的嵌套类也有类似的限制：如果一个声明为某个类型的成员似乎不是函数类型，但在实例化之后，该成员的类型
变成了函数类型，这样的程序是不符合语法规范的，因为成员的语义解释由数据成员变成了函数成员。

.. code-block:: c++

    template<typename T, unsigned... values>
    struct Inner {
        T member(values...); // 编译错误
    };

    template<typename T, typename ...Types>
    struct Outer {
        template<Types... values>
        struct Inner {
            T member(values...); // 编译错误
        };
    };

折叠表达式
--------------------

使用C+17，一个被称作 **折叠表达式** 的新特性被添加。除了 ``.`` ， ``->`` 和 ``[]`` 之外，其他所有的二元操作符都能适用。

给定一个未展开的表达式模式 pack 和一个非模式的表达式 value，C++17允许我们为任何此类操作符 op 编写如下模式的代码：

.. code-block:: c++

    (pack op ... op value) // 右折叠（binary right fold）
    (value op ... op pack) // 左折叠（binary left fold）

请注意，括号是必不可少的。

.. code-block:: c++

    template<typename... T>
    bool g() {
        return (trait<T>() && ... && true);
    }

折叠表示也是包扩展。注意，如果 pack 为空，折叠表达式的类型仍可以由 non-pack 操作数（上述形式中的 value）确定。

然而，该特性的设计者也需要一个省略操作数 value 的可选项。因此，其他两种形式在C++17也是可行的：

.. code-block:: c++

    (pack op ...) // unary right fold
    (... op pack) // unary left fold

明显地，这给空表达式带来了问题：我们怎么确定它们的类型和值？答案是，unary fold使用空表达式时一般是错误的，除了以下三种例外：

- && 的一元折叠传入空表达式时产生 true 值

- || 的一元折叠传入空表达式时产生 false 值

- 逗号操作符（,）的一元折叠传入空表达式时产生 void 表达式

重载这些特殊的操作符可能会产生让你吃惊的结果：

.. code-block:: c++

    struct BooleanSymbol {
        ...
    };

    BooleanSymbol operator ||(BooleanSymbol, BooleanSymbol);

    template<typename... BTs>
    void symbolic(BTs... ps) {
        BooleanSymbol result = (ps || ...);
        ...
    }

对于空表达式，会产生一个布尔值；而对于其他扩展，会产生一个 BooleanSymbol 类型的值。因此必须谨慎使用一元折叠表达式，反之，推荐
使用二元折叠表达式。

友元
============

友元声明的基本思想很简单：在友元声明出现的类中，识别出该类中有特权的类或者函数。然而，由于以下两个事实，时间稍微有点复杂：

- 友元声明可能是实体的唯一声明

- 友元声明可以是定义

类模板的友元类
----------------

友元类声明不能是定义，因此很少有问题。在模板的上下文中，友元类模板唯一的新方面是命名一个类模板的特定实例作为友元：

.. code-block:: c++

    template<typename T>
    class Node;

    template<typename T>
    class Tree {
        friend class Node<T>;
        ...
    };

注意，类模板必须在它的实例作为类或者类模板的友元的地方可见。使用普通类时，没有这样的需求：

.. code-block:: c++

    template<typename T>
    classTree {
        friend class Factory; // 即使前面未声明 Factory 也没问题
        friend class Node<T>; // 如果前面未声明 Node 会导致编译错误 
    };

声明其他类模板实例为友元：

.. code-block:: c++

    template<typename T>
    class Stack {
    public:
        ...
        template<typename>
        friend class Stack;
    };

C++11添加了将模板参数声明为友元的语法：

.. code-block:: c++

    template<typename T>
    class Wrap {
        friend T;
        ...
    };

对于任何类型都是合法的，对于实际上不是类类型的 T 会被忽略。

类模板的友元函数
---------------------

函数模板的一个实例可以成为一个友元，只要确保友元函数的名称后面紧接着尖括号。尖括号包含模板实参，但如果实参可以
被推导出来，尖括号可以是空的：

.. code-block:: c++

    template<typename T1, typename T2>
    void combine(T1, T2);

    class Mixer {
        friend void combine<>(int&, int&); // 没问题：T1 = int&, T2 = int&
        friend void combine<int, int>(int, int); // 没问题：T1 = int, T2 = int
        friend void combine<char>(char, int); // 没问题：T1 = char, T2 = int
        friend void combine<char>(char&, int); // 错误：无法匹配 combine() 模板
        friend void combine<>(long, long) { ... } // 错误：不允许定义
    };

注意，我们不能定义一个模板实例（至多可以定义特化），因此，命名一个实例的友元声明不能是定义。

如果名称后面不是紧接着尖括号，存在两种可能情况：

- 如果名称没有限定符（换句话说，名称中不包含 :: ），它绝不会指向一个模板实例。如果在友元声明的地方，
  没有匹配的非模板函数可见，那么该友元声明是该函数的第一个声明。声明也可以是一个定义。

- 如果名称包含限定符（名称包含 :: ），该名称必须指向前面声明过的函数或者函数模板。优先匹配函数而不是函数模板。
  然而，这样的友元声明不能是定义。

.. code-block:: c++

    void mutiply(void *); // 普通函数

    template<typename T>
    void mutiply(T);  // 函数模板

    class Comrades {
        friend void mutiply(int) {} // 定义一个新的函数：::mutiply(int)

        friend void ::mutiply(void*); // 指向上面的普通函数，而不模板实例 mutiply<void*>(void*)

        friend void ::mutiply(int); // 指向模板的一个实例

        friend void ::mutiply<double*>(double*); //指向模板的一个实例

        friend void ::error() {} // 错误：带限定符的友元不能是定义
    };

类模板中声明友元也应用相同的规则，但是模板参数可能会参与识别将成为友元的函数:

.. code-block:: c++

    template<typename T>
    class Node {
        Node<T>* allocate();
        ...
    };

    template<typename T>
    class List {
        friend Node<T>* Node<T>::allocate();
        ...
    };

友元函数也可以在类模板中被定义，在这种情况下，仅当它实际上被使用时才会被实例化。该函数会在类模板所在的命名空间作用域实例化。

.. code-block:: c++

    template<typename T>
    class Creator {
        friend void feed(Creator<T>) {
            ...
        }
    };

    int main()
    {
        Creator<void> one;
        feed(one); // 实例化 ::feed(Creator<void>)

        Creator<double> two;
        feed(two); // 实例化 ::feed(Creator<double>)
    }

在这个例子中， Creator 的每个实例生成一个不同的函数。注意，即使这些函数作为模板实例化的一部分被生成，但函数本身是普通函数，不是模板的
实例。然而，它们被认为是模板的实体，并且它们的定义仅在使用时被实例化。同时请注意，由于函数在类定义中被定义，因此它们隐式内联。

友元模板
---------------
通常，当将函数模板或者类模板的实例声明为友元时，我们可以精确地表示那个实体将成为友元。尽管如此，有时表示一个模板的所有实例为类的友元是
有用的。这就需要 **友元模板** 。

.. code-block:: c++

    class Manager {
        template<typename T>
        friend class Task;

        template<typename T>
        friend void Schedule<T>::dispatch(Task<T>*);

        template<typename T>
        friend int ticket() {
            return ++Manager::counter;
        }
        static int counter;
    };

就像普通友元声明一样，仅当友元模板的名称不包含限定符，并且名称后没有紧接尖括号时，友元模板才可以是一个定义。

友元模板只能声明为主模板和主模板的成员。任何和主模板相关联的偏特化和显示特化也被自动地认为是友元。


