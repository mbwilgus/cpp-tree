#ifndef __TREE_H__
#define __TREE_H__

#include <functional>
#include <iterator>
#include <stack>

#ifndef _P_UNUSED_
#define _P_UNUSED_ __attribute__((unused))
#endif

template <typename T, typename Compare = std::less<T>> class bst
{
  protected:
    struct bst_node;

    class bst_node_iterator_base;
    class bst_node_iterator;
    class const_bst_node_iterator;

    bst_node* root;

  public:
    using value_type     = T;
    using value_compare  = Compare;
    using iterator       = bst_node_iterator;
    using const_iterator = const_bst_node_iterator;

    bst() = default;
    bst(const bst& source);
    bst(bst&& source);
    virtual ~bst();

    iterator insert(const_iterator pos, const T& data);
    iterator insert(const T& data);

    iterator erase(const_iterator pos);

    iterator find(const T& data);
    const_iterator find(const T& data) const;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;

    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    iterator last();
    const_iterator last() const;
    const_iterator clast() const;

  protected:
    template <typename Visitor>
    static void preorder_visit(bst_node* node, Visitor visit);
    template <typename Visitor>
    static void inorder_visit(bst_node* node, Visitor visit);
    template <typename Visitor>
    static void postorder_visit(bst_node* node, Visitor visit);

    static bst_node* subtree_find(bst_node* node, const T& data);

    static bst_node* subtree_min(bst_node* node);
    static bst_node* subtree_max(bst_node* node);

    static bst_node* subtree_succ(bst_node* node);
    static bst_node* subtree_pred(bst_node* node);

    virtual bst_node* make_node(const T& data);

    virtual void base_insert(bst_node* node);

    void transplant(bst_node* u, bst_node* v);

    inline virtual void erase_single_child_node(bst_node* node,
                                                bst_node* replacement);
    inline virtual void erase_double_child_node(bst_node* node,
                                                bst_node* replacement);

    virtual void base_erase(bst_node* node);
};

template <typename T, typename Compare> struct bst<T, Compare>::bst_node {
    T data;
    bst_node* parent = nullptr;
    bst_node* left   = nullptr;
    bst_node* right  = nullptr;

    bst_node() = default;
    bst_node(const T& data);
    bst_node(const bst_node& source);

    bool operator<(const bst_node& rhs) const;
};

template <typename T, typename Compare>
class bst<T, Compare>::bst_node_iterator_base
{
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = typename bst<T, Compare>::value_type;
    using difference_type   = std::size_t;

    bst_node_iterator_base(bst_node* node);
    bst_node_iterator_base(const bst_node_iterator_base& source);

    value_type& value();
    const value_type& value() const;
    value_type* ptr();
    const value_type* ptr() const;

    void increment();
    bool equals(const bst_node_iterator_base& rhs) const;

  private:
    bst_node* node;

    friend class bst;
};

template <typename T, typename Compare>
class bst<T, Compare>::bst_node_iterator : public bst_node_iterator_base
{
  public:
    using iterator_category =
        typename bst_node_iterator_base::iterator_category;
    using value_type      = typename bst_node_iterator_base::value_type;
    using difference_type = typename bst_node_iterator_base::difference_type;

    using reference = value_type&;
    using pointer   = value_type*;

    bst_node_iterator(bst_node* node);
    bst_node_iterator(const bst_node_iterator& source);

    reference operator*();
    pointer operator->();

    bst_node_iterator& operator++();
    bst_node_iterator operator++(int);

    bool operator==(const bst_node_iterator& rhs) const;
    bool operator!=(const bst_node_iterator& rhs) const;
};

template <typename T, typename Compare>
class bst<T, Compare>::const_bst_node_iterator : public bst_node_iterator_base
{
  public:
    using iterator_category =
        typename bst_node_iterator_base::iterator_category;
    using value_type      = const typename bst_node_iterator_base::value_type;
    using difference_type = typename bst_node_iterator_base::difference_type;

    using reference = value_type&;
    using pointer   = value_type*;

    const_bst_node_iterator(bst_node* node);
    const_bst_node_iterator(const const_bst_node_iterator& source);
    const_bst_node_iterator(const bst_node_iterator& source);

    reference operator*();
    pointer operator->();

    const_bst_node_iterator& operator++();
    const_bst_node_iterator operator++(int);

    bool operator==(const const_bst_node_iterator& rhs) const;
    bool operator!=(const const_bst_node_iterator& rhs) const;
};

template <typename T, typename Compare>
bst<T, Compare>::bst(const bst& source) : root(new bst_node(*source.root))
{
}

template <typename T, typename Compare>
bst<T, Compare>::bst(bst&& source) : root(std::move(source.root))
{
}

