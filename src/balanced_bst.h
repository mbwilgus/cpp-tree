#ifndef BALANCED_BST_H
#define BALANCED_BST_H

#include "bst.h"

template <typename T,
          typename Compare   = std::less<T>,
          typename Allocator = std::allocator<T>>
class balanced_bst : public bst<T, Compare, Allocator>
{
  private:
    using bst      = bst<T, Compare, Allocator>;
    using Sentinel = typename bst::Sentinel;
    using BstNode  = typename bst::BstNode;

  public:
    balanced_bst()          = delete;
    virtual ~balanced_bst() = default;

  protected:
    explicit balanced_bst(Sentinel* alloc)
        : bst{alloc}
    {
    }

    struct BalancedBstNode : public BstNode { // NOLINT
        BalancedBstNode() = default;

        BalancedBstNode(const T&  data,
                        Sentinel* owner) noexcept(noexcept(BstNode{data,
                                                                   owner}))
            : BstNode{data, owner}
        {
        }

        BalancedBstNode(const BalancedBstNode& that,
                        Sentinel* owner) noexcept(noexcept(BstNode{that,
                                                                   owner}))
            : BstNode{that, owner}
        {
        }

        void left_rotate() noexcept
        {
            /*
             * example: T is this node and P is T's parent (if any)
             *
             * notice how the result is that the right subtree rooted at B (if
             * any) is moved up a level; the height of the portion of the tree
             * that is actually shown does not change, but the height of the
             * nodes of the right subtree rooted at B will be one less as long
             * as they exist
             *
             *                 ...                ...
             *                  |                  |
             *                  P                  P
             *                 / \                / \
             *                T  ...             B  ...
             *               / \        -->     / \
             *              A   B              T  ...
             *                 / \            / \
             *                C  ...         A   C
             */

            // we are "rotating" T and its right child, B, to the left
            BstNode* child = this->right;

            // T will become B's left child (because T is less than B), however
            // we can't make C the right child of B (because C is less than B)
            //
            // but T is less than C so we can make it T's right child (the
            // position formerly held by B)
            this->right = child->left;
            if (child->left != nullptr) child->left->parent = this;

            // move B to T's old position
            child->parent = this->parent;
            // NOTE: we have to consider the parent of T and it is possible that
            // T is the root
            if (this->parent == nullptr) {
                this->owner->root = child;
            } else if (this == this->parent->left) {
                this->parent->left = child;
            } else {
                this->parent->right = child;
            }

            // make T the left child of B
            child->left  = this;
            this->parent = child;
        }

        void right_rotate() noexcept
        {
            // SEE: the explanation provided for left_rotate; this situation is
            // symmetric

            BstNode* child = this->left;

            this->left = child->right;
            if (child->right != nullptr) child->right->parent = this;

            child->parent = this->parent;
            if (this->parent == nullptr) {
                this->owner->root = child;
            } else if (this == this->parent->right) {
                this->parent->right = child;
            } else {
                this->parent->left = child;
            }

            child->right = this;
            this->parent = child;
        }
    };

    virtual void post_insert(BalancedBstNode* node) = 0;
    virtual void post_erase(BalancedBstNode* node)  = 0;
};

#endif // BALANCED_BST_H
