#ifndef RB_TREE_H
#define RB_TREE_H

#include "balanced_bst.h"

#include <functional>
#include <memory>

template <typename T,
          typename Compare   = std::less<T>,
          typename Allocator = std::allocator<T>>
class rb_tree : public balanced_bst<T, Compare, Allocator>
{
  private:
    using AllocTraits = std::allocator_traits<Allocator>;

    using bst          = bst<T, Compare, Allocator>;
    using balanced_bst = balanced_bst<T, Compare, Allocator>;
    using Sentinel     = typename bst::Sentinel;

    using BstNode         = typename bst::BstNode;
    using BalancedBstNode = typename balanced_bst::BalancedBstNode;

    using NodeColor = bool;
    static constexpr NodeColor red{false};  // NOLINT
    static constexpr NodeColor black{true}; // NOLINT

    struct RedBlackNode : public BalancedBstNode { // NOLINT
        RedBlackNode() = default;
        RedBlackNode(const T&  data,
                     Sentinel* owner) noexcept(noexcept(BalancedBstNode{data,
                                                                        owner}))
            : BalancedBstNode{data, owner}
        {
        }

        RedBlackNode(const RedBlackNode& that,
                     Sentinel* owner) noexcept(noexcept(BalancedBstNode{that,
                                                                        owner}))
            : BalancedBstNode{that, owner}
        {
        }

        NodeColor color{red};
    };

    template <typename NodeAllocator>
    using BstAllocator = typename bst::template BstAllocator<NodeAllocator>;

    using NodeAllocator =
        typename AllocTraits::template rebind_alloc<RedBlackNode>;
    using RedBlackNodeAllocator = BstAllocator<NodeAllocator>;

  public:
    rb_tree()
        : balanced_bst{new RedBlackNodeAllocator{}}
    {
    }

    rb_tree(const rb_tree& that)
        : rb_tree{}
    {
        this->sentinel_->copy(that.sentinel_);
    }

    rb_tree(rb_tree&& that) noexcept;
    virtual ~rb_tree() = default;

  private:
    void base_insert(BstNode* node) override
    {
        bst::base_insert(node);
        post_insert(static_cast<BalancedBstNode*>(node));
    }

    void post_insert(BalancedBstNode* node) override
    {
        auto col = [](RedBlackNode* node) -> NodeColor {
            return node != nullptr ? node->color : black;
        };

        while (col(static_cast<RedBlackNode*>(node->parent)) == red) {
            auto* parent      = static_cast<RedBlackNode*>(node->parent);
            auto* grandparent = static_cast<RedBlackNode*>(parent->parent);

            if (parent == grandparent->left) {
                auto* uncle = static_cast<RedBlackNode*>(grandparent->right);

                if (col(uncle) == red) {
                    parent->color      = black;
                    uncle->color       = black;
                    grandparent->color = red;
                    node               = grandparent;
                } else {
                    if (node == parent->right) {
                        node = parent;
                        node->left_rotate();
                    }

                    parent->color      = black;
                    grandparent->color = red;

                    grandparent->right_rotate();
                }
            } else {
                auto* uncle = static_cast<RedBlackNode*>(grandparent->left);

                if (col(uncle) == red) {
                    parent->color      = black;
                    uncle->color       = black;
                    grandparent->color = red;
                    node               = grandparent;
                } else {
                    if (node == parent->left) {
                        node = parent;
                        node->right_rotate();
                    }

                    parent->color      = black;
                    grandparent->color = red;

                    grandparent->left_rotate();
                }
            }
        }

        static_cast<RedBlackNode*>(this->sentinel_->root)->color = black;
    }

    void single_child_or_leaf_node_erase(BstNode* node, BstNode* rep) override
    {
        bst::single_child_or_leaf_node_erase(node, rep);
        if (static_cast<RedBlackNode*>(node)->color == black) {
            post_erase(static_cast<RedBlackNode*>(rep));
        }
    }

    void double_child_node_erase(BstNode* node, BstNode* rep) override
    {
        auto* fixme = static_cast<BalancedBstNode*>(rep->right);

        if (rep->parent == node && fixme != nullptr) fixme->parent = rep;

        bst::double_child_node_erase(node, rep);

        auto* node_rb = static_cast<RedBlackNode*>(node);
        auto* rep_rb  = static_cast<RedBlackNode*>(rep);

        // have already assumed that rep is not null
        NodeColor color = rep_rb->color;
        rep_rb->color   = node_rb->color;

        if (color == black && fixme != nullptr) post_erase(fixme);
    }

    // just the way it is
    // NOLINTNEXTLINE(readability-function-cognitive-complexity)
    void post_erase(BalancedBstNode* fixme) override
    {
        auto col = [](RedBlackNode* node) -> NodeColor {
            return node != nullptr ? node->color : black;
        };

        auto* node = static_cast<RedBlackNode*>(fixme);

        while (node->color == black && node != this->sentinel_->root) {

            auto* parent = static_cast<RedBlackNode*>(node->parent);

            if (node == parent->left) {
                auto* uncle = static_cast<RedBlackNode*>(parent->right);

                if (col(uncle) == red) {
                    uncle->color  = black;
                    parent->color = red;
                    parent->left_rotate();
                    uncle = static_cast<RedBlackNode*>(parent->right);
                }

                auto* cousin_left  = static_cast<RedBlackNode*>(uncle->left);
                auto* cousin_right = static_cast<RedBlackNode*>(uncle->right);

                if (col(cousin_left) == black && col(cousin_right) == black) {
                    uncle->color = red;
                    node         = parent;
                } else {
                    if (col(cousin_right) == black) {
                        cousin_left->color = black;
                        uncle->color       = red;
                        uncle->right_rotate();
                        uncle = static_cast<RedBlackNode*>(parent->right);
                    }

                    uncle->color  = parent->color;
                    parent->color = black;
                    static_cast<RedBlackNode*>(uncle->right)->color = black;
                    parent->left_rotate();
                    node = static_cast<RedBlackNode*>(this->sentinel_->root);
                }
            } else {
                auto* uncle = static_cast<RedBlackNode*>(parent->left);

                if (col(uncle) == red) {
                    uncle->color  = black;
                    parent->color = red;
                    parent->right_rotate();
                    uncle = static_cast<RedBlackNode*>(parent->left);
                }

                auto* cousin_left  = static_cast<RedBlackNode*>(uncle->left);
                auto* cousin_right = static_cast<RedBlackNode*>(uncle->right);

                if (col(cousin_left) == black && col(cousin_right) == black) {
                    uncle->color = red;
                    node         = parent;
                } else {
                    if (col(cousin_left) == black) {
                        cousin_right->color = black;
                        uncle->color        = red;
                        uncle->left_rotate();
                        uncle = static_cast<RedBlackNode*>(parent->left);
                    }

                    uncle->color  = parent->color;
                    parent->color = black;
                    static_cast<RedBlackNode*>(uncle->left)->color = black;
                    parent->right_rotate();
                    node = static_cast<RedBlackNode*>(this->sentinel_->root);
                }
            }
        }

        node->color = black;
    }
};

#endif // RB_TREE_H
