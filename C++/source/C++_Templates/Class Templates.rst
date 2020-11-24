类模板
==================

与函数模板类似，类也能被一个或者多个类型进行参数化，容器类就是一种典型的类模板。

实现类模板 **Stack**
-----------------------------

基于 vector 实现 **Stack** 类模板。

.. literalinclude:: codes/basics/stack1.hpp
    :language: c++
    :linenos:

声明类模板
+++++++++++++++++

声明类模板的方式和声明函数模板类似。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        ...
    };

.. important:: 

    当类模板作为名称使用时，使用类名称；如果作为类型使用时，则需要在类名称后面添加
    模板实参，但是在模板类内部可以省略模板实参。

以下两个模板是等价的，推荐使用第一种方式。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        ....
        Stack(Stack const&); // copy constructor
        Stack& operator=(Stack const&) // assignment operator
    };

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        ....
        Stack(Stack<T> const&); // copy constructor
        Stack<T>& operator=(Stack<T> const&) // assignment operator
    };

实现成员方法
+++++++++++++++++

成员方法既可以在类模板内部实现，此时会默认内联；也可以实现类模板外部。

.. code-block:: c++
    :linenos:
    
    template<typename T>
    class Stack {
        ...

        void push(T const &elem) {
            elems.push_back(elem); // append copy of passed elem
        }

        ...
    }

.. code-block:: c++
    :linenos:

    template<typename T>
    void Stack<T>::push(T const &elem) {
        elems.push_back(elem); // append copy of passed elem
    }

使用类模板 **Stack**
---------------------------

在 C++17 之前，使用类模板必须显示指定模板实参。成员方法仅在被使用
后才会被实例化，因此 ``Stack<int>`` 的 ``pop`` 方法不会被实例化。

.. literalinclude:: codes/basics/stack1test.cpp
    :language: c++
    :linenos:

类模板的局部使用
-----------------------

类模板通常会在模板实参上应用多个操作，但是仅仅要求模板实参提供所有必须的操作即可（只有被调用过的
成员方法才会被实例化）。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        ...
        void printOn(std::ostream& strm) const {
            for(T const& elemt : elems) {
                strm << elem << ' '; // call << for each element
            }
        }
    };

如果给 **Stack** 添加 **printOn** 成员方法，我们还是可以使用未定义 operator<< 的型别实例化类模板。

.. code-block:: c++
    :linenos:

    Stack<std::pair<int, int>> ps; // note: std::pair has no operator<< defined
    ps.push({1, 4});
    ps.push({5, 6});
    std:: << ps.top().first << '\n';
    std:: << ps.top().second << '\n';

    ps.printOn(std::cout); // ERROR: operator<< not supported for element type

Concepts
+++++++++++++++

我们怎么去表示一个模板实例化时需要提供哪些操作呢？ **Concept** 经常用来表示一组模板实例化时所必须的
约束。比如 ``ramdom access iterator`` 和 ``default constructible`` 。

友元
--------------

通常 operator<< 通常使用友元的方式实现，以下重载 operator<< 并不是一个函数模板，并且只有在该函数
被调用时才会实例化。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        ...
        void printOn(std::ostream& strm) const {
            for(T const& elemt : elems) {
                strm << elem << ' '; // call << for each element
            }
        }

        friend std::ostream& operator<< (std::ostream& strm, Stack const& s) {
            s.printOn(strm);
            return strm;
        }
    }; 

如果友元的声明与定义分开，情况则会变得很复杂。实际上，我们有两种选择：

1. 将友元声明为函数模板。

    .. code-block:: c++
        :linenos:

            template<typename T>
            class Stack {
                ...

                template<typename U>
                friend std::ostream& operator<< (std::ostream& strm, Stack<U> const& s);
            }; 

            template<typename U>
            std::ostream &operator<<(std::ostream &os, const Stack<U> &s) {
                s.printOn(strm);
                return os;
            }

2. 通过前向声明将 operator<< 声明为函数模板。

    .. code-block:: c++
    
            template<typename T>
            class Stack;

            // forward declare operator<< template
            template<typename T>
            std::ostream& operator<< (std::ostream&, Stack<T> const&);

            template<typename T>
            class Stack {
                ...

                friend std::ostream& operator<< <T> (std::ostream& strm, Stack const& s);
            }; 

            template<typename U>
            std::ostream &operator<<(std::ostream &os, const Stack<U> &s) {
                s.printOn(strm);
                return os;
            }
    
    .. note:: 

        请注意 operator<< 后面的 <T> 是不可以省略的，它表明了我们声明了一个 operator<< 模板的
        特化作为友元。如果没有 <T> ，我们则声明了一个新的非模板的函数。

