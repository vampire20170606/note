llvm GraphTraits
######################

基于 GraphTraits 的接口实现通用的图算法。

GraphTraits 接口介绍
-----------------------------

llvm 通过 Traits 技巧给所有的图定义一套的接口，然后可以基于这套
接口可以实现与具体的图模型无关的算法，比如遍历算法等。通过特化的方式
提供每个具体图模型的某些接口（不一定要实现所有的接口，只有当你使用的算法
依赖相应的接口时才需要实现），然后基于这些接口实现一些通用的算法。

.. code-block:: c++
    :linenos:

    template<class GraphType>
    struct GraphTraits {
    // Elements to provide:

    // typedef NodeRef           - Type of Node token in the graph, which should
    //                             be cheap to copy.
    // typedef ChildIteratorType - Type used to iterate over children in graph,
    //                             dereference to a NodeRef.

    // static NodeRef getEntryNode(const GraphType &)
    //    Return the entry node of the graph

    // static ChildIteratorType child_begin(NodeRef)
    // static ChildIteratorType child_end  (NodeRef)
    //    Return iterators that point to the beginning and ending of the child
    //    node list for the specified node.

    // typedef  ...iterator nodes_iterator; - dereference to a NodeRef
    // static nodes_iterator nodes_begin(GraphType *G)
    // static nodes_iterator nodes_end  (GraphType *G)
    //    nodes_iterator/begin/end - Allow iteration over all nodes in the graph

    // typedef EdgeRef           - Type of Edge token in the graph, which should
    //                             be cheap to copy.
    // typedef ChildEdgeIteratorType - Type used to iterate over children edges in
    //                             graph, dereference to a EdgeRef.

    // static ChildEdgeIteratorType child_edge_begin(NodeRef)
    // static ChildEdgeIteratorType child_edge_end(NodeRef)
    //     Return iterators that point to the beginning and ending of the
    //     edge list for the given callgraph node.
    //
    // static NodeRef edge_dest(EdgeRef)
    //     Return the destination node of an edge.

    // static unsigned       size       (GraphType *G)
    //    Return total number of nodes in the graph

    // If anyone tries to use this class without having an appropriate
    // specialization, make an error.  If you get this error, it's because you
    // need to include the appropriate specialization of GraphTraits<> for your
    // graph, or you need to define it for a new graph type. Either that or
    // your argument to XXX_begin(...) is unknown or needs to have the proper .h
    // file #include'd.
    using NodeRef = typename GraphType::UnknownGraphTypeError;
    };

以下就是针对 clang 的 CFG 实现的 GraphTraits 的特化版本。

.. code-block:: c++
    :linenos:

    template <> struct GraphTraits< const ::clang::CFGBlock *> {
    using NodeRef = const ::clang::CFGBlock *;
    using ChildIteratorType = ::clang::CFGBlock::const_succ_iterator;

    static NodeRef getEntryNode(const clang::CFGBlock *BB) { return BB; }
    static ChildIteratorType child_begin(NodeRef N) { return N->succ_begin(); }
    static ChildIteratorType child_end(NodeRef N) { return N->succ_end(); }
    };

图的后序遍历
----------------

llvm 使用的迭代器的方式实现对图的后序遍历，主要依赖 ``child_begin`` 和 
``child_end`` 接口实现遍历算法。

首先该迭代器使用 po_iterator_storage 来实现对已经遍历过节点的记录，并且分别针对
存储的容器是由外部传入还是内部构建存储进行了特化，实现样例如下：

.. code-block:: c++
    :linenos:

    // 可以考虑集成一些定制化的接口来监视节点的遍历过程
    /// Default po_iterator_storage implementation with an internal set object.
    template<class SetType, bool External>
    class po_iterator_storage {
    SetType Visited;

    public:
    // Return true if edge destination should be visited.
    template <typename NodeRef>
    bool insertEdge(Optional<NodeRef> From, NodeRef To) {
        return Visited.insert(To).second;
    }

    // Called after all children of BB have been visited.
    template <typename NodeRef> void finishPostorder(NodeRef BB) {}
    };    

    /// Specialization of po_iterator_storage that references an external set.
    template<class SetType>
    class po_iterator_storage<SetType, true> {
    SetType &Visited;

    public:
    po_iterator_storage(SetType &VSet) : Visited(VSet) {}
    po_iterator_storage(const po_iterator_storage &S) : Visited(S.Visited) {}

    // Return true if edge destination should be visited, called with From = 0 for
    // the root node.
    // Graph edges can be pruned by specializing this function.
    template <class NodeRef> bool insertEdge(Optional<NodeRef> From, NodeRef To) {
        return Visited.insert(To).second;
    }

    // Called after all children of BB have been visited.
    template <class NodeRef> void finishPostorder(NodeRef BB) {}
    };

po_iterator 使用栈记录节点的遍历记录，方便进行回溯。po_iterator 实现了一个
traversChild 接口，首先从栈中弹出待遍历的节点，然后一直 travere 到没有子节点
或者子节点已经遍历过，最终栈上存放的是后序遍历的节点。

.. code-block:: c++
    :linenos:

    void traverseChild() {
        // while 条件表明当前节点已经没有子节点，作为当前后序遍历的节点
        while (VisitStack.back().second != GT::child_end(VisitStack.back().first)) {
        NodeRef BB = *VisitStack.back().second++;
            if (this->insertEdge(Optional<NodeRef>(VisitStack.back().first), BB)) {
                // 当图中存在环的时候，避免重复遍历
                // If the block is not visited...
                VisitStack.push_back(std::make_pair(BB, GT::child_begin(BB)));
            }
        }
    }

提供 begin 和 end 静态方法构建迭代器。

.. code-block:: c++
    :linenos:

    // Provide static "constructors"...
    static po_iterator begin(GraphT G) {
        return po_iterator(GT::getEntryNode(G));
    }
    static po_iterator end(GraphT G) { return po_iterator(); }

    static po_iterator begin(GraphT G, SetType &S) {
        return po_iterator(GT::getEntryNode(G), S);
    }
    static po_iterator end(GraphT G, SetType &S) { return po_iterator(S); }

po_iterator 是一个前向迭代器，因此只需要重载++运算符即可。++运算符需要遍历到下一个
后序遍历的节点。

.. code-block:: c++
    :linenos:

    po_iterator &operator++() { // Preincrement
        this->finishPostorder(VisitStack.back().first);
        VisitStack.pop_back();
        if (!VisitStack.empty())
        traverseChild();
        return *this;
    }

    po_iterator operator++(int) { // Postincrement
        po_iterator tmp = *this;
        ++*this;
        return tmp;
    }

通过函数模板实现型别推导。

.. code-block:: c++
    :linenos:
    
    template <class T>
    po_iterator<T> po_begin(const T &G) { return po_iterator<T>::begin(G); }
    template <class T>
    po_iterator<T> po_end  (const T &G) { return po_iterator<T>::end(G); }
