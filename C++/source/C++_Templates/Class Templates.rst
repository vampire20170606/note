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

