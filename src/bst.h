#ifndef __BST_H__
#define __BST_H__

#include <algorithm>
#include <deque>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stack>

#ifndef _P_UNUSED_
#define _P_UNUSED_ __attribute__((unused))
#endif

template <typename T, typename Compare = std::less<T>,
          typename Allocator = std::allocator<T>>
class bst
{
  protected:
    struct bst_node;

    class const_bst_node_iterator;
    class mutable_iterator;

    bst_node* root = nullptr;

  public:
    using value_type = T;

    using allocator_type  = Allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_compare   = Compare;

    using reference       = value_type&;
    using const_reference = const value_type&;

    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer =
        typename std::allocator_traits<Allocator>::const_pointer;

    using const_iterator         = const_bst_node_iterator;
    using iterator               = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = const_reverse_iterator;

    using position_modifier = mutable_iterator;

    bst() = default;
    bst(const bst& source);
    bst(bst&& source);
    virtual ~bst();

    allocator_type get_allocator() const noexcept;

    bool empty() const noexcept;
    size_type size() const noexcept;

    size_type height() const noexcept;

    void clear() noexcept;

    iterator insert(const_iterator pos, const T& data);
    iterator insert(const T& data);

    template <typename InputIterator>
    void insert(InputIterator first, InputIterator last);

    void insert(std::initializer_list<value_type> ilist);

    iterator erase(const_iterator pos);

    iterator modify(const_iterator pos, const T& data);

    iterator find(const T& data) const;

    position_modifier position(const T& data);

    iterator begin() const;
    iterator end() const;

    reverse_iterator rbegin() const;
    reverse_iterator rend() const;

  protected:
    template <typename Visitor>
    static void preorder_visit(bst_node* node, Visitor visit);
    template <typename Visitor>
    static void inorder_visit(bst_node* node, Visitor visit);
    template <typename Visitor>
    static void postorder_visit(bst_node* node, Visitor visit);

    static size_type subtree_depth(bst_node* node);

    static bst_node* subtree_find(bst_node* node, const T& data);

    static bst_node* subtree_min(bst_node* node);
    static bst_node* subtree_max(bst_node* node);

    static bst_node* subtree_succ(bst_node* node);
    static bst_node* subtree_pred(bst_node* node);

    virtual bst_node* make_node(const T& data);
    virtual bst_node* copy_node(bst_node* node);

    virtual void base_insert(bst_node* node);

    void transplant(bst_node* u, bst_node* v);

    inline virtual void erase_single_child_node(bst_node* node,
                                                bst_node* replacement);
    inline virtual void erase_double_child_node(bst_node* node,
                                                bst_node* replacement);

    virtual void destroy_node(bst_node* node);

    virtual void base_erase(bst_node* node);

  private:
    using node_allocator = typename std::allocator_traits<
        Allocator>::template rebind_alloc<bst_node>;
    using alloc_traits = std::allocator_traits<node_allocator>;

    node_allocator alloc;

    size_type size_ = 0;
};

template <typename T, typename Compare, typename Allocator>
struct bst<T, Compare, Allocator>::bst_node {
    T data;
    bst_node* parent = nullptr;
    bst_node* left   = nullptr;
    bst_node* right  = nullptr;

    bst_node() = default;
    bst_node(const T& data);
    bst_node(const bst_node& source);

    bool operator<(const bst_node& rhs) const;
    bool operator!=(const bst_node& rhs) const;
};

template <typename T, typename Compare, typename Allocator>
class bst<T, Compare, Allocator>::const_bst_node_iterator
{
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = typename bst<T, Compare, Allocator>::value_type;
    using difference_type   = std::size_t;

    using reference = value_type&;
    using pointer   = value_type*;

    const_bst_node_iterator(bst_node* node);
    const_bst_node_iterator(const const_bst_node_iterator& source);

    reference operator*();
    pointer operator->();

    const_bst_node_iterator& operator++();
    const_bst_node_iterator operator++(int);
    const_bst_node_iterator& operator--();
    const_bst_node_iterator operator--(int);

