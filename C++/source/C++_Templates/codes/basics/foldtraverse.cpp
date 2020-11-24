#include <iostream>

struct Node {
    int value;
    Node* left;
    Node* right;
    Node(int i = 0) : value(i), left(nullptr), right(nullptr) {}
};

auto left = &Node::left;
auto right = &Node::right;

template<typename T, typename... Types>
Node* traverse(T np, Types... paths)
{
    return (np ->* ... ->* paths); // np->*path1->*path2
}

int main() {

    auto root = new Node{0};
    root->left = new Node{1};
    root->left->right = new Node{2};

    traverse(root, left, right);
    return 0;
}