类模板特化
---------------

我们可以使用特定的实参型别特化一个类模板，特化允许你对特定的型别进行优化，或者修复某些型别的类模板实例
的非良构行为。

你可以单独特化类模板的某个成员方法，此时你可以针对特定的型别的模板参数的成员方法重新实现。

.. code-block:: c++
    :linenos:

    template<>
    const int& Stack<int>::top() const {
        std::cout << "call integer top\n";
        return elems.back();
    }

    Stack<int> intStack;
    intStack.push(10); // call primary template's push member method
    intStack.top(); // call top's specialization for int

此时对 Stack 类模板的 **int** 型别的 top 成员方法进行了特化， 其他的成员方法和主模板一样。

.. code-block:: c++
    :linenos:

    template<>
    class Stack<int> {
    private:
        std::deque<int> elems; // use deque as the underlying container
    public:
        void push(int const& );
        int const& top() const;
    };

    void Stack<int>::push(const int &elem) {
        elems.push_back(elem);
    }

    const int & Stack<int>::top() const {
        return elems.back();
    }

    int main()
    {
        Stack<int> intStack;
        intStack.push(10);
        intStack.top();

        intStack.pop(); // ERROR: unimplemented member method
    }

使用特定的型别特化类模板就像定义一个新的类一样，你可以改变成员变量，也可以改变成员方法的接口，你甚至可以
删除某些不必要的成员方法，或者添加额外的成员方法，除了名称与主模板一样外，其他的地方可以与主模板完全不一样。

偏特化
----------

类模板可以被部分特化，此时该特化还是一个类模板。与全特化类似，我们可以将偏特化的类模板实现的与主模板完全不同。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack<T*>
    {
    private:
        std::deque<T*> elems;
    public:
        void push(T const* );
        T const* top() const;
        void removeAll();
    };

    template<typename T>
    void Stack<T *>::push(const T *elem) {
        elems.push_back(elem);
    }

    template<typename T>
    T const* Stack<T *>::top() const {
        return elems.back();
    }

    template<typename T>
    void Stack<T *>::removeAll() {
        elems.clear();
    }

带多个模板参数的偏特化
+++++++++++++++++++++++++++

类模板也可以用来特化多个模板参数之间的关系。

.. code-block:: c++
    :linenos:

    // primary template
    template<typename T1, typename T2>
    class MyClass {
        ...
    };

    // partial specialization: both template parameters have same type
    template<typename T>
    class MyClass<T, T> {
        ...
    };

    // partial specialization: second type is int
    template<typename T>
    class MyClass<T, int> {
        ...
    };

    // partial specialization: both template parameters are pointer types
    template<typename T1, template T2>
    class MyClass<T1 *, T2 *> {
        ...
    };

    MyClass<int, float> m1; // match MyClass<T1, T2>
    MyClass<float, float> m2; // match MyClass<T, T>
    MyClass<float, int> m3; // match MyClass<T, int>
    MyClass<int *, float *> m4; // match MyClass<T1 *, T2 *>

如果匹配多个模板偏特化，则会产生编译错误。

.. code-block:: c++

    MyClass<int *, int *> m5; // ERROR: match MyClass<T, T> and MyClass<T1 *, T2 *>

此时可以通过提供一个额外的偏特化来解决歧义。

.. code-block:: c++
    :linenos:

    // partial specialization: both template parameters are pointers of the same type
    template<typename T>
    class MyClass<T *, T *> {
        ...
    };

默认类模板实参
-----------------------

与函数模板一样，你也可以给类模板添加模板参数添加默认值。

.. literalinclude:: codes/basics/stack3.hpp
    :language: c++
    :linenos:

类型别名
--------------

Typedefs 和别名声明
+++++++++++++++++++++++++++

1. 使用 **typedef** 

.. code-block:: c++

    typedef Stack<int> IntStack; // typedef
    void foo(IntStack const&); 
    IntStack stackArray[10];

2. 使用 **using** 

.. code-block:: c++

    using IntStack = Stack<int>; // alias declaration
    void foo(IntStack const&); 
    IntStack stackArray[10];

