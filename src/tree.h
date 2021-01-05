#ifndef __TREE_H__
#define __TREE_H__

#include <functional>
#include <iterator>
#include <memory>
#include <stack>
#include <utility>

#ifndef _P_UNUSED_
#define _P_UNUSED_ __attribute__((unused))
#endif

template <typename T, typename Compare = std::less<T>> class bst
{
  protected:
    struct node_base;

    class node_base_iterator_base;
    class node_base_iterator;
    class const_node_base_iterator;

    node_base* root();

    template <typename Visitor>
    static void preorder_visit(node_base* node, Visitor visit);
    template <typename Visitor>
    static void inorder_visit(node_base* node, Visitor visit);
    template <typename Visitor>
    static void postorder_visit(node_base* node, Visitor visit);

    static node_base* subtree_find(node_base* node, const T& data);

    static node_base* subtree_min(node_base* node);
    static node_base* subtree_max(node_base* node);

    static node_base* subtree_succ(node_base* node);
    static node_base* subtree_pred(node_base* node);

    void left_rotate(node_base* node);
    void right_rotate(node_base* node);

    void base_insert(node_base* node);

    void transplant(node_base* u, node_base* v);
    void base_erase(node_base* node);

  public:
    using value_type     = T;
    using value_compare  = Compare;
    using iterator       = node_base_iterator;
    using const_iterator = const_node_base_iterator;

    bst() = default;
    bst(const bst& source);
    bst(bst&& source);
    ~bst();

    virtual iterator insert(const_iterator pos, const T& data);
    virtual iterator insert(const T& data);

    virtual iterator erase(const_iterator pos);

    iterator find(const T& data);
    const_iterator find(const T& data) const;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

  private:
    node_base* root_ = nullptr;
};

template <typename T, typename Compare> struct bst<T, Compare>::node_base {
    T data;
    node_base* parent = nullptr;
    node_base* left   = nullptr;
    node_base* right  = nullptr;

    node_base() = default;
    node_base(const T& data);
    node_base(const node_base& source);

    bool operator<(const node_base& rhs) const;
};

template <typename T, typename Compare>
class bst<T, Compare>::node_base_iterator_base
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = typename bst<T, Compare>::value_type;
    using difference_type   = std::size_t;

    node_base_iterator_base(node_base* node);
    node_base_iterator_base(const node_base_iterator_base& source);

    value_type& value();
    const value_type& value() const;
    value_type* ptr();
    const value_type* ptr() const;

    void increment();
    bool equals(const node_base_iterator_base& rhs) const;

  private:
    node_base* node;

    friend class bst;
};

template <typename T, typename Compare>
class bst<T, Compare>::node_base_iterator : public node_base_iterator_base
{
  public:
    using iterator_category =
        typename node_base_iterator_base::iterator_category;
    using value_type      = typename node_base_iterator_base::value_type;
    using difference_type = typename node_base_iterator_base::difference_type;

    using reference = value_type&;
    using pointer   = value_type*;

    node_base_iterator(node_base* node);
    node_base_iterator(const node_base_iterator& source);

    reference operator*();
    pointer operator->();

    node_base_iterator& operator++();
    node_base_iterator operator++(int);

    bool operator==(const node_base_iterator& rhs) const;
    bool operator!=(const node_base_iterator& rhs) const;
};

template <typename T, typename Compare>
class bst<T, Compare>::const_node_base_iterator : public node_base_iterator_base
{
  public:
    using iterator_category =
        typename node_base_iterator_base::iterator_category;
    using value_type      = const typename node_base_iterator_base::value_type;
    using difference_type = typename node_base_iterator_base::difference_type;

    using reference = value_type&;
    using pointer   = value_type*;

    const_node_base_iterator(node_base* node);
    const_node_base_iterator(const const_node_base_iterator& source);
    const_node_base_iterator(const node_base_iterator& source);

    reference operator*();
    pointer operator->();

    const_node_base_iterator& operator++();
    const_node_base_iterator operator++(int);

    bool operator==(const const_node_base_iterator& rhs) const;
    bool operator!=(const const_node_base_iterator& rhs) const;
};

template <typename T, typename Compare>
typename bst<T, Compare>::node_base* bst<T, Compare>::root()
{
    return root_;
}

template <typename T, typename Compare>
template <typename Visitor>
void bst<T, Compare>::preorder_visit(node_base* node, Visitor visit)
{
    std::stack<node_base*> traversal;

    if (node)
        traversal.push(node);

    while (!traversal.empty()) {
        node_base* cursor = traversal.top();
        traversal.pop();

        visit(cursor);

        if (cursor->right)
            traversal.push(node->right);
        if (cursor->left)
            traversal.push(node->left);
    }
}

template <typename T, typename Compare>
template <typename Visitor>
void bst<T, Compare>::inorder_visit(node_base* node, Visitor visit)
{
    node_base* cursor = subtree_min(node);
    while (cursor) {
        visit(cursor);
        cursor = subtree_succ(cursor);
    }
}

