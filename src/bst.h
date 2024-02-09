#ifndef BST_H
#define BST_H

#include <algorithm>
#include <cstddef>
#include <deque>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stack>
#include <utility>

template <typename T,
          typename Compare   = std::less<T>,
          typename Allocator = std::allocator<T>>
class bst
{
  protected:
    struct Sentinel;
    struct BstNode;

    using NodeAllocator = typename std::allocator_traits<
        Allocator>::template rebind_alloc<BstNode>;
    using NodeTraits = std::allocator_traits<NodeAllocator>;

    // base class for Sentinel and BstNode
    struct Node { // NOLINT
        Node() = default;

        explicit Node(Sentinel* owner) noexcept
            : owner{owner}
        {
        }

        [[nodiscard]] inline bool is_data() const noexcept
        {
            return this != owner;
        }

        const Node* successor() const noexcept
        {
            if (is_data()) [[likely]] {
                auto* node = static_cast<const BstNode*>(this);

                // if there is a right subtree, then we return the minumum of
                // that subtree; as that is the next in-order node
                if (node->right != nullptr) return node->right->min();

                // otherwise we must traverse up the tree until we reach the
                // root of a left subtree; we have exhasted the traversal of
                // that subtree, so the next in-order node is it's root
                BstNode* parent = node->parent;

                while (parent != nullptr && node == parent->right) {
                    node   = parent;
                    parent = parent->parent;
                }

                // if we have exhasted all the nodes then we return the Sentinel
                if (parent == nullptr) return owner;

                return parent;
            }

            // otherwise, if this node is the Sentinel then the next node should
            // be the first in-order node (i.e., the minimum)
            return static_cast<const Sentinel*>(this)->min;
        }

        const Node* predecessor() const noexcept
        {
            // SEE: the notes in successor; the explanation for this method is
            // symmetric to the one provided for that method
            if (is_data()) [[likely]] {
                auto* node = static_cast<const BstNode*>(this);

                if (node->left) return node->left->max();

                BstNode* parent = node->parent;

                while (parent != nullptr && node == parent->left) {
                    node   = parent;
                    parent = parent->parent;
                }

                if (parent == nullptr) return owner;

                return parent;
            }

            return static_cast<const Sentinel*>(this)->max;
        }

        Sentinel* owner{nullptr};
    };

    /* this class contains the root of the tree, but also acts as the
     * past-the-end node
     *
     * actual tree nodes shall keep a reference to the Sentinel that contains
     * its root so that it can be iterated to
     *
     * also it holds reference to the min and max nodes of the tree so that they
     * may subsequently be iterated to */
    struct Sentinel : public Node { // NOLINT
        Sentinel() noexcept
            : Node{this}
        {
        }

        virtual ~Sentinel() = default;

        virtual BstNode* make_node(const T& data)       = 0;
        virtual BstNode* make_node(const BstNode& node) = 0;
        virtual void     destroy_node(BstNode* node)    = 0;

        void copy(Sentinel* that)
        {
            if (that->root == nullptr) return;

            std::stack<BstNode*> parents;

            auto copy_node = [this](BstNode*              node,
                                    Sentinel*             that,
                                    std::stack<BstNode*>& parents) {
                auto* copy = this->make_node(*node);

                if (node != that->root) [[likely]] {
                    BstNode* parent = parents.top();

                    copy->parent = parent;

                    if (node == node->parent->left) {
                        parent->left = copy;
                        if (node->parent->right == nullptr) {
                            parents.pop(); // pop if done, otherwise a right
                                           // child exists
                        }
                    } else {
                        parent->right = copy;
                        parents.pop();
                    }
                } else {
                    this->root = copy;
                }

                if (that->is_min(node)) [[unlikely]] {
                    this->update_min(copy);
                }

                if (that->is_max(node)) [[unlikely]] {
                    this->update_max(copy);
                }

                // only add internal nodes to the parent stack
                if (node->left != nullptr || node->right != nullptr) {
                    parents.push(copy);
                }
            };

            preorder_visit(that->root, copy_node, that, parents);
            this->size = that->size;
        }

        void clear()
        {
            postorder_visit(this->root, [this](BstNode* node) {
                this->destroy_node(node);
            });
            reset();
            size = 0;
        }