这两种情况我们只是给已存在的类型定义了一个新的名称，而不是定义了一个新的类型。因此 ``IntStack``
和 ``Stack<int>`` 是可以互换的。

别名模板
+++++++++++++++++

typedef 不支持模板，而别名声明可以通过模板为一组类型提供一个便利的名称。

.. code-block:: c++

    template<typename T>
    using DequeStack = Stack<T, std::deque<T>>;

.. important:: 

    模板只能声明和定义在全局作用域、命名空间已经类声明内，而不能在函数内或者块作用域内声明和定义模板。

成员型别的别名模板
++++++++++++++++++++++

别名模板可以方便给类模板的成员型别定义简短的名称。

.. code-block:: c++

    template<typename T>
    struct MyType {
        typedef ... iterator;
    };

    // alias template for iterator inside MyType
    template<typename T>
    using MyTypeIterator = typename MyType<T>::iterator;

    // use alias template declare variable
    MyTypeIterator<int> pos;

型别特征后缀 _t
+++++++++++++++++++++++++

C++14 使用别名模板定义简化了型别特征。

.. code-block:: c++

    namespace std  {
        template<typename T>
        using add_const_t = typename add_const<T>::type;
    }

    std::add_const_t<T>    // since C++14
    typename add_const<T>::type    //since C++11

类模板实参推导（Class Tempalete Arguments Deduction, CTAD）
------------------------------------------------------------------

在 C++17 之前，你必须显示指定类模板的所有模板参数（默认模板参数除外）。如果构造方法能够
用来推导所有的模板参数，C++17 允许你不指定模板实参。

.. code-block:: c++

    Stack<int> intStack1;
    Stack<int> intStack2 = intStack1; // OK in all version
    Stack intStack2 = intStack1; // OK since C++17

通过提供一个单参的构造方法，CTAD 可以推导出 Stack 类模板的模板实参型别。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        private:
            std::vector<T> elems; 
        public:
            Stack() = default;
            Stack(T const& elem) : elems({elem}) {} // initialize stack with one element
        ...
    };

    Stack intStack = 0; // Stack<int> deduced since C++17


字符串字面量的类模板实参推导
+++++++++++++++++++++++++++++++

.. code-block:: c++

    Stack stringStack = "bottom"; // Stack<char const[7]> deduced since C++17

当构造方法的形参声明为引用型别时，字符串字面量会推导为数组型别，这会导致存储
生成 ``std::vector<const char[7]>`` 的成员变量，而 C++ 不支持用一个数组构造另一个
数组，因而会导致编译错误。

我们可以考虑包构造方法的形参型别声明为按值传递，因而字符串字面量会退化为指针型别。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        private:
            std::vector<T> elems; 
        public:
            Stack(T elem) : elems({elem}) {} // pass by value allow decay
        ...
    };

    Stack stringStack = "bottom"; // Stack<char const*> deduced since C++17

推导指引（Deduction Guides）
++++++++++++++++++++++++++++++++++++++++++++

推导指引能够让你引导编译器推导出合适的模板实参型别。推导指引必须和类模板在同一作用域，一般放在
类模板定义后面。

.. code-block:: c++
    :linenos:

    template<typename T>
    class Stack {
        private:
            std::vector<T> elems; 
        public:
            Stack(T elem) : elems({elem}) {} // pass by value allow decay
        ...
    };

    Stack(const char*) -> Stack<std::string>; // must appear at the same scope of Stack

    Stack stringStack{"bottom"}; // OK: Stack<std::string> deduced since C++17
    
    // try convert "bottom" into Stack<std::string> failed
    Stack stringStack = "bottom"; // Stack<std::string> deduced, but still not valid


模板聚合（Templatized Aggregates）
-----------------------------------

聚合类是指满足以下条件的 class 和 struct ：

    1. 没有用户提供的或者继承的构造函数
    2. 没有私有的或者受保护的非静态数据成员
    3. 没有虚方法
    4. 没有虚继承、私有继承和 protect 继承

聚合类也可以是模板。

.. code-block:: c++

    template<typename T>
    struct ValueWithComment {
        T value;
        std::string comment;
    };  

    ValueWithComment<int> vc;
    va.value = 42;
    vc.comment = "initial value";

    //use C++17 Deduction Guides
    ValueWithComment(char const*, char const*)
        -> ValueWithComment<std::string>;
    ValueWithComment vc2 = {"hello", "initial value"};

