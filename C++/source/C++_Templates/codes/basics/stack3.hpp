#include <vector>
#include <cassert>
#include <deque>

template<typename T, typename Cont = std::vector<T>>
class Stack {
private:
    Cont elems;  // elements

public:
    void push(T const& elem); // push element
    void pop();
    T const& top() const; // return top element
    bool empty() const {  // return whether the stack is empty
        return elems.empty();
    }
};

template<typename T, typename Cont>
void Stack<T, Cont>::push(const T &elem) {
    elems.push_back(elem); // append copy of passed elem
}

template<typename T, typename Cont>
void Stack<T, Cont>::pop() {
    assert(!elems.empty());
    elems.pop_back();
}

template<typename T, typename Cont>
const T & Stack<T, Cont>::top() const {
    assert(!elems.empty());
    return elems.back();
}

int main()
{
    Stack<int> intStack;
    intStack.push(10);

    Stack<int, std::deque<int>> dintStack;
    dintStack.push(10);
}
