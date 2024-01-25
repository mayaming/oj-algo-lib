#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <iterator>
#include <cstddef>
#include <utility>
#include <algorithm>

using namespace std;

template<class K, class V, class Compare = less<K>>
class RBTree {
public:
    RBTree(): root(nullptr) {}
    virtual ~RBTree() {
        if (root != nullptr) delete root;
    }

    void insert(const K &k, const V &v) {
        if (this->root == nullptr) this->root = new Node(nullptr, k, v);
        else insert(root, k, v);
    }

    string to_graphviz() {
        string s = "digraph rb_tree {\n";
        vector<string> collect;
        if (root != nullptr) root->to_graphviz(collect);
        for (string &rel: collect) {
            s += "    " + rel + ";\n";
        }
        s += "}";
        return s;
    }

    struct Node {
        enum Dir { LEFT = -1, ROOT = 0, RIGHT = 1};
        enum Color { RED, BLACK };

        Node(Node *p, const K &k, const V &v): parent(p), kv(k, v), left(nullptr), right(nullptr) {}

        virtual ~Node() {
            if (left != nullptr) delete left;
            if (right != nullptr) delete right;
        }

        inline const K& key() const noexcept { return this->kv.first; }
        inline const V& value() const noexcept { return this->kv.second; }
        inline bool isLeaf() const noexcept { return this->left == nullptr && this->right == nullptr; }
        inline bool isRoot() const noexcept { return this->parent == nullptr; }
        inline bool isRed() const noexcept { return this->color == RED; }
        inline bool isBlack() const noexcept { return this->color == BLACK; }
        inline Node* sibling() const noexcept { return this->parent == nullptr ? nullptr : (this->dir() == LEFT ? this->parent->right : this->parent->left); }
        inline bool hasSibling() const noexcept { return this->sibling() != nullptr; }
        inline Node* uncle() const noexcept { return this->parent == nullptr ? nullptr : this->parent->sibling(); }
        inline bool hasUncle() const noexcept { return this->uncle() != nullptr; }
        inline Node* grandParent() const noexcept { return this->parent == nullptr ? nullptr : this->parent->parent; }
        inline Dir dir() const noexcept { return this->parent == nullptr ? ROOT : (this == this->parent->left ? LEFT : RIGHT); }

        Node* next() {
            Node *node = this;
            if (node->right != nullptr) {
                node = node->right;
                while (node->left != nullptr) node = node->left;
            }
            else {
                while (node->dir() == Dir::RIGHT) node = node->parent;
                node = node->parent;
            }
            return node;
        }

        Node* prev() {
            Node *node = this;
            if (node->left != nullptr) {
                node = node->left;
                while (node->right != nullptr) node = node->right;
            }
            else {
                while (node->dir() == Dir::LEFT) node = node->parent;
                node = node->parent;
            }
            return node;
        }

        void to_graphviz(vector<string> &collect) {
            if (left != nullptr) {
                collect.push_back(to_string(key()) + " -> " + to_string(left->key()));
            }
            if (right != nullptr) {
                collect.push_back(to_string(key()) + " -> " + to_string(right->key()));
            }
            if (left != nullptr) left->to_graphviz(collect);
            if (right != nullptr) right->to_graphviz(collect);
        }

        pair<K, V> kv;
        Node *parent, *left, *right;
        Color color = RED;
    };

    struct Iterator 
    {
    public:
        Iterator(): ptr(nullptr) {}
        Iterator(Node *p): ptr(p) {}
        Iterator(const Iterator &iter): ptr(iter.ptr) {}
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = pair<K, V>;
        using pointer           = value_type*;
        using reference         = value_type&;

        reference operator* () const { return ptr->kv; }
        pointer operator-> () const { return &(operator*()); }
        Iterator& operator++ () {
            ptr = ptr->next();
            return *this;
        }
        Iterator operator++ (int) {
            Iterator tmp = *this;
            ptr = ptr->next();
            return tmp;
        }

        Iterator& operator-- () {
            ptr = ptr->prev();
            return *this;
        }
        Iterator operator-- (int) {
            Iterator tmp = *this;
            ptr = ptr->prev();
            return tmp;
        }