        inline void update_all(BstNode* node) noexcept // NOLINT
        {
            this->root = node;
            this->min  = node;
            this->max  = node;
        }

        inline bool is_min(Node* node) { return node == this->min; }
        inline void update_min(Node* min) { this->min = min; }
        inline bool is_max(Node* node) { return node == this->max; }
        inline void update_max(Node* max) { this->max = max; }

        const BstNode* find(const T& data) const
            noexcept(noexcept(Compare{}(data, data)))
        {
            return this->root->find(data);
        }

        void reset() noexcept
        {
            root = nullptr;
            min  = this;
            max  = this;
        }

        BstNode* root{nullptr};

        Node* min{this};
        Node* max{this};

        size_t size{0};
    };

    // struct FinalSentinel : public BstSentinel<NodeAllocator>;

    template <typename NodeAllocator>
    struct BstSentinel : public NodeAllocator, public Sentinel { // NOLINT
        using NodeTraits = std::allocator_traits<NodeAllocator>;
        using NodeType   = typename NodeTraits::value_type;

        ~BstSentinel()
        {
            postorder_visit(this->root, [this](BstNode* node) {
                this->destroy_node(node);
            });
        }

        BstNode* make_node(const T& data) override
        {
            auto* node = NodeTraits::allocate(*this, 1);
            NodeTraits::construct(*this,
                                  static_cast<NodeType*>(node),
                                  data,
                                  this);
            return node;
        }

        BstNode* make_node(const BstNode& node) override
        {
            auto* copy = NodeTraits::allocate(*this, 1);
            NodeTraits::construct(*this,
                                  copy,
                                  static_cast<const NodeType&>(node),
                                  this);
            return copy;
        }

        void destroy_node(BstNode* node) override
        {
            NodeTraits::destroy(*this, node);
            NodeTraits::deallocate(*this, static_cast<NodeType*>(node), 1);
        }
    };

    struct BstNode : public Node { // NOLINT
        BstNode() = default;

        BstNode(const T& data, Sentinel* owner) noexcept(noexcept(T{data}))
            : Node{owner}
            , data{data}
        {
        }

        BstNode(const BstNode& that,
                Sentinel*      owner) noexcept(noexcept(T{that.data}))
            : Node{owner}
            , data{that.data}
        {
        }

        BstNode* min() noexcept
        {
            BstNode* node{this};
            while (node->left != nullptr) node = node->left;
            return node;
        }

        BstNode* max() noexcept
        {
            BstNode* node{this};
            while (node->right != nullptr) node = node->right;
            return node;
        }

        [[nodiscard]] size_t height() const noexcept
        {
            std::deque<const BstNode*> queue;
            queue.push_back(this);

            size_t h = 0;

            while (!queue.empty()) {

                size_t nodes = queue.size();

                while (nodes-- > 0) {
                    const BstNode* node = queue.front();
                    queue.pop_front();

                    if (node->left != nullptr) queue.push_back(node->left);
                    if (node->right != nullptr) queue.push_back(node->right);
                }

                ++h;
            }

            return --h;
        }

        const BstNode* find(const T& data) const
            noexcept(noexcept(Compare{}(data, data)))
        {
            const BstNode* node{this};

            auto not_equal = [](const T& lhs, const T& rhs) noexcept(
                                 noexcept(Compare{}(lhs, rhs))) {
                return Compare{}(lhs, rhs) != Compare{}(rhs, lhs);
            };

            while (node != nullptr && not_equal(data, node->data)) {
                if (Compare{}(data, node->data)) {
                    node = node->left;
                } else {
                    node = node->right;
                }
            }
            return node;
        }

        inline void reset() noexcept
        {
            parent = nullptr;
            left   = nullptr;
            right  = nullptr;
        }

        friend bool operator<(const BstNode& lhs, const BstNode& rhs) noexcept(
            noexcept(Compare{}(lhs.data, rhs.data)))
        {
            return Compare{}(lhs.data, rhs.data);
        }

        friend bool operator!=(const BstNode& lhs, const BstNode& rhs) noexcept(
            noexcept(Compare{}(lhs.data, rhs.data)))
        {
            return lhs.data < rhs.data != rhs.data < lhs.data;
        }

        T data;

