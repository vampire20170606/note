Tuple 实现
==============================

.. code-block:: c++

    // primary template
    template<bool AssertValue, typename... T>
    struct Tuple;

    // Explicit Specialization
    template<bool AssertValue, typename T>
    struct Tuple<AssertValue, T> {
        static_assert(AssertValue, "Tuple<T>");
        Tuple() : val() {}
        Tuple(T const& v) : val(v) {}

        T val;
    };

    template<bool AssertValue>
    struct Tuple<AssertValue>
    {

    };

    // type trait
    template<bool AssertValue,  typename T, typename ...Ts>
    struct Tuple<AssertValue, T, Ts...> : Tuple<AssertValue, Ts...> {
        T val;
        Tuple() : val(), Tuple<AssertValue, Ts...>(){}
        explicit Tuple(const T& v, const Ts &...args) : val(v), Tuple<AssertValue, Ts...>(args...) {}
    };


    template<unsigned Index, bool AssetValue,  typename ...Ts>
    struct TupleTrait;

    template<unsigned Index, bool AssertValue, typename T, typename ...Ts>
    struct TupleTrait<Index, AssertValue, Tuple<AssertValue, T, Ts...>> {
        using value_type = typename TupleTrait<Index - 1, AssertValue,  Tuple<AssertValue, Ts...>>::value_type;
        using tuple_type = typename TupleTrait<Index - 1, AssertValue,  Tuple<AssertValue, Ts...>>::tuple_type;
    };

    template<bool AssertValue, typename T, typename ...Ts>
    struct TupleTrait<0, AssertValue, Tuple<AssertValue, T, Ts...>> {
        using value_type = T;
        using tuple_type = Tuple<AssertValue, T, Ts...>;
    };

    template<>
    struct TupleTrait<0, Tuple<true>>
    {
        using value_type = Tuple<true>;
        using tuple_type = Tuple<true>;
    };

    template<unsigned Index, typename ...Ts>
    typename TupleTrait<Index, true, Tuple<true, Ts...>>::value_type &get(Tuple<true, Ts...> &t) {
        using tuple_type = typename TupleTrait<Index, true, Tuple<true, Ts...>>::tuple_type;
        return static_cast<tuple_type *>(&t)->val;
    }
