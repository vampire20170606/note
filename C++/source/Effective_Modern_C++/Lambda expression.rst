lambda 表达式
===============================

.. code-block:: c++

    int a;
    auto p1 = [=](int value) { return a < value;};
    auto p2 = [&](int value) { return a < value;};
    auto p3 = [a](int value) { return a < value;};
    auto p4 = [&a](int value) { return a < value;};
    auto p5 = [&a](int value) -> bool { return a < value;};

    // 捕获 this 指针
    class Widget {
    public:
        void nothing() {
            auto p4 = [this](int value) { return mm < value;}
        }
    private:
        int mm;
    }
    
    // C++14 初始化捕获
    auto p6 = [pw =std::make_unique<Widget>()] {
        return pw->isValid();
    }

    // C++14 泛型 lambda
    auto p7 = [](auto x) {
        return x;
    }

    // 上述模板与以下匿名类功能一样
    class anonymous_class {
    public:
        template<typename T>
        auto operator(T x) {
            return x;
        }
    private:
        // 对应捕获列表
    } 

    auto p8 = [](auto&& x) {
        return f(std::forward<decltype(x)>(x));
    }