        friend bool operator== (const Iterator &x, const Iterator &y) {
            return x.ptr == y.ptr;
        }

        friend bool operator!= (const Iterator &x, const Iterator &y) {
            return x.ptr != y.ptr;
        }

    private:
        Node *ptr;
    };

    Iterator begin() {
        Node *node = root;
        while (node != nullptr && node->left != nullptr) node = node->left;
        return Iterator(node);
    }

    Iterator end() {
        return Iterator(nullptr);
    }

private:
    using Dir = typename Node::Dir;
    using Color = typename Node::Color;

    void insert(Node *node, const K &k, const V &v) {
        int c = cmp(k, node->key()) ? -1 : (cmp(node->key(), k) ? 1 : 0);
        if (c == 0) {
            node->kv = make_pair(k, v);
        }
        else if (c < 1) {
            if (node->left == nullptr) {
                node->left = new Node(node, k, v);
                maintainAfterInsert(node->left);
            }
            else {
                insert(node->left, k, v);
            }
        }
        else {
            if (node->right == nullptr) {
                node->right = new Node(node, k, v);
                maintainAfterInsert(node->right);
            }
            else {
                insert(node->right, k, v);
            }
        }
    }

    void rotateLeft(Node *node) {
        // clang-format off
        //     |                       |
        //     N                       R
        //    / \     l-rotate(N)     / \
        //   L   R    ==========>    N   RR
        //      / \                 / \
        //    RL   RR              L   RL
        // clang-format on

        assert(node != nullptr && node->right != nullptr);
            
        Node* parent = node->parent;
        Dir dir = node->dir();

        Node* r = node->right;
        node->right = r->left;
        r->left = node;
        r->parent = parent;

        updateMyChildrensParent(node);
        updateMyChildrensParent(r);

        switch (dir) {
            case Node::ROOT: this->root = r; break;
            case Node::LEFT: parent->left = r; break;
            case Node::RIGHT: parent->right = r; break;
        }
    }

    void rotateRight(Node *node) {
        // clang-format off
        //       |                   |
        //       N                   L
        //      / \   r-rotate(N)   / \
        //     L   R  ==========> LL   N
        //    / \                     / \
        //  LL   LR                 LR   R
        // clang-format on
        assert(node != nullptr && node->left != nullptr);

        Node *parent = node->parent;
        Dir dir = node->dir();

        Node *l = node->left;
        node->left = l->right;
        l->right = node;
        l->parent = parent;

        updateMyChildrensParent(node);
        updateMyChildrensParent(l);

        switch (dir) {
            case Dir::ROOT: this->root = l; break;
            case Dir::LEFT: parent->left = l; break;
            case Dir::RIGHT: parent->right = l; break;
        }
    }