template <typename T, typename Compare>
template <typename Visitor>
void bst<T, Compare>::postorder_visit(node_base* node, Visitor visit)
{
    std::stack<node_base*> iterating;
    std::stack<node_base*> traversal;

    if (node)
        iterating.push(node);

    while (!iterating.empty()) {
        node_base* cursor = iterating.top();
        iterating.pop();

        traversal.push(cursor);

        if (cursor->right)
            iterating.push(cursor->right);

        if (cursor->left)
            iterating.push(cursor->left);
    }

    while (!traversal.empty()) {
        visit(traversal.top());
        traversal.pop();
    }
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base*
bst<T, Compare>::subtree_find(node_base* node, const T& data)
{
    auto not_equals = [](const T& a, const T& b) -> bool {
        return value_compare{}(a, b) || value_compare{}(b, a);
    };

    while (node && not_equals(data, node->data)) {
        if (value_compare{}(data, node->data))
            node = node->left;
        else
            node = node->right;
    }
    return node;
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base*
bst<T, Compare>::subtree_min(node_base* node)
{
    while (node->left)
        node = node->left;
    return node;
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base*
bst<T, Compare>::subtree_max(node_base* node)
{
    while (node->right)
        node = node->right;
    return node;
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base*
bst<T, Compare>::subtree_succ(node_base* node)
{
    if (node->right)
        return subtree_min(node->right);
    node_base* parent = node->parent;
    while (parent && node == parent->right) {
        node   = parent;
        parent = parent->parent;
    }
    return parent;
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base*
bst<T, Compare>::subtree_pred(node_base* node)
{
    if (node->left)
        return subtree_max(node->left);
    node_base* parent = node->parent;
    while (parent && node == parent->left) {
        node   = parent;
        parent = parent->parent;
    }
    return parent;
}

template <typename T, typename Compare>
void bst<T, Compare>::left_rotate(node_base* node)
{
    node_base* child = node->right;
    node->right      = child->left;
    if (child->left)
        child->left->parent = node;
    child->parent = node->parent;
    if (!node->parent)
        root_ = child;
    else if (node == node->parent->left)
        node->parent->left = child;
    else
        node->parent->right = child;
    child->left  = node;
    node->parent = child;
}

template <typename T, typename Compare>
void bst<T, Compare>::right_rotate(node_base* node)
{
    node_base* child = node->left;
    node->left       = child->right;
    if (child->right)
        child->right->parent = node;
    child->parent = node->parent;
    if (!node->parent)
        root_ = child;
    else if (node == node->parent->right)
        node->parent->right = child;
    else
        node->parent->left = child;
    child->right = node;
    node->parent = child;
}

template <typename T, typename Compare>
void bst<T, Compare>::base_insert(node_base* node)
{
    node_base* parent = nullptr;
    node_base* cursor = root_;
    while (cursor) {
        parent = cursor;
        if (*node < *parent)
            cursor = cursor->left;
        else
            cursor = cursor->right;
    }
    node->parent = parent;
    if (!parent)
        root_ = node;
    else if (*node < *parent)
        parent->left = node;
    else
        parent->right = node;
}

template <typename T, typename Compare>
void bst<T, Compare>::transplant(node_base* u, node_base* v)
{
    if (!u->parent)
        root_ = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    if (v)
        v->parent = u->parent;
}

template <typename T, typename Compare>
void bst<T, Compare>::base_erase(node_base* node)
{
    if (!node->left)
        transplant(node, node->right);
    else if (!node->right)
        transplant(node, node->left);
    else {
        node_base* min = subtree_min(node->right);
        if (min->parent != node) {
            transplant(min, min->right);
            min->right = node->right;
            min->right->parent = min;
        }
        transplant(node, min);
        min->left = node->left;
        min->left->parent = min;
    }

    delete node;
}

template <typename T, typename Compare>
bst<T, Compare>::bst(const bst& source) : root_(new node_base(*source.root))
{
}

template <typename T, typename Compare>
bst<T, Compare>::bst(bst&& source) : root_(std::move(source.root))
{
}

template <typename T, typename Compare> bst<T, Compare>::~bst()
{
    auto destroy = [](node_base* node) { delete node; };

    postorder_visit(root_, destroy);
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator
bst<T, Compare>::insert(_P_UNUSED_ const_iterator pos, const T& data)
{
    return insert(data);
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::insert(const T& data)
{
    node_base* node = new node_base{data};
    base_insert(node);
    return iterator{node};
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::erase(const_iterator pos)
{
    node_base* node = pos.node;
    iterator next(node);
    ++next;
    base_erase(node);
    return next;
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::find(const T& data)
{
    node_base* node = subtree_find(root_, data);
    return iterator{node};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator
bst<T, Compare>::find(const T& data) const
{
    node_base* node = subtree_find(root_, data);
    return const_iterator{node};
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::begin()
{
    return iterator{subtree_min(root_)};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::begin() const
{
    return const_iterator{subtree_min(root_)};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::cbegin() const
{
    return const_iterator{subtree_min(root_)};
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::end()
{
    return iterator{nullptr};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::end() const
{
    return const_iterator{nullptr};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::cend() const
{
    return const_iterator{nullptr};
}

template <typename T, typename Compare>
bst<T, Compare>::node_base::node_base(const T& data) : data(data)
{
}

template <typename T, typename Compare>
bst<T, Compare>::node_base::node_base(const node_base& source)
    : data(source.data)
{
}

template <typename T, typename Compare>
bool bst<T, Compare>::node_base::operator<(const node_base& rhs) const
{
    return value_compare{}(data, rhs.data);
}

template <typename T, typename Compare>
bst<T, Compare>::node_base_iterator_base::node_base_iterator_base(
    node_base* node)
    : node(node)
{
}

template <typename T, typename Compare>
bst<T, Compare>::node_base_iterator_base::node_base_iterator_base(
    const node_base_iterator_base& source)
    : node(source.node)
{
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base_iterator_base::value_type&
bst<T, Compare>::node_base_iterator_base::value()
{
    return node->data;
}

template <typename T, typename Compare>
const typename bst<T, Compare>::node_base_iterator_base::value_type&
bst<T, Compare>::node_base_iterator_base::value() const
{
    return node->data;
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base_iterator_base::value_type*
bst<T, Compare>::node_base_iterator_base::ptr()
{
    return std::addressof(node->data);
}

template <typename T, typename Compare>
const typename bst<T, Compare>::node_base_iterator_base::value_type*
bst<T, Compare>::node_base_iterator_base::ptr() const
{
    return std::addressof(node->data);
}

template <typename T, typename Compare>
void bst<T, Compare>::node_base_iterator_base::increment()
{
    node = subtree_succ(node);
}

template <typename T, typename Compare>
bool bst<T, Compare>::node_base_iterator_base::equals(
    const node_base_iterator_base& rhs) const
{
    return node == rhs.node;
}

template <typename T, typename Compare>
bst<T, Compare>::node_base_iterator::node_base_iterator(node_base* node)
    : node_base_iterator_base(node)
{
}

template <typename T, typename Compare>
bst<T, Compare>::node_base_iterator::node_base_iterator(
    const node_base_iterator& source)
    : node_base_iterator_base(source)
{
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base_iterator::reference
bst<T, Compare>::node_base_iterator::operator*()
{
    return node_base_iterator_base::value();
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base_iterator::pointer
bst<T, Compare>::node_base_iterator::operator->()
{
    return node_base_iterator_base::ptr();
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base_iterator&
bst<T, Compare>::node_base_iterator::operator++()
{
    node_base_iterator_base::increment();
    return *this;
}

template <typename T, typename Compare>
typename bst<T, Compare>::node_base_iterator
bst<T, Compare>::node_base_iterator::operator++(int)
{
    node_base_iterator tmp(*this);
    node_base_iterator_base::increment();
    return tmp;
}

template <typename T, typename Compare>
bool bst<T, Compare>::node_base_iterator::operator==(
    const node_base_iterator& rhs) const
{
    return node_base_iterator_base::equals(rhs);
}

template <typename T, typename Compare>
bool bst<T, Compare>::node_base_iterator::operator!=(
    const node_base_iterator& rhs) const
{
    return !node_base_iterator_base::equals(rhs);
}

template <typename T, typename Compare>
bst<T, Compare>::const_node_base_iterator::const_node_base_iterator(
    node_base* node)
    : node_base_iterator_base(node)
{
}

template <typename T, typename Compare>
bst<T, Compare>::const_node_base_iterator::const_node_base_iterator(
    const const_node_base_iterator& source)
    : node_base_iterator_base(source)
{
}

template <typename T, typename Compare>
bst<T, Compare>::const_node_base_iterator::const_node_base_iterator(
    const node_base_iterator& source)
    : node_base_iterator_base(source)
{
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_node_base_iterator::reference
bst<T, Compare>::const_node_base_iterator::operator*()
{
    return node_base_iterator_base::value();
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_node_base_iterator::pointer
bst<T, Compare>::const_node_base_iterator::operator->()
{
    return node_base_iterator_base::ptr();
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_node_base_iterator&
bst<T, Compare>::const_node_base_iterator::operator++()
{
    node_base_iterator_base::increment();
    return *this;
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_node_base_iterator
bst<T, Compare>::const_node_base_iterator::operator++(int)
{
    const_node_base_iterator tmp(*this);
    node_base_iterator_base::increment();
    return tmp;
}

template <typename T, typename Compare>
bool bst<T, Compare>::const_node_base_iterator::operator==(
    const const_node_base_iterator& rhs) const
{
    return node_base_iterator_base::equals(rhs);
}

template <typename T, typename Compare>
bool bst<T, Compare>::const_node_base_iterator::operator!=(
    const const_node_base_iterator& rhs) const
{
    return !node_base_iterator_base::equals(rhs);
}

#endif
