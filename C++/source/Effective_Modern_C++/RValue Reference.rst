右值引用、移动语义和完美转发
===================================


形参总是左值，即使其型别是右值引用。

.. code-block:: c++
    
    void f(Widget&& w); // w 是个左值

std::move 和 std::forward
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``std::move`` 和 ``std::forward`` 都是紧紧执行强制型别转换的函数模板。 ``std::move`` 无条件的将实参强制转换成右值，
而 ``std::forward`` 则仅当传入的实参被绑定到右值时，才实施向右值型别的强制型别转换。

.. code-block:: c++

    // move 返回的肯定是个右值
    template<typename T>
    typename std::remove_reference<T>::type&&
    move(T&& param)
    {
        using RetTy = typename std::remove_reference<T>::type&&;
        return static_cast<RetTy>(param);
    }

区分万能引用和右值引用
~~~~~~~~~~~~~~~~~~~~~~~~~~

``T&&`` 具有两种含义，一种是表示右值引用，会绑定到右值；另一种既可以表示右值引用，也可以表示左值引用，
即既可以绑定左值，也可以绑定右值，甚至绑定带CV修饰符的对象，称为万能引用（也叫转发引用）。

.. code-block:: c++

    void f(Widget&& param); // 右值引用

    Widget&& var1 = Widget(); // 右值引用

    template<typename T>
    void f(std::vector<T>&& param); // 右值引用

    // 万能引用出现的两种场景，都会涉及型别推导
    template<typename T>
    void f(T&& param); // 万能引用

    auto&& var2 = var1; // 万能引用

    template<typename... T>
    void f(T&&... param); // 涉及参数包的万能引用

    Widget w;
    f(w); // T => Widget&
    f(std::move(w)) // T => Widget&&

``std::forward`` 适用于万能引用，能够将传入参数的左值和右值属性传递给下一个被调用的方法，而 ``std::move`` 
适用于右值引用，从而将其强制转换成右值。

.. code-block:: c++

    // 标签派发
    void patchImpl(int a, std::true_type) {
        std::cout << "match integer.\n";
    }

    template<typename T>
    void patchImpl(T&& t, std::false_type) {
        std::cout << "match non-integer.\n";
    }

    template<typename T>
    void dispatch(T&& t) {
        patchImpl(std::forward<T>(t), std::is_integral<std::remove_reference_t<T>>());
    }

    dispatch("hello"); // match non-integer 
    dispatch('a'); // match integer

    // std::enable_if, SFINAE
    void dispatch(int )
    {
        std::cout << "match integer.\n";
    }

    template<typename T,
            typename = std::enable_if_t<!std::is_integral<std::remove_reference_t<T>>::value>>
    void dispatch(T&& )
    {
        std::cout << "match non-integer\n";
    }

    dispatch("hello"); // match non-integer 
    dispatch('a'); // match integer

引用折叠
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    template<typename T>
    T&& forward(std::remove_reference_t<T>& param)
    {
        return static_cast<T&&>(param)
    }

    // 引用折叠发生的四种语境

    // 1. 模板实例化
    template<typename T>
    T&& forward(std::remove_reference_t<T>& param)
    {
        return static_cast<T&&>(param)
    }

    // 2. auto 型别生成
    auto&& a = 10; // auto => int, decltype(a) => int&&

    // 3. typedef和using的别名声明
    template<typename T>
    using type = T&&;

    // 4. decltype
    decltype(10)&& mn = 10;
