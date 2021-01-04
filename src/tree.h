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
    static void post_order_visit(node_base* node, Visitor visit);

    static node_base* subtree_min(node_base* node);
    static node_base* subtree_max(node_base* node);

    static node_base* subtree_succ(node_base* node);
    static node_base* subtree_pred(node_base* node);

    void left_rotate(node_base* node);
    void right_rotate(node_base* node);

    void base_insert(node_base* node);

  public:
    using value_type     = T;
    using iterator       = node_base_iterator;
    using const_iterator = const_node_base_iterator;

    bst() = default;
    bst(const bst& source);
    bst(bst&& source);
    ~bst();

    virtual iterator insert(const_iterator pos, const T& data);
    virtual iterator insert(const T& data);

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
void bst<T, Compare>::post_order_visit(node_base* node, Visitor visit)
{
    std::stack<node_base*> iterating;
    std::stack<node_base*> traversal;

    if (node)
        iterating.push(node);

    while (!iterating.empty()) {
        node_base* cursor = iterating.top();
        iterating.pop();

        traversal.push(cursor);

        if (cursor->left)
            iterating.push(cursor->left);

        if (cursor->right)
            iterating.push(cursor->right);
    }

    while (!traversal.empty()) {
        visit(traversal.top());
        traversal.pop();
    }
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
        node = parent;
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
bst<T, Compare>::bst(const bst& source) : root_(new node_base(*source.root))
{
}

template <typename T, typename Compare>
bst<T, Compare>::bst(bst&& source) : root_(std::move(source.root))
{
}

template <typename T, typename Compare> bst<T, Compare>::~bst()
{
    auto destroy = [](node_base* node) {
        delete node;
    };

    post_order_visit(root_, destroy);
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
    return Compare{}(data, rhs.data);
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
