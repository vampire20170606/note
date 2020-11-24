#include <array>
#include <cassert>

template<typename T, std::size_t MaxArraySize>
class Stack {
private:
    std::array<T, MaxArraySize> elems;
    std::size_t numElems;
public:
    Stack();
    void push(T const&);
    void pop();
    T const& top();
    bool empty() const {
        return numElems == 0;
    }

    std::size_t size() const {
        return numElems;
    }
};

template<typename T, std::size_t MaxArraySize>
Stack<T, MaxArraySize>::Stack() : numElems(0) {}

template<typename T, std::size_t MaxArraySize>
void Stack<T, MaxArraySize>::push(T const& elem) {
    assert(numElems < MaxArraySize);
    elems[numElems] = elem;
    ++numElems;
}

template<typename T, std::size_t MaxArraySize>
void Stack<T, MaxArraySize>::pop() {
    assert(!elems.empty());
    --numElems;
}

template<typename T, std::size_t MaxArraySize>
T const& Stack<T, MaxArraySize>::top() {
    assert(!elems.empty());
    return elems[numElems - 1];
}