    bool operator==(const const_bst_node_iterator& rhs) const;
    bool operator!=(const const_bst_node_iterator& rhs) const;

  private:
    bst_node* node;
    bool before_start = false;
    bool after_end    = false;
    bst_node* next    = nullptr;

    friend class bst;
};

template <typename T, typename Compare, typename Allocator>
class bst<T, Compare, Allocator>::mutable_iterator
{
  public:
    using iterator_category = std::output_iterator_tag;
    using value_type        = void;
    using difference_type   = std::ptrdiff_t;
    using pointer           = void;
    using reference         = void;

    mutable_iterator(bst* t, iterator iter);

    mutable_iterator& operator++();
    mutable_iterator& operator++(int);

    mutable_iterator& operator=(const bst::value_type& data);

  private:
    bst* t;
    iterator iter;
};

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::bst(const bst& source)
{
    std::stack<bst_node*> parent_copies;

    auto copy_and_connect = [&](bst_node* node) {
        bst_node* copy = copy_node(node);

        if (node->parent) {
            bst_node* parent = parent_copies.top();
            copy->parent = parent;
            if (node == node->parent->left)
                parent->left = copy;
            else {
                parent->right = copy;
                parent_copies.pop();
            }
        } else
            root = copy;

        if (node->left || node->right)
            parent_copies.push(copy);
    };

    preorder_visit(source.root, copy_and_connect);
}

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::bst(bst&& source) : root(std::move(source.root))
{
}

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::~bst()
{
    auto destroy = [this](bst_node* node) { destroy_node(node); };
    postorder_visit(root, destroy);
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::allocator_type
bst<T, Compare, Allocator>::get_allocator() const noexcept
{
    return allocator_type{};
}

template <typename T, typename Compare, typename Allocator>
bool bst<T, Compare, Allocator>::empty() const noexcept
{
    return !root;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::size_type
bst<T, Compare, Allocator>::size() const noexcept
{
    return size_;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::size_type
bst<T, Compare, Allocator>::height() const noexcept
{
    return subtree_depth(root);
}

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::clear() noexcept
{
    auto destroy = [this](bst_node* node) { destroy_node(node); };
    postorder_visit(root, destroy);

    root  = nullptr; // otherwise might contain grabage data
    size_ = 0;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::iterator
bst<T, Compare, Allocator>::insert(_P_UNUSED_ const_iterator pos, const T& data)
{
    return insert(data);
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::iterator
bst<T, Compare, Allocator>::insert(const T& data)
{
    bst_node* node = make_node(data);
    base_insert(node);

    ++size_;

    return iterator{node};
}

template <typename T, typename Compare, typename Allocator>
template <typename InputIterator>
void bst<T, Compare, Allocator>::insert(InputIterator first, InputIterator last)
{
    std::for_each(first, last, [&](const value_type& data) { insert(data); });
}

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::insert(std::initializer_list<value_type> ilist)
{
    insert(ilist.begin(), ilist.end());
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::iterator
bst<T, Compare, Allocator>::erase(const_iterator pos)
{
    bst_node* node = pos.node;
    bst_node* next = subtree_succ(node);
    base_erase(node);
    destroy_node(node);

    --size_;

    if (next)
        return iterator{next};
    else
        return end();
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::iterator
bst<T, Compare, Allocator>::modify(const_iterator pos, const T& data)
{
    bst_node* node = make_node(data);
    iterator iter(pos);
    if (pos.node != node) {
        base_erase(pos.node);
        destroy_node(pos.node);
        base_insert(node);
        iter = iterator{node};
    }
    return iter;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::iterator
bst<T, Compare, Allocator>::find(const T& data) const
{
    bst_node* node = subtree_find(root, data);
    return iterator{node};
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::position_modifier
bst<T, Compare, Allocator>::position(const T& data)
{
    iterator pos = find(data);
    return position_modifier{this, pos};
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::iterator
bst<T, Compare, Allocator>::begin() const
{
    return iterator{subtree_min(root)};
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::iterator
bst<T, Compare, Allocator>::end() const
{
    return ++iterator{subtree_max(root)};
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::reverse_iterator
bst<T, Compare, Allocator>::rbegin() const
{
    return std::make_reverse_iterator(end());
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::reverse_iterator
bst<T, Compare, Allocator>::rend() const
{
    return std::make_reverse_iterator(begin());
}

template <typename T, typename Compare, typename Allocator>
template <typename Visitor>
void bst<T, Compare, Allocator>::preorder_visit(bst_node* node, Visitor visit)
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

template <typename T, typename Compare, typename Allocator>
template <typename Visitor>
void bst<T, Compare, Allocator>::inorder_visit(bst_node* node, Visitor visit)
{
    bst_node* cursor = subtree_min(node);
    while (cursor) {
        visit(cursor);
        cursor = subtree_succ(cursor);
    }
}

template <typename T, typename Compare, typename Allocator>
template <typename Visitor>
void bst<T, Compare, Allocator>::postorder_visit(bst_node* node, Visitor visit)
{
    std::stack<bst_node*> setup;
    std::stack<bst_node*> traversal;

    if (node)
        setup.push(node);

    while (!setup.empty()) {
        bst_node* cursor = setup.top();
        setup.pop();

        traversal.push(cursor);

        if (cursor->right)
            setup.push(cursor->right);

        if (cursor->left)
            setup.push(cursor->left);
    }

    while (!traversal.empty()) {
        visit(traversal.top());
        traversal.pop();
    }
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::size_type
bst<T, Compare, Allocator>::subtree_depth(bst_node* node)
{
    std::deque<bst_node*> queue;
    queue.push_back(node);

    size_type h = 0;

    while (!queue.empty()) {

        size_type level_population = queue.size();

        while (level_population--) {
            bst_node* processing = queue.front();
            queue.pop_front();

            if (processing->left)
                queue.push_back(processing->left);

            if (processing->right)
                queue.push_back(processing->right);
        }

        ++h;
    }

    return --h;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::bst_node*
bst<T, Compare, Allocator>::subtree_find(bst_node* node, const T& data)
{
    bst_node rhs(data);

    while (node && *node != rhs) {
        if (rhs < *node)
            node = node->left;
        else
            node = node->right;
    }
    return node;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::bst_node*
bst<T, Compare, Allocator>::subtree_min(bst_node* node)
{
    while (node->left)
        node = node->left;
    return node;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::bst_node*
bst<T, Compare, Allocator>::subtree_max(bst_node* node)
{
    while (node->right)
        node = node->right;
    return node;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::bst_node*
bst<T, Compare, Allocator>::subtree_succ(bst_node* node)
{
    if (!node)
        return nullptr;
    if (node->right)
        return subtree_min(node->right);
    bst_node* parent = node->parent;
    while (parent && node == parent->right) {
        node   = parent;
        parent = parent->parent;
    }
    return parent;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::bst_node*
bst<T, Compare, Allocator>::subtree_pred(bst_node* node)
{
    if (!node)
        return nullptr;
    if (node->left)
        return subtree_max(node->left);
    bst_node* parent = node->parent;
    while (parent && node == parent->left) {
        node   = parent;
        parent = parent->parent;
    }
    return parent;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::bst_node*
bst<T, Compare, Allocator>::make_node(const T& data)
{
    bst_node* node = alloc_traits::allocate(alloc, 1);
    alloc_traits::construct(alloc, node, data);
    return node;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::bst_node*
bst<T, Compare, Allocator>::copy_node(bst_node* node)
{
    bst_node* copy = alloc_traits::allocate(alloc, 1);
    alloc_traits::construct(alloc, copy, *node);
    return copy;
}

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::base_insert(bst_node* node)
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

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::transplant(bst_node* u, bst_node* v)
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

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::erase_single_child_node(bst_node* node,
                                                         bst_node* replacement)
{
    transplant(node, replacement);
}

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::erase_double_child_node(bst_node* node,
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

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::destroy_node(bst_node* node)
{
    alloc_traits::destroy(alloc, node);
    alloc_traits::deallocate(alloc, node, 1);
}

template <typename T, typename Compare, typename Allocator>
void bst<T, Compare, Allocator>::base_erase(bst_node* node)
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
}

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::bst_node::bst_node(const T& data) : data(data)
{
}

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::bst_node::bst_node(const bst_node& source)
    : data(source.data)
{
}

template <typename T, typename Compare, typename Allocator>
bool bst<T, Compare, Allocator>::bst_node::operator<(const bst_node& rhs) const
{
    return value_compare{}(data, rhs.data);
}

template <typename T, typename Compare, typename Allocator>
bool bst<T, Compare, Allocator>::bst_node::operator!=(const bst_node& rhs) const
{
    return value_compare{}(data, rhs.data) != value_compare{}(rhs.data, data);
}

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::const_bst_node_iterator::const_bst_node_iterator(
    bst_node* node)
    : node(node)
{
}

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::const_bst_node_iterator::const_bst_node_iterator(
    const const_bst_node_iterator& source)
    : node(source.node), before_start(source.before_start),
      after_end(source.after_end), next(source.next)
{
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::const_bst_node_iterator::reference
bst<T, Compare, Allocator>::const_bst_node_iterator::operator*()
{
    return node->data;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::const_bst_node_iterator::pointer
bst<T, Compare, Allocator>::const_bst_node_iterator::operator->()
{
    return std::addressof(node->data);
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::const_bst_node_iterator&
bst<T, Compare, Allocator>::const_bst_node_iterator::operator++()
{
    if (before_start) {
        node         = next;
        before_start = false;
    } else {
        bst_node* successor = subtree_succ(node);
        if (!(successor || after_end)) {
            after_end = true;
            next      = node;
        }
        node = successor;
    }
    return *this;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::const_bst_node_iterator
bst<T, Compare, Allocator>::const_bst_node_iterator::operator++(int)
{
    const_bst_node_iterator tmp(*this);
    ++*this;
    return tmp;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::const_bst_node_iterator&
bst<T, Compare, Allocator>::const_bst_node_iterator::operator--()
{
    if (after_end) {
        node      = next;
        after_end = false;
    } else {
        bst_node* predecessor = subtree_pred(node);
        if (!(predecessor || before_start)) {
            before_start = true;
            next         = node;
        }
        node = subtree_pred(node);
    }
    return *this;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::const_bst_node_iterator
bst<T, Compare, Allocator>::const_bst_node_iterator::operator--(int)
{
    const_bst_node_iterator tmp(*this);
    --*this;
    return tmp;
}

template <typename T, typename Compare, typename Allocator>
bool bst<T, Compare, Allocator>::const_bst_node_iterator::operator==(
    const const_bst_node_iterator& rhs) const
{
    return node == rhs.node;
}

template <typename T, typename Compare, typename Allocator>
bool bst<T, Compare, Allocator>::const_bst_node_iterator::operator!=(
    const const_bst_node_iterator& rhs) const
{
    return !operator==(rhs);
}

template <typename T, typename Compare, typename Allocator>
bst<T, Compare, Allocator>::mutable_iterator::mutable_iterator(bst* t,
                                                               iterator iter)
    : t(t), iter(iter)
{
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::mutable_iterator&
bst<T, Compare, Allocator>::mutable_iterator::operator++()
{
    return this;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::mutable_iterator&
bst<T, Compare, Allocator>::mutable_iterator::operator++(int)
{
    return this;
}

template <typename T, typename Compare, typename Allocator>
typename bst<T, Compare, Allocator>::mutable_iterator&
bst<T, Compare, Allocator>::mutable_iterator::operator=(
    const bst::value_type& data)
{
    if (iter == t->end())
        iter = t->insert(iter, data);
    else
        iter = t->modify(iter, data);
    return *this;
}

#endif