        BstNode* parent{nullptr};
        BstNode* left{nullptr};
        BstNode* right{nullptr};

        const T* get() const noexcept { return std::addressof(data); }
    };

    class ConstBstNodeIterator;
    class MutableIterator;

  public:
    using value_type = T;

    using allocator_type  = Allocator;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using value_compare   = Compare;

    using reference       = const value_type&;
    using const_reference = reference;

    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer =
        typename std::allocator_traits<Allocator>::const_pointer;

    using const_iterator         = ConstBstNodeIterator;
    using iterator               = const_iterator;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using reverse_iterator       = const_reverse_iterator;

    using position = MutableIterator;

  protected:
    /* all statndard input iteartors returned by the methods of bst are "const"
     *
     * this is because allowing the arbitrary modification of data could lead to
     * the bst contract being violated */
    class ConstBstNodeIterator
    {
        friend class bst;

      private:
        using Self = ConstBstNodeIterator;

      public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type        = T;
        using difference_type   = std::size_t;

        using reference = const T&;
        using pointer   = const T*;

        explicit ConstBstNodeIterator(const Node* node) noexcept
            : node_{node}
        {
        }

        ConstBstNodeIterator(const ConstBstNodeIterator& that) noexcept
            : node_{that.node_}
        {
        }

        ConstBstNodeIterator& operator=(const Self& that) noexcept
        {
            if (this != &that) {
                node_ = that.node_;
            }
            return *this;
        }

        const T& operator*() const noexcept
        {
            return *static_cast<const BstNode*>(node_)->get();
        }

        const T* operator->() const noexcept
        {
            return static_cast<const BstNode*>(node_)->get();
        }

        Self& operator++() noexcept
        {
            node_ = node_->successor();
            return *this;
        }

        Self operator++(int) noexcept
        {
            Self tmp{*this};
            ++*this;
            return tmp;
        }

        Self& operator--() noexcept
        {
            node_ = node_->predecessor();
            return *this;
        }

        Self operator--(int) noexcept
        {
            Self tmp{*this};
            --*this;
            return tmp;
        }

        friend bool operator==(const Self& lhs, const Self& rhs) noexcept
        {
            return lhs.node_ == rhs.node_;
        }

        friend bool operator!=(const Self& lhs, const Self& rhs) noexcept
        {
            return !(lhs.node_ == rhs.node_);
        }

      private:
        const Node* node_;

        inline Node* extract() noexcept
        {
            // const_cast is OK; Node(s) are never actually declared const
            return const_cast<Node*>(node_);
        }
    };

    class MutableIterator
    {
        friend class bst;

      public:
        using iterator_category = std::output_iterator_tag;
        using value_type        = void;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = void;

        MutableIterator& operator++() noexcept { return this; }

        MutableIterator& operator++(int) noexcept { return this; }

        MutableIterator& operator=(const T& data)
        {
            if (iter_ == t_->end()) {
                iter_ = t_->insert(iter_, data);
            } else {
                iter_ = t_->modify(iter_, data);
            }
            return *this;
        }

        MutableIterator& operator=(std::nullptr_t) noexcept
        {
            const_iterator old{iter_};
            ++iter_;
            t_->erase(old);
            return *this;
        }

      private:
        bst*     t_;
        iterator iter_;

        // only bst methods can construct MutableIterator
        MutableIterator(bst* t, iterator iter)
            : t_{t}
            , iter_{iter}
        {
        }
    };

    template <typename NodeAllocator>
    void set_alloc(BstSentinel<NodeAllocator>* alloc) noexcept
    {
        sentinel_ = alloc;
    }

  public:
    bst() { set_alloc(new BstSentinel<NodeAllocator>{}); }

    bst(const bst& that)
        : bst{}
    {
        sentinel_->copy(that.sentinel_);
    }

    bst(bst&& that) noexcept
        : sentinel_{std::exchange(that.sentinel_, nullptr)}
    {
    }

    virtual ~bst() { delete sentinel_; }

    bst& operator=(const bst& that)
    {
        if (this != &that) {
            sentinel_->clear();
            sentinel_->copy(that.sentinel_);
        }

        return *this;
    }

    bst& operator=(bst&& that) noexcept
    {
        if (this != &that) {
            sentinel_->clear();
            sentinel_ = std::exchange(that.sentinel_, nullptr);
        }

        return *this;
    }

