智能指针
==================

unique_ptr
~~~~~~~~~~~~~~~

专属所有权，一般情况下不会增加内存占用空间，即与裸指针占用的空间一致。如果析构器是带状态的lambda
或者仿函数则会导致unique_ptr占用的空间比裸指针大。

.. code-block:: c++

    std::unique_ptr<int> p {new int};
    std::unique_ptr<int> p = new int; // 编译错误
    
    // 修改默认析构器
    auto del = [](int *p) {
        std::cout << "delete " << p << "\n";
        delete p;
    };
    std::unique_ptr<int, decltype(del)> p1 {new int, del}; 

shared_ptr
~~~~~~~~~~~~~~~

共享所有权, 一般情况下 shared_ptr 占用的存储空间是裸指针的两倍，一个是指向资源的指针，另一个是指向控制块的指针。  

.. code-block:: c++

    std::shared_ptr<int> sp1{new int};
    auto sp2 = sp1;

    auto del = [](int *p) {
        std::cout << "delete " << p << "\n";
        delete p;
    };

    std::shared_ptr<int> p1{new int, del};

    // 错误用法 
    auto pw = new int;
    std::shared_ptr<int> spw1{pw};
    std::shared_ptr<int> spw2{pw}; // 会导致 pw 被 delete 两次

weak_ptr
~~~~~~~~~~~~~~~~~~

能够检测 shared_ptr 是否失效，可以用于缓存失效、观察者模式、环形依赖。

.. code-block:: c++

    std::shared_ptr<int> sp{new int};
    std::weak_ptr<int> wp{sp};

    std::cout << wp.expired() << "\n"; // false
    sp = nullptr;
    std::cout << wp.expired() << "\n"; // true

    std::shared_ptr<int> sp2 = wp.lock(); // 如果已经失效，则返回空

make_unique 和 make_shared
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

    template<typename T, typename... Ts>
    auto make_unique(Ts&&... params)
    {
        return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
    }

    void process(std::shared_ptr<Widget> spw, int priority);
    int computePriority();

    process(std::shared_ptr<Widget>(new Widget), computePriority()); // 可能会发生内存泄漏

    process(std::make_shared<Widget>(), computePriority()); // 保证资源管理的原子性

