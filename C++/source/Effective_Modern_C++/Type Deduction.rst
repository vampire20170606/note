型别推导
====================

模板型别推导
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

C++98 仅有一套型别推导规则，即函数模板实参推导；C++11 增加了两套规则，即 ``auto`` 和
``decltype``， C++14 增加了decltype(auto) .

函数模板型别推导
-----------------------

.. code-block:: c++

    template<typename T>
    void f(ParamType param)

    f(expr) // 从 expr 来推导 T 和 ParamType 的型别

上述代码片段设计三个型别， ``expr`` 的型别、``T`` 的型别以及 ``ParamType`` 的型别。
``T`` 和 ``ParamType`` 这两个型别往往不一样，以下分三种情况来讨论型别推导规则。

情况1：ParamType 是指针或者引用，但不是万能引用
+++++++++++++++++++++++++++++++++++++++++++++++

1. 若 expr 具有引用类型，先忽略引用部分；
2. 然后，对 expr 的型别和 ParamType 的型别执行模式匹配，来决定 T 的型别。

.. code-block:: c++


    #include <iostream>
    #include <typeinfo>

    template<typename T>
    struct TypePrinter
    {
        static void print() {
            std::cout << typeid(T).name() << " ";
        }
    };

    template<typename T>
    struct TypePrinter<const T>
    {
        static void print() {
            std::cout << "const ";
            TypePrinter<T>::print();
        }
    };

    template<typename T>
    struct TypePrinter<T &>
    {
        static void print() {
            TypePrinter<T>::print();
            std::cout << "& ";
        }
    };

    template<typename T>
    struct TypePrinter<T &&>
    {
        static void print() {
            TypePrinter<T>::print();
            std::cout << "&& ";
        }
    };

    template<typename T, int N>
    struct TypePrinter<T [N]>
    {
        static void print() {
            TypePrinter<T>::print();
            std::cout << "[N]";
        }
    };

    template<typename T, int N>
    struct TypePrinter<const T [N]>
    {
        static void print() {
            std::cout << "const ";
            TypePrinter<T>::print();
            std::cout << "[N]";
        }
    };

    template<typename T>
    struct TypePrinter<T *>
    {
        static void print() {
            TypePrinter<T>::print();
            std::cout << "* ";
        }
    };

    template<typename T>
    void f(T& param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        int x = 27;
        const int cx = x;
        const int&rx = x;
        f(x);  // T => int, ParamType => int&
        f(cx); // T => const int, ParamType => const int&
        f(rx); // T => const int, ParamType => const int&
    }

ParamType 添加CV修饰符时的类型推导。

.. code-block:: c++

    template<typename T>
    void f(const T& param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        int x = 27;
        const int cx = x;
        const int&rx = x;
        f(x);  // T => int, ParamType => int&
        f(cx); // T => int, ParamType => const int&
        f(rx); // T => int, ParamType => const int&
    }

指针的型别推导与引用一致。

.. code-block:: c++

    template<typename T>
    void f(T* param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        int x = 27;
        int *px = &x;
        const int*cpx = &x;
        f(px); // T => int, ParamType => int*
        f(cpx); // T => const int, ParamType => const int*
    }


情况2：ParamType 是万能引用
+++++++++++++++++++++++++++++++++++++++++++++++

1. 如果 expr 是左值， T 和 ParamType 都会被推导为左值引用。
2. 如果 expr 是右值，则应用规则情况1中的规则

.. code-block:: c++

    template<typename T>
    void f(T&& param);


.. code-block:: c++

    template<typename T>
    void f(T&& param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        int x = 27;
        const int cx = x;
        const int& rx = x;
        f(x); // T => int&, ParamType => int&
        f(cx); // T => const int&, ParamType => const int&
        f(rx); // T => const int&, ParamType => const int&
        f(3); // T => int, ParamType => int&&
    }

情况3：ParamType 既非指针也非引用
+++++++++++++++++++++++++++++++++++++++++++++++

此时我们处理的是按值传递，所以 param 将会是传入对象的一个副本，即一个全新的对象。
所以在进行类型推导的时候将忽略 expr 型别的引用、CV修饰符属性。

.. code-block:: c++

    template<typename T>
    void f(T param); // param 按值传递

.. code-block:: c++

    template<typename T>
    void f(T param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        int x = 27;
        const int cx = x;
        const int& rx = x;
        f(x); // T => int, ParamType => int
        f(cx); // T => int, ParamType => int
        f(rx); // T => int, ParamType => int
        f(3); // T => int, ParamType => int

        int *px = &x;
        int const *pcx = &x;
        int const *const pccx = &x;
        f(px); // T => int*, ParamType => int*
        f(pcx); // T => const int*, ParamType => const int*
        f(pccx); // T => const int*, ParamType => const int*
    }

情况4：数组实参
+++++++++++++++++++++++++++

.. code-block:: c++

    // 以下两个模板等价，因为 C++ 允许数组类型退化为指针类型
    template<typename T>
    void f(T param[]) {
        TypePrinter<T>::print();
    }

    template<typename T>
    void f(T* param) {
        TypePrinter<T>::print();
    }

因此，当形参为数组类型时，其处理方式和指针的处理方式一致。

.. code-block:: c++

    template<typename T>
    void f(T param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        const int a[10] = {0};
        f(a); // T => const int*, ParamType => const int*
    }