template <typename T, typename Compare> bst<T, Compare>::~bst()
{
    auto destroy = [](bst_node* node) { delete node; };

    postorder_visit(root, destroy);
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
    bst_node* node = make_node(data);
    base_insert(node);
    return iterator{node};
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::erase(const_iterator pos)
{
    bst_node* node = pos.node;
    iterator next(node);
    ++next;
    base_erase(node);
    return next;
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::find(const T& data)
{
    bst_node* node = subtree_find(root, data);
    return iterator{node};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator
bst<T, Compare>::find(const T& data) const
{
    bst_node* node = subtree_find(root, data);
    return const_iterator{node};
}

template <typename T, typename Compare>
typename bst<T, Compare>::iterator bst<T, Compare>::begin()
{
    return iterator{subtree_min(root)};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::begin() const
{
    return const_iterator{subtree_min(root)};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::cbegin() const
{
    return const_iterator{subtree_min(root)};
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
typename bst<T, Compare>::iterator bst<T, Compare>::last()
{
    return iterator{subtree_max(root)};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::last() const
{
    return const_iterator{subtree_max(root)};
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_iterator bst<T, Compare>::clast() const
{
    return const_iterator{subtree_max(root)};
}

template <typename T, typename Compare>
template <typename Visitor>
void bst<T, Compare>::preorder_visit(bst_node* node, Visitor visit)
{
    std::stack<bst_node*> traversal;

    if (node)
        traversal.push(node);

    while (!traversal.empty()) {
        bst_node* cursor = traversal.top();
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
void bst<T, Compare>::inorder_visit(bst_node* node, Visitor visit)
{
    bst_node* cursor = subtree_min(node);
    while (cursor) {
        visit(cursor);
        cursor = subtree_succ(cursor);
    }
}

template <typename T, typename Compare>
template <typename Visitor>
void bst<T, Compare>::postorder_visit(bst_node* node, Visitor visit)
{
    std::stack<bst_node*> iterating;
    std::stack<bst_node*> traversal;

    if (node)
        iterating.push(node);

    while (!iterating.empty()) {
        bst_node* cursor = iterating.top();
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
typename bst<T, Compare>::bst_node*
bst<T, Compare>::subtree_find(bst_node* node, const T& data)
{
    auto not_equals = [](const T& a, const T& b) -> bool {
        // a < b XOR b < a
        return !value_compare{}(a, b) != !value_compare{}(b, a);
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
typename bst<T, Compare>::bst_node* bst<T, Compare>::subtree_min(bst_node* node)
{
    while (node->left)
        node = node->left;
    return node;
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node* bst<T, Compare>::subtree_max(bst_node* node)
{
    while (node->right)
        node = node->right;
    return node;
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node*
bst<T, Compare>::subtree_succ(bst_node* node)
{
    if (node->right)
        return subtree_min(node->right);
    bst_node* parent = node->parent;
    while (parent && node == parent->right) {
        node   = parent;
        parent = parent->parent;
    }
    return parent;
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node*
bst<T, Compare>::subtree_pred(bst_node* node)
{
    if (node->left)
        return subtree_max(node->left);
    bst_node* parent = node->parent;
    while (parent && node == parent->left) {
        node   = parent;
        parent = parent->parent;
    }
    return parent;
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node* bst<T, Compare>::make_node(const T& data)
{
    return new bst_node{data};
}

template <typename T, typename Compare>
void bst<T, Compare>::base_insert(bst_node* node)
{
    bst_node* parent = nullptr;
    bst_node* cursor = root;
    while (cursor) {
        parent = cursor;
        if (*node < *parent)
            cursor = cursor->left;
        else
            cursor = cursor->right;
    }
    node->parent = parent;
    if (!parent)
        root = node;
    else if (*node < *parent)
        parent->left = node;
    else
        parent->right = node;
}

template <typename T, typename Compare>
void bst<T, Compare>::transplant(bst_node* u, bst_node* v)
{
    // NOTE: this method only takes care of wiring v into the position that u
    // currently holds (whatever happens to u is up to the caller)

    // u is the root
    if (!u->parent)
        root = v;

    // u is the left child of its parent
    else if (u == u->parent->left)
        // replace the subtree at u with the subtree at v
        u->parent->left = v;

    // u is the right child of its parent
    else
        u->parent->right = v;

    // ensure the symmetry of the child-parent relationship (i.e., the parent
    // of v must now be the original parent of u now that v is the child of
    // this parent)
    if (v)
        v->parent = u->parent;
}

template <typename T, typename Compare>
void bst<T, Compare>::erase_single_child_node(bst_node* node,
                                              bst_node* replacement)
{
    transplant(node, replacement);
}

template <typename T, typename Compare>
void bst<T, Compare>::erase_double_child_node(bst_node* node,
                                              bst_node* replacement)
{
    if (node != replacement->parent) {
        transplant(replacement, replacement->right);
        replacement->right         = node->right;
        replacement->right->parent = replacement;
    }

    transplant(node, replacement);
    replacement->left         = node->left;
    replacement->left->parent = replacement;
}

template <typename T, typename Compare>
void bst<T, Compare>::base_erase(bst_node* node)
{
    // node has no left child (and perhaps no right child as well)
    if (!node->left)
        erase_single_child_node(node, node->right);

    // node has no right child
    else if (!node->right)
        erase_single_child_node(node, node->left);

    // node has both a left and right child
    else {
        bst_node* successor = subtree_min(node->right);
        erase_double_child_node(node, successor);
    }

    delete node;
}

template <typename T, typename Compare>
bst<T, Compare>::bst_node::bst_node(const T& data) : data(data)
{
}

template <typename T, typename Compare>
bst<T, Compare>::bst_node::bst_node(const bst_node& source) : data(source.data)
{
}

template <typename T, typename Compare>
bool bst<T, Compare>::bst_node::operator<(const bst_node& rhs) const
{
    return value_compare{}(data, rhs.data);
}

template <typename T, typename Compare>
bst<T, Compare>::bst_node_iterator_base::bst_node_iterator_base(bst_node* node)
    : node(node)
{
}

template <typename T, typename Compare>
bst<T, Compare>::bst_node_iterator_base::bst_node_iterator_base(
    const bst_node_iterator_base& source)
    : node(source.node)
{
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node_iterator_base::value_type&
bst<T, Compare>::bst_node_iterator_base::value()
{
    return node->data;
}

template <typename T, typename Compare>
const typename bst<T, Compare>::bst_node_iterator_base::value_type&
bst<T, Compare>::bst_node_iterator_base::value() const
{
    return node->data;
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node_iterator_base::value_type*
bst<T, Compare>::bst_node_iterator_base::ptr()
{
    return std::addressof(node->data);
}

template <typename T, typename Compare>
const typename bst<T, Compare>::bst_node_iterator_base::value_type*
bst<T, Compare>::bst_node_iterator_base::ptr() const
{
    return std::addressof(node->data);
}

template <typename T, typename Compare>
void bst<T, Compare>::bst_node_iterator_base::increment()
{
    node = subtree_succ(node);
}

template <typename T, typename Compare>
bool bst<T, Compare>::bst_node_iterator_base::equals(
    const bst_node_iterator_base& rhs) const
{
    return node == rhs.node;
}

template <typename T, typename Compare>
bst<T, Compare>::bst_node_iterator::bst_node_iterator(bst_node* node)
    : bst_node_iterator_base(node)
{
}

template <typename T, typename Compare>
bst<T, Compare>::bst_node_iterator::bst_node_iterator(
    const bst_node_iterator& source)
    : bst_node_iterator_base(source)
{
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node_iterator::reference
bst<T, Compare>::bst_node_iterator::operator*()
{
    return bst_node_iterator_base::value();
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node_iterator::pointer
bst<T, Compare>::bst_node_iterator::operator->()
{
    return bst_node_iterator_base::ptr();
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node_iterator&
bst<T, Compare>::bst_node_iterator::operator++()
{
    bst_node_iterator_base::increment();
    return *this;
}

template <typename T, typename Compare>
typename bst<T, Compare>::bst_node_iterator
bst<T, Compare>::bst_node_iterator::operator++(int)
{
    bst_node_iterator tmp(*this);
    bst_node_iterator_base::increment();
    return tmp;
}

template <typename T, typename Compare>
bool bst<T, Compare>::bst_node_iterator::operator==(
    const bst_node_iterator& rhs) const
{
    return bst_node_iterator_base::equals(rhs);
}

template <typename T, typename Compare>
bool bst<T, Compare>::bst_node_iterator::operator!=(
    const bst_node_iterator& rhs) const
{
    return !bst_node_iterator_base::equals(rhs);
}

template <typename T, typename Compare>
bst<T, Compare>::const_bst_node_iterator::const_bst_node_iterator(
    bst_node* node)
    : bst_node_iterator_base(node)
{
}

template <typename T, typename Compare>
bst<T, Compare>::const_bst_node_iterator::const_bst_node_iterator(
    const const_bst_node_iterator& source)
    : bst_node_iterator_base(source)
{
}

template <typename T, typename Compare>
bst<T, Compare>::const_bst_node_iterator::const_bst_node_iterator(
    const bst_node_iterator& source)
    : bst_node_iterator_base(source)
{
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_bst_node_iterator::reference
bst<T, Compare>::const_bst_node_iterator::operator*()
{
    return bst_node_iterator_base::value();
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_bst_node_iterator::pointer
bst<T, Compare>::const_bst_node_iterator::operator->()
{
    return bst_node_iterator_base::ptr();
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_bst_node_iterator&
bst<T, Compare>::const_bst_node_iterator::operator++()
{
    bst_node_iterator_base::increment();
    return *this;
}

template <typename T, typename Compare>
typename bst<T, Compare>::const_bst_node_iterator
bst<T, Compare>::const_bst_node_iterator::operator++(int)
{
    const_bst_node_iterator tmp(*this);
    bst_node_iterator_base::increment();
    return tmp;
}

template <typename T, typename Compare>
bool bst<T, Compare>::const_bst_node_iterator::operator==(
    const const_bst_node_iterator& rhs) const
{
    return bst_node_iterator_base::equals(rhs);
}

template <typename T, typename Compare>
bool bst<T, Compare>::const_bst_node_iterator::operator!=(
    const const_bst_node_iterator& rhs) const
{
    return !bst_node_iterator_base::equals(rhs);
}

#endif