    allocator_type get_allocator() const noexcept { return allocator_type{}; }
    [[nodiscard]] bool   empty() const noexcept { return begin() == end(); }
    [[nodiscard]] size_t size() const noexcept { return sentinel_->size; }

    [[nodiscard]] size_t height() const noexcept
    {
        return sentinel_->root->height();
    }

    void clear() { sentinel_->clear(); }

    iterator insert(const_iterator, const T& data) { return insert(data); }

    iterator insert(const T& data)
    {
        BstNode* node = sentinel_->make_node(data);
        base_insert(node);

        ++sentinel_->size;

        return iterator{node};
    }

    template <typename InputIterator>
    void insert(InputIterator first, InputIterator last)
    {
        std::for_each(first, last, [this](const T& data) {
            this->insert(data);
        });
    }

    void insert(std::initializer_list<T> in) { insert(in.begin(), in.end()); }

    iterator erase(const_iterator pos)
    {
        auto*    node = static_cast<BstNode*>(pos.extract());
        iterator next = ++pos;
        base_erase(node);

        --sentinel_->size;

        if (sentinel_->is_min(node)) [[unlikely]] {
            sentinel_->update_min(node->parent);
        }

        if (sentinel_->is_max(node)) [[unlikely]] {
            sentinel_->update_max(node->parent);
        }

        sentinel_->destroy_node(node);

        return next;
    }

    iterator modify(const_iterator pos, const T& data)
    {
        iterator iter{pos};

        auto not_equal = [](const T& lhs, const T& rhs) noexcept(
                             noexcept(Compare{}(data, data))) {
            return Compare{}(lhs, rhs) != Compare{}(rhs, lhs);
        };

        if (not_equal(*pos, data)) {
            auto* node = static_cast<BstNode*>(pos.extract());

            base_erase(node);
            node->reset();
            node->data = data;
            base_insert(node);

            iter = iterator{node};
        }

        return iter;
    }

    iterator find(const T& data) const noexcept
    {
        const BstNode* node = sentinel_->find(data);
        if (node == nullptr) return iterator{sentinel_};
        return iterator{node};
    }

    position position_of(const T& data) noexcept
    {
        iterator pos = find(data);
        return position{this, pos};
    }

    iterator begin() noexcept { return iterator{sentinel_->min}; }

    const_iterator cbegin() const noexcept
    {
        return const_iterator{sentinel_->min};
    }

    const_iterator begin() const noexcept { return cbegin(); }

    iterator       end() noexcept { return iterator{sentinel_}; }
    const_iterator cend() const noexcept { return const_iterator{sentinel_}; }
    const_iterator end() const noexcept { return cend(); }

    reverse_iterator rbegin() noexcept
    {
        return std::make_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return std::make_reverse_iterator(cend());
    }

    const_reverse_iterator rbegin() const noexcept { return crbegin(); }