    void maintainAfterInsert(Node *node) {
        assert(node != nullptr);

        if (node->isRoot()) {
            // Case 1: Current node is root (RED)
            // No need to fix.
            assert(node->isRed());
            return;
        }

        if (node->parent->isBlack()) {
            // Case 2: Parent is BLACK
            // No need to fix.
            return;
        }

        if (node->parent->isRoot()) {
            // clang-format off
            // Case 3: Parent is root and is RED
            //   Paint parent to BLACK.
            //    <P>         [P]
            //     |   ====>   |
            //    <N>         <N>
            //   p.s.
            //    `<X>` is a RED node;
            //    `[X]` is a BLACK node (or NIL);
            //    `{X}` is either a RED node or a BLACK node;
            // clang-format on
            assert(node->parent->isRed());
            node->parent->color = Color::BLACK;
            return;
        }

        if (node->hasUncle() && node->uncle()->isRed()) {
            // clang-format off
            // Case 4: Both parent and uncle are RED
            //   Paint parent and uncle to BLACK;
            //   Paint grandparent to RED.
            //        [G]             <G>
            //        / \             / \
            //      <P> <U>  ====>  [P] [U]
            //      /               /
            //    <N>             <N>
            // clang-format on
            assert(node->parent->isRed());
            node->parent->color = Color::BLACK;
            node->uncle()->color = Color::BLACK;
            node->grandParent()->color = Color::RED;
            maintainAfterInsert(node->grandParent());
            return;
        }

        if (!node->hasUncle() || node->uncle()->isBlack()) {
            // Case 5 & 6: Parent is RED and Uncle is BLACK
            //   p.s. NIL nodes are also considered BLACK
            assert(!node->isRoot());

            if (node->dir() != node->dir()) {
                // clang-format off
                // Case 5: Current node is the opposite direction as parent
                //   Step 1. If node is a LEFT child, perform l-rotate to parent;
                //           If node is a RIGHT child, perform r-rotate to parent.
                //   Step 2. Goto Case 6.
                //      [G]                 [G]
                //      / \    rotate(P)    / \
                //    <P> [U]  ========>  <N> [U]
                //      \                 /
                //      <N>             <P>
                // clang-format on

                // Step 1: Rotation
                Node* parent = node->parent;
                if (node->dir() == Dir::LEFT) {
                    rotateRight(node->parent);
                } 
                else /* node->dir() == Dir::RIGHT */ {
                    rotateLeft(node->parent);
                }
                node = parent;
                // Step 2: vvv
            }

            // clang-format off
            // Case 6: Current node is the same direction as parent
            //   Step 1. If node is a LEFT child, perform r-rotate to grandparent;
            //           If node is a RIGHT child, perform l-rotate to grandparent.
            //   Step 2. Paint parent (before rotate) to BLACK;
            //           Paint grandparent (before rotate) to RED.
            //        [G]                 <P>               [P]
            //        / \    rotate(G)    / \    repaint    / \
            //      <P> [U]  ========>  <N> [G]  ======>  <N> <G>
            //      /                         \                 \
            //    <N>                         [U]               [U]
            // clang-format on

            assert(node->grandParent() != nullptr);

            // Step 1
            if (node->parent->dir() == Dir::LEFT) {
                rotateRight(node->grandParent());
            } 
            else {
                rotateLeft(node->grandParent());
            }

            // Step 2
            node->parent->color = Color::BLACK;
            node->sibling()->color = Color::RED;

            return;
        }
    }

    static void updateMyChildrensParent(Node *node) {
        if (node->left != nullptr) {
            node->left->parent = node;
        }
        if (node->right != nullptr) {
            node->right->parent = node;
        }
    }

    Node *root;
    Compare cmp;
};

int main() {
    RBTree<int, string> tree1;
    tree1.insert(1, "one");
    tree1.insert(2, "two");
    tree1.insert(3, "three");
    tree1.insert(4, "four");
    tree1.insert(5, "five");
    tree1.insert(6, "six");
    tree1.insert(7, "seven");
    tree1.insert(8, "eight");
    tree1.insert(9, "nine");
    cout <<tree1.to_graphviz() <<endl;

    RBTree<int, string> tree2;
    tree2.insert(9, "nine");
    tree2.insert(8, "eight");
    tree2.insert(7, "seven");
    tree2.insert(6, "six");
    tree2.insert(5, "five");
    tree2.insert(4, "four");
    tree2.insert(3, "three");
    tree2.insert(2, "two");
    tree2.insert(1, "one");
    cout <<tree2.to_graphviz() <<endl;

    RBTree<int, string> tree3;
    tree3.insert(3, "three");
    tree3.insert(9, "nine");
    tree3.insert(2, "two");
    tree3.insert(6, "six");
    tree3.insert(5, "five");
    tree3.insert(4, "four");
    tree3.insert(8, "eight");
    tree3.insert(1, "one");
    tree3.insert(7, "seven");
    cout <<tree3.to_graphviz() <<endl;

    cout <<"seq:" <<endl;
    auto iter = tree3.begin();
    while (iter != tree3.end()) {
        cout <<iter->first <<" " <<iter->second <<endl;
        ++iter;
    }

    vector<pair<int, string>> v;
    copy(tree3.begin(), tree3.end(), back_inserter(v));
    for (pair<int, string> p: v) cout <<p.second <<endl;
    return 0;
}