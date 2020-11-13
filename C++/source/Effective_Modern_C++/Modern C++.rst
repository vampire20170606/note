转向现代 C++
=========================

C++11 和 C++14 的特性包括 auto、智能指针、移动语义、lambda表达式等。

初始化语法
~~~~~~~~~~~~~~

.. code-block:: c++

    // 初始化语法
    // C++98
    int x(0);
    int y = 0;
    // C++11
    int z{0};
    int w = {0};

    auto i{0}; // i => std::initializer_list<int>

    class Widget {
        ...

        private:
            int y{0}; // 可行
            int x = 0; // 可行
            int z(0); // 不可行，比如 using w = int; int z(w); 将解释为函数声明
    }

    // atomic 的拷贝构造和赋值运算符是禁止的
    std::atomic<int> a1 {0}; // 可行
    std::atomic<int> a2 (0); // 可行
    // 不可行，因为此时会调用拷贝构造函数，即首先调用单参数的默认构造函数将 0 转成 atomic， 然后调用拷贝构造
    std::atomic<int> a3 = 0; 

    // 内建类型隐式转换处理
    double x, y, z;
    int sum1{x + y + z}; // 编译错误
    int sum2(x + y + z); // 没问题
    int sum3 = x + y + z; // 没问题

    class Widget {
    public:
        Widget(int i, bool b);
        Widget(int i, double d);
        Widget(std::initializer_list<long double> i1);
    }

    Widget w1(10, true); // Widget(int i, bool b);
    Widget w1{10, true}; //  Widget(std::initializer_list<long double> i1), 10 和 true 强转为 long double

优先 nullptr，而非 0 或 NULL 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    // 函数重载
    void f(int);
    void f(bool);
    void f(void *);

    f(0); // 调用 f(int)
    f(NULL); // 取决于 NULL 具体的型别，但不会匹配 f(void *)
    f(nullptr); // 调用 f(void *)

    // 函数模板参数推导
    template<typename FuncType, typename PtrType>
    decltype(auto) f(FuncType func, PtrType ptr) {
        return func(ptr);
    }

    auto r1 = f(f1, 0); // 错误， PtrType => int
    auto r2 = f(f2, 0); // 错误， PtrType => decltype(NULL)
    auto r3 = f(f3, nullptr); // 正确， PtrType => nullptr_t, nullptr_t 可以转成任意指针类型
    
优先别名声明，而非typedef
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    // 声明类型别名
    typedef void (*FP)(int, float);
    using FP = void (*)(int, float);

    // 别名模板
    template<typename T>
    using MyAllocList = std::list<T, MyAlloc<T>>;
    MyAllocList<Widget> lw; 

    template<typename T>
    struct MyAllocList {
        typedef std::list<T, MyAlloc<T>> type;
    };
    MyAllocList<Widget>::type lw;

    // 使用嵌套 typedef
    template<typename T>
    class Widget {
    private:
        typename MyAllocList<T>::type list; // MyAllocList<T>::type 是带依赖型别，即 type 的解释依赖于 T 的具体型别，可以解释为类型，也可以解释为静态变量
    }

    // 使用模板别名
    template<typename T>
    class Widget {
    private:
        MyAllocList<T> list;
    }

优先限定作用域的枚举型别，而非不限定作用域的枚举型别
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    // 枚举常量泄漏到枚举型别所在作用域
    enum Color {Red, White, Black};
    auto Red = 1; // 错误

    enum class Color {Red, White, Black};
    auto Red = 1; // 没问题
    auto c = Color::Red;

    // enum class 禁止隐式转换
    enum Color {Red, White, Black};
    auto c = red;
    if(c < 14.5) {} // c 转成整数型别，进而转换到浮点型别

    enum class Color {Red, White, Black};
    auto c = Color::red;
    if(c < 14.5) {} // 错误，禁止转换
    if(static_cast<float>(c) < 14.5) {} // 强制转换

    // C++98 禁止枚举的前置声明
    enum Color;  // C++98 的编译器会报错，因为不知道枚举型别的潜在类型

    // C++11
    enum class Color; // 默认使用 int 作为潜在型别
    enum class Color : char; // 使用 char 作为潜在型别
    enum Color : char; // 使用 char 作为潜在型别
    enum Color; // 错误，不知道Color使用哪种型别作为潜在型别

优先选用删除函数，而非 private 未定义函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    // C++98 禁止使用编译自动生成的方法
    class Widget { // 对于友元和成员函数需要在链接阶段才能发现错误
    private:
        Widget(const Widget&);
        Widget& operator=(const Widget&);
    }

    // C++11 
    class Widget {
    public:
        Widget(const Widget&) = delete;
        Widget& operator=(const Widget&)  = delete;
    }

    // delete 可以应用到任何函数
    bool isLucky(int number);
    bool isLucky(char) = delete; // 拒绝 char 型别
    bool isLucky(bool) = delete; // 拒绝 bool 型别
    bool isLucky(double) = delete; // 拒绝 double/float 型别

    if(isLucky('a')) ... // 错误，调用了删除函数
    if(isLucky(true)) ... // 错误，调用了删除函数
    if(isLucky(3.5)) ... // 错误，调用了删除函数

    // 阻止不应该进行的模板具现化
    template<typename T>
    void processPointer(T *ptr);

    template<>
    void processPointer<void>(void *ptr) = delete; // 函数模板全特化

    class Widget {
    public:
        template<typename T>
        void processPointer(T *) {}
    private:
        template<>
        void Widget::processPointer<void>(void *); // 编译错误，无法在非namespace的作用域进行模板显示特化
    };

        class Widget {
    public:
        template<typename T>
        void processPointer(T *) {}
    };

    template<>
    void Widget::processPointer<void>(void *) = delete;

override 声明
~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    class Widget {
    public:
        void doWork() & {
            std::cout << "lvalue \n";
        }

        void doWork() && {
            std::cout << "rvalue \n";
        }
    };

    Widget w;
    w.doWork(); // 输出 lvalue
    Widget{}.doWork(); // 输出 rvalue

    class Widget {
    public:
        void doWork() const {
            std::cout << "const \n";
        }

        void doWork() {
            std::cout << "non-const \n";
        }
    };

    const Widget cw;
    Widget w;
    w.doWork(); // 输出 non-const
    cw.doWork(); // 输出 const

    // override 可以确保你是否按照预期重写相应的基类方法
    class Base {
    public:
        virtual void mf1() const;
        virtual void mf2() &;
    };

    class Derived : public Base {
    public:
        virtual void mf1() const override; // 没问题，和 Base 的虚函数签名一致
        virtual void mf2() && override; // 错误，Base 是左值，而 Derived 是右值
    };

    // final 禁止重写该方法，禁止派生
    class Base final {
    public:
        virtual void mf1() const;
    };

    class Derived : public Base { // 编译错误，Base 禁止被派生
    }; 

    // override 和 final 是语境关键字，即它们只在特定语境下是作为保留字。
    class Warn {
    public:
        void override(); // C++11 和 C++98 都是合法代码
        void final();
    }

constexpr
~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    constexpr auto int a = 10; // 编译时常量

    // 当传入编译时常量时，会在编译时计算结果；当传入值在运行时才知道时，就跟普通函数一样
    // C++11 constexpr 函数不能包含多余一条语句
    constexpr int pow(int base, int exp) noexcept 
    {
        return exp == 0 ? 1 : base * pow(base, exp - 1);
    }

    // C++14 移除了 constexpr 的限制
    constexpr int pow(int base, int exp) noexcept 
    {
        auto result = 1;
        for(int i = 0; i < exp; ++i) result *= base;
        return result;
    }