    reverse_iterator rend() noexcept
    {
        return std::make_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept
    {
        return std::make_reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept { return crend(); }

    template <typename Visitor, typename... Args>
    void preorder_from(const_iterator it, Visitor&& visit, Args&&... args) const
    {
        preorder_visit(it.extract(),
                       &bst::data_visit,
                       std::forward<Visitor>(visit),
                       std::forward<Args>(args)...);
    }

    template <typename Visitor, typename... Args>
    void inorder_from(const_iterator it, Visitor&& visit, Args&&... args) const
    {
        inorder_visit(it.extract(),
                      &bst::data_visit,
                      std::forward<Visitor>(visit),
                      std::forward<Args>(args)...);
    }

    template <typename Visitor, typename... Args>
    void
    postorder_from(const_iterator it, Visitor&& visit, Args&&... args) const
    {
        postorder_visit(it.extract(),
                        &bst::data_visit,
                        std::forward<Visitor>(visit),
                        std::forward<Args>(args)...);
    }

  protected:
    template <typename Visitor, typename... Args>
    static void preorder_visit(BstNode* node, Visitor&& visit, Args&&... args)
    {
        std::stack<BstNode*> stack;

        if (node) {
            stack.push(node);
        }

        while (!stack.empty()) {
            BstNode* cursor = stack.top();
            stack.pop();

            std::forward<Visitor>(visit)(cursor, std::forward<Args>(args)...);

            if (cursor->right != nullptr) stack.push(cursor->right);
            if (cursor->left != nullptr) stack.push(cursor->left);
        }
    }

    template <typename Visitor, typename... Args>
    static void inorder_visit(BstNode* node, Visitor&& visit, Args&&... args)
    {
        Node* cursor = node->min();

        while (cursor->is_data() && cursor != node->parent) {
            auto* tmp = static_cast<BstNode*>(cursor);
            std::forward<Visitor>(visit)(tmp, std::forward<Args>(args)...);
            cursor = cursor->successor();
        }
    }

    template <typename Visitor, typename... Args>
    static void postorder_visit(BstNode* node, Visitor&& visit, Args&&... args)
    {
        std::stack<BstNode*> stack;

        preorder_visit(
            node,
            [](BstNode* node, std::stack<BstNode*>& stack) {
                stack.push(node);
            },
            stack);

        while (!stack.empty()) {
            auto* tmp = static_cast<BstNode*>(stack.top());
            std::forward<Visitor>(visit)(tmp, std::forward<Args>(args)...);
            stack.pop();
        }
    }

    virtual void base_insert(BstNode* node)
    {
        BstNode* parent{nullptr};
        BstNode* cursor{sentinel_->root};

        // find where the node should be added; parent will be a leaf node
        // and we will add the new node as one of its children
        while (cursor != nullptr) {
            parent = cursor;
            if (*node < *parent) {
                cursor = cursor->left;
            } else {
                cursor = cursor->right;
            }
        }

        node->parent = parent;

        if (parent == nullptr) { // there is no root; update min and max as well
            sentinel_->update_all(node);
        } else if (*node < *parent) { // add as left child and possibly update
                                      // min; min will always be a left child
            parent->left = node;
            if (sentinel_->is_min(parent)) sentinel_->update_min(node);
        } else { // the analogous logic for right children
            // NOTE: this will put equal, but "newer" nodes to the
            // right of the already exisitng node
            parent->right = node;
            if (sentinel_->is_max(parent)) sentinel_->update_max(node);
        }
    }

    void transplant(BstNode* u, BstNode* v)
    {
        // NOTE: this method only takes care of wiring v into the position
        // that u currently holds (whatever happens to u is up to the
        // caller)

        if (u->parent == nullptr) { // u is the root
            sentinel_->root = v;
        } else if (u == u->parent->left) { // u is the left child of its parent
            // replace the subtree at u with the subtree at v
            u->parent->left = v;
        } else { // u is the right child of its parent
            u->parent->right = v;
        }

        // ensure the symmetry of the child-parent relationship (i.e., the
        // parent of v must now be the original parent of u now that v is
        // the child of this parent)
        if (v != nullptr) v->parent = u->parent;
    }

    virtual void single_child_or_leaf_node_erase(BstNode* node, BstNode* rep)
    {
        transplant(node, rep);
    }

    virtual void double_child_node_erase(BstNode* node, BstNode* rep)
    {
        if (rep->parent != node) {
            transplant(rep, rep->right);
            rep->right         = node->right;
            rep->right->parent = rep;
        }

        transplant(node, rep);
        rep->left         = node->left;
        rep->left->parent = rep;
    }

    virtual void base_erase(BstNode* node)
    {
        if (node->left == nullptr) { // node has no left child (and perhaps
                                     // no right child as well)
            // NOTE: in either case we are maintaining the contract since
            // the successor of the deleted node must be the minimum of the
            // right subtree (if any); because there is no left subtree
            single_child_or_leaf_node_erase(node, node->right);
        } else if (node->right == nullptr) { // node has no right child
            /*
             *
             *         |          |
             *         A          B
             *        /    -->   / \
             *       B         ... ...
             *      / \
             *    ... ...
             *
             *
             */
            single_child_or_leaf_node_erase(node, node->left);
        } else { // node has both a left and right child
            double_child_node_erase(node, node->right->min());
        }
    }

    Sentinel* sentinel_;

  private:
    template <typename Visitor, typename... Args>
    static void data_visit(const BstNode* node, Visitor visit, Args&&... args)
    {
        visit(node->data, std::forward<Args>(args)...);
    }
};

#endif // BST_H