.. code-block:: c++

    template<typename T>
    void f(T param[]) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        const int a[10] = {0};
        f(a); // T => const int, ParamType => const int*
    }

当形参为数组引用时。

.. code-block:: c++

    // 普通引用
    template<typename T>
    void f(T& param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        const int a[10] = {0};
        f(a); // T => const int[10], ParamType => const int (&) [10]
    }

    // 万能引用
    template<typename T>
    void f(T&& param) {
        TypePrinter<T>::print();
    }

    int main(int argc, char *argv[])  {
        const int a[10] = {0};
        f(a); // T => const int (&) [10], ParamType => const int (&) [10]
    }

    template<typename T, std::size_t N>
    constexpr std::size_t arraySize(T (&)[N]) noexcept {
        return N;
    }

    int main(int argc, char *argv[])  {
        const int a[10] = {0};
        static_assert(arraySize(a) == 10 && "a's size equal 10.");
    }


情况4：函数实参
+++++++++++++++++++++++++++

函数也可以退化为指针，所以它的推导规则与数组型别一致。


auto 型别推导
~~~~~~~~~~~~~~~~~~~~~~~~~~~

在进行模板型别推导时，我们使用如下的模板形式：

.. code-block:: c++

    template<typename T>
    void f(ParamType param)

    f(expr)

当使用 auto 来声明变量时， auto 扮演了 T 的角色，而变量的型别扮演了 ParamType 的角色。

.. code-block:: c++

    auto x = 27;

    // auto x = 27; 概念上等价的函数模板
    template<typename T>
    void func_for_x(T param);
    
    func_for_x(27);

    auto& rx = x;

    // auto& rx = x; 概念上等价的函数模板
    template<typename T>
    void func_for_rx(T& param);
    
    func_for_rx(x);

    auto&& uref = 27;

    // auto&& uref = 27; 概念上等价的函数模板
    template<typename T>
    void func_for_uref(T&& param);
    
    func_for_uref(x);

C++98 有如下两种初始化语法

.. code-block:: c++
    int x1 = 27;
    int x2(27);

C++11 增加了两种初始化语法，称作统一初始化

.. code-block:: c++
    int x3 = {27};
    int x4{27}; // 该初始化语法不允许窄化

    int b = 10;
    char a1{b}; // compile error
    char a2(b); // warning, no error

当使用 auto 推导使用统一初始化语法的变量型别时，会将型别推导为一个 std::initializer_list， 但是模板
推导却不会。

.. code-block:: c++

    auto x{5} // 型别为 std::initializer_list<int>， 值为 {5}
    auto x1 = {11, 12, 13} // 型别为 std::initializer_list<int> 

    template<typename T>
    void f(T param);
    f({11, 12, 13}); // 错误，无法推导 T 的型别

    template<typename T>
    void f(std::initializer_list<T> param);
    f({11, 12, 13}); // work

C++14 允许使用 auto 来说明函数返回值需要推导，而且增加了 lambda 在形参上使用 auto 来说明参数型别需要
推导，然而这些 auto 用法使用的是模板型别推导而非 auto 型别推导，因而无法处理大括号初始化表达式。

.. code-block:: c++

    // C++14
    auto f() {
        return 10; // auto => int
    }

    auto p = [](const auto& v) -> auto { return 10;};

decltype
~~~~~~~~~~~~~~~~~~

对于给定的名字或表达式，decltype 能告诉你改名字或表达式的类型。decltype 和 sizeof 一样，不会计算表达式的值。

.. code-block:: c++

    const int i{0}; // decltype(i) == const int

    bool f(const Widget& w); // decltype(w) == const Widget&
                            // decltype(f) == bool f(const Widget& )

    struct Point { // decltype(Point::x) == int
        int x, y; // decltype(Point::y) == int
    };

    Widget w; // decltype(w) == Widget

    if(f(w)) ... // decltype(f(w)) == bool

    template<typename T>
    class vector {
        public:
            T& operator[](std::size_t index);
    }

    vector<int> v; // decltype(v) == vector<int>
    if(v[0] == 0) ... // decltype(v[0]) == int&

    // C++11
    template<typename Container, typename Index>
    auto f(Container &c, Index i) -> decltype(c[i]) // C++11 返回值型别尾序语法
    {
        return c[i];
    }

    // C++14 返回值推导
    template<typename Container, typename Index>
    auto f(Container &c, Index i) // 依据模板型别推导规则，当 c[i] 返回 T& 时， 推导出来的返回型别为 T
    {
        return c[i];
    }

    // C++14 返回值推导
    template<typename Container, typename Index>
    decltype(auto) f(Container &c, Index i) // 此时推导出来的型别与 c[i] 的型别一致
    {
        return c[i];
    }

    // C++14
    template<typename Container, typename Index>
    decltype(auto) f(Container &&c, Index i) // 通过万能引用使得能够同时处理左值和右值得情况
    {
        return std::forward<Container>(c)[i]; // 时有 forward 保留 c 的原始值语义
    }

    int b1 = 10; 
    int b2 = 10;

    // decltype(b1) == int
    // decltype((b1)) == int& // 左值表达的括号表达式返回左值引用
    // decltype(b1 + b2) == int
    // decltype((b1 + b2)) == int // 右值表达式则不受影响
    
